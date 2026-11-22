/* zlib_compat_test.c - checks the zlib-format paths in src/lib/util,
 * which run on libretro-common's encodings/deflate.c and
 * encodings/crc32.c.
 *
 * Reads streams written by zlib (fixtures.h) through core_fcompress,
 * zip_file_decompress and png_read_bitmap, round-trips data through the
 * writers (core_fcompress, png_write_bitmap, CHD zlib compression), and
 * checks crc32 and SHA-1 against published values.  Files are written to the current
 * directory.  Exits non-zero on the first failure.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osdcore.h"
#include "corefile.h"
#include "unzip.h"
#include "png.h"
#include "chd.h"
#include "bitmap.h"
#include "palette.h"

#include <encodings/crc32.h>
#undef INLINE
#define INLINE inline
#include <lrc_hash.h>

#include "fixtures.h"

static int failures = 0;

static void CHECK(int cond, const char *fmt, ...)
{
   va_list ap;
   if (cond)
      return;
   va_start(ap, fmt);
   fputs("FAIL: ", stderr);
   vfprintf(stderr, fmt, ap);
   fputc('\n', stderr);
   va_end(ap);
   failures++;
}

/*-------------------------------------------------
    the OSD file layer, on stdio
-------------------------------------------------*/

struct _osd_file
{
   FILE *fp;
};

file_error osd_open(const char *path, uint32_t openflags, osd_file **file, uint64_t *filesize)
{
   const char *mode;
   osd_file   *f;

   if ((openflags & OPEN_FLAG_WRITE) && (openflags & OPEN_FLAG_CREATE))
      mode = (openflags & OPEN_FLAG_READ) ? "w+b" : "wb";
   else if (openflags & OPEN_FLAG_WRITE)
      mode = "r+b";
   else
      mode = "rb";

   if (!(f = (osd_file *)malloc(sizeof(*f))))
      return FILERR_OUT_OF_MEMORY;
   if (!(f->fp = fopen(path, mode)))
   {
      free(f);
      return FILERR_NOT_FOUND;
   }
   fseek(f->fp, 0, SEEK_END);
   *filesize = (uint64_t)ftell(f->fp);
   *file     = f;
   return FILERR_NONE;
}

file_error osd_close(osd_file *file)
{
   fclose(file->fp);
   free(file);
   return FILERR_NONE;
}

file_error osd_read(osd_file *file, void *buffer, uint64_t offset, uint32_t length, uint32_t *actual)
{
   fseek(file->fp, (long)offset, SEEK_SET);
   *actual = (uint32_t)fread(buffer, 1, length, file->fp);
   return FILERR_NONE;
}

file_error osd_write(osd_file *file, const void *buffer, uint64_t offset, uint32_t length, uint32_t *actual)
{
   fseek(file->fp, (long)offset, SEEK_SET);
   *actual = (uint32_t)fwrite(buffer, 1, length, file->fp);
   return (*actual == length) ? FILERR_NONE : FILERR_FAILURE;
}

int osd_uchar_from_osdchar(uint32_t *uchar, const char *osdchar, size_t count)
{
   *uchar = (unsigned char)osdchar[0];
   return 1;
}

void osd_break_into_debugger(const char *message)
{
   CHECK(0, "debugger break: %s", message);
}

osd_ticks_t osd_ticks_per_second(void)
{
   return 1000000;
}

int osd_num_processors(void)
{
   return 2;
}

/*-------------------------------------------------
    expected contents
-------------------------------------------------*/

/* as the fixture generator made them */
static void test_pattern(uint8_t *out, uint32_t length, uint32_t seed)
{
   static const char alphabet[] = "ABCD  \n";
   uint32_t x = seed, i;
   for (i = 0; i < length; i++)
   {
      x = (x * 1103515245u + 12345u) & 0x7fffffffu;
      out[i] = ((i / 64) % 3) ? (uint8_t)alphabet[(x >> 16) % 7] : (uint8_t)(x >> 8);
   }
}

static uint32_t test_pixel(int x, int y)
{
   return MAKE_ARGB(0xff, (x * 7 + y * 3) & 0xff, (x * y + 11) & 0xff,
         (255 - x * 5 - y) & 0xff);
}

static void write_fixture(const char *path, const unsigned char *data, size_t length)
{
   FILE *fp = fopen(path, "wb");
   CHECK(fp != NULL, "cannot write %s", path);
   if (!fp)
      return;
   CHECK(fwrite(data, 1, length, fp) == length, "short write to %s", path);
   fclose(fp);
}

/* reads a compressed core_file in uneven pieces and compares it */
static void read_compressed(const char *path, const uint8_t *expect, uint32_t length)
{
   static const uint32_t pieces[] = { 1, 7, 333, 4096, 2 };
   core_file *file;
   uint8_t   *got = (uint8_t *)malloc(length + 64);
   uint32_t   done = 0, n = 0, extra;

   CHECK(core_fopen(path, OPEN_FLAG_READ, &file) == FILERR_NONE, "open %s", path);
   if (failures)
   {
      free(got);
      return;
   }
   CHECK(core_fcompress(file, FCOMPRESS_MEDIUM) == FILERR_NONE, "fcompress read %s", path);
   while (done < length)
   {
      uint32_t want = pieces[n++ % (sizeof(pieces) / sizeof(pieces[0]))];
      uint32_t got_now;
      if (want > length - done)
         want = length - done;
      got_now = core_fread(file, got + done, want);
      CHECK(got_now == want, "%s: read %u of %u at %u", path, got_now, want, done);
      if (got_now != want)
         break;
      done += got_now;
   }
   CHECK(done == length && !memcmp(got, expect, length), "%s: contents differ", path);
   extra = core_fread(file, got, 64);
   CHECK(extra == 0, "%s: %u bytes past the end", path, extra);
   core_fclose(file);
   free(got);
}

/*-------------------------------------------------
    tests
-------------------------------------------------*/

static void test_crc32(void)
{
   CHECK(encoding_crc32(0, (const uint8_t *)"123456789", 9) == 0xCBF43926u,
         "crc32 check value");
   /* continuing a crc across calls, as png.c and state.c do */
   CHECK(encoding_crc32(encoding_crc32(0, (const uint8_t *)"1234", 4),
            (const uint8_t *)"56789", 5) == 0xCBF43926u, "crc32 continued");
}

static void sha1_hex(const uint8_t *digest, char *out)
{
   int i;
   for (i = 0; i < 20; i++)
      sprintf(out + i * 2, "%02x", digest[i]);
}

static void test_sha1(void)
{
   static uint8_t million[1000000];
   struct sha1_state sha1;
   uint8_t digest[20];
   char    hex[41];
   size_t  done = 0, piece = 1;

   /* FIPS 180 test vectors */
   sha1_stream_init(&sha1);
   sha1_stream_update(&sha1, (const uint8_t *)"abc", 3);
   CHECK(sha1_stream_final(&sha1, digest), "sha1 abc final");
   sha1_hex(digest, hex);
   CHECK(!strcmp(hex, "a9993e364706816aba3e25717850c26c9cd0d89d"), "sha1 abc: %s", hex);

   /* a million 'a', fed in uneven pieces as chd.c feeds hunks */
   memset(million, 'a', sizeof(million));
   sha1_stream_init(&sha1);
   while (done < sizeof(million))
   {
      size_t n = (piece > sizeof(million) - done) ? sizeof(million) - done : piece;
      sha1_stream_update(&sha1, million + done, n);
      done += n;
      piece = piece * 7 % 4099 + 1;
   }
   CHECK(sha1_stream_final(&sha1, digest), "sha1 million final");
   sha1_hex(digest, hex);
   CHECK(!strcmp(hex, "34aa973cd4c4daa4f61eeb2bdbad27316534016f"), "sha1 million a: %s", hex);
}

static void test_corefile_reads_zlib(void)
{
   uint8_t *plain = (uint8_t *)malloc(FIXTURE_PLAIN_LENGTH);
   test_pattern(plain, FIXTURE_PLAIN_LENGTH, FIXTURE_PLAIN_SEED);
   write_fixture("zc_stream.bin", fixture_zlib_stream, sizeof(fixture_zlib_stream));
   read_compressed("zc_stream.bin", plain, FIXTURE_PLAIN_LENGTH);
   free(plain);
}

static void test_corefile_roundtrip(int level)
{
   const uint32_t length = 300000;
   uint8_t   *plain = (uint8_t *)malloc(length);
   core_file *file;
   uint32_t   done = 0, piece = 1;

   test_pattern(plain, length, 11 + level);
   CHECK(core_fopen("zc_roundtrip.bin", OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &file) == FILERR_NONE,
         "create zc_roundtrip.bin");
   if (failures)
   {
      free(plain);
      return;
   }
   CHECK(core_fcompress(file, level) == FILERR_NONE, "fcompress write level %d", level);
   while (done < length)
   {
      uint32_t want = (piece > length - done) ? length - done : piece;
      CHECK(core_fwrite(file, plain + done, want) == want, "write at %u", done);
      done += want;
      piece = piece * 3 % 997 + 1;
   }
   core_fclose(file);
   read_compressed("zc_roundtrip.bin", plain, length);
   free(plain);
}

static void test_zip(void)
{
   zip_file *zip;
   const zip_file_header *header;
   uint8_t *expect = (uint8_t *)malloc(FIXTURE_ZIP_BIG_LENGTH);
   int entries = 0;

   test_pattern(expect, FIXTURE_ZIP_BIG_LENGTH, FIXTURE_ZIP_BIG_SEED);
   write_fixture("zc_fixture.zip", fixture_zip, sizeof(fixture_zip));
   CHECK(zip_file_open("zc_fixture.zip", &zip) == ZIPERR_NONE, "zip open");
   if (failures)
   {
      free(expect);
      return;
   }
   for (header = zip_file_first_file(zip); header != NULL; header = zip_file_next_file(zip))
   {
      uint8_t  *data = (uint8_t *)malloc(header->uncompressed_length + 1);
      zip_error err  = zip_file_decompress(zip, data, header->uncompressed_length);
      CHECK(err == ZIPERR_NONE, "zip entry %s: error %d", header->filename, (int)err);
      if (!strcmp(header->filename, "big.bin"))
         CHECK(header->uncompressed_length == FIXTURE_ZIP_BIG_LENGTH
               && !memcmp(data, expect, FIXTURE_ZIP_BIG_LENGTH), "zip big.bin differs");
      else if (!strcmp(header->filename, "small.txt"))
      {
         int i, ok = header->uncompressed_length == 19 * 5;
         for (i = 0; ok && i < 5; i++)
            ok = !memcmp(data + i * 19, "mame2010 zip entry\n", 19);
         CHECK(ok, "zip small.txt differs");
      }
      else
         CHECK(0, "unexpected zip entry %s", header->filename);
      free(data);
      entries++;
   }
   CHECK(entries == 2, "zip had %d entries", entries);
   zip_file_close(zip);
   zip_file_cache_clear();
   free(expect);
}

static void compare_bitmap(const char *what, bitmap_t *bitmap)
{
   int x, y, bad = 0;
   CHECK(bitmap->width == FIXTURE_PNG_WIDTH && bitmap->height == FIXTURE_PNG_HEIGHT,
         "%s: %dx%d", what, bitmap->width, bitmap->height);
   if (failures)
      return;
   for (y = 0; y < bitmap->height; y++)
      for (x = 0; x < bitmap->width; x++)
         if ((*BITMAP_ADDR32(bitmap, y, x) & 0xffffff) != (test_pixel(x, y) & 0xffffff))
            bad++;
   CHECK(bad == 0, "%s: %d pixels differ", what, bad);
}

static void test_png(void)
{
   core_file *file;
   bitmap_t  *bitmap = NULL;
   bitmap_t  *source;
   int x, y;

   /* written by zlib, every filter type, the image split over IDATs */
   write_fixture("zc_fixture.png", fixture_png, sizeof(fixture_png));
   CHECK(core_fopen("zc_fixture.png", OPEN_FLAG_READ, &file) == FILERR_NONE, "open png");
   if (failures)
      return;
   CHECK(png_read_bitmap(file, &bitmap) == PNGERR_NONE, "png read");
   core_fclose(file);
   if (bitmap)
   {
      compare_bitmap("zc_fixture.png", bitmap);
      bitmap_free(bitmap);
      bitmap = NULL;
   }

   /* written here, read back */
   source = bitmap_alloc(FIXTURE_PNG_WIDTH, FIXTURE_PNG_HEIGHT, BITMAP_FORMAT_RGB32);
   for (y = 0; y < FIXTURE_PNG_HEIGHT; y++)
      for (x = 0; x < FIXTURE_PNG_WIDTH; x++)
         *BITMAP_ADDR32(source, y, x) = test_pixel(x, y);
   CHECK(core_fopen("zc_written.png", OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &file) == FILERR_NONE,
         "create png");
   if (failures)
      return;
   CHECK(png_write_bitmap(file, NULL, source, 0, NULL) == PNGERR_NONE, "png write");
   core_fclose(file);
   bitmap_free(source);
   CHECK(core_fopen("zc_written.png", OPEN_FLAG_READ, &file) == FILERR_NONE, "reopen png");
   if (failures)
      return;
   CHECK(png_read_bitmap(file, &bitmap) == PNGERR_NONE, "png read back");
   core_fclose(file);
   if (bitmap)
   {
      compare_bitmap("zc_written.png", bitmap);
      bitmap_free(bitmap);
   }
}

#define TEST_HUNKBYTES 4096
#define TEST_HUNKS     12

/* compressible, empty and incompressible hunks, so both the zlib and
 * the uncompressed hunk types are written */
static void chd_hunk(uint8_t *out, int hunk)
{
   uint32_t x = 2463534242u + (uint32_t)hunk;
   int i;
   switch (hunk % 3)
   {
      case 0:
         test_pattern(out, TEST_HUNKBYTES, 100 + hunk);
         break;
      case 1:
         memset(out, 0, TEST_HUNKBYTES);
         break;
      default:
         for (i = 0; i < TEST_HUNKBYTES; i++)
         {
            x ^= x << 13;
            x ^= x >> 17;
            x ^= x << 5;
            out[i] = (uint8_t)(x >> 24);
         }
         break;
   }
}

static void test_chd(void)
{
   static uint8_t data[TEST_HUNKBYTES], got[TEST_HUNKBYTES];
   chd_verify_result verify;
   chd_file *chd;
   chd_error err;
   double    ratio;
   int       hunk;

   err = chd_create("zc_test.chd", (uint64_t)TEST_HUNKBYTES * TEST_HUNKS, TEST_HUNKBYTES,
         CHDCOMPRESSION_ZLIB, NULL);
   CHECK(err == CHDERR_NONE, "chd create: %d", (int)err);
   if (failures)
      return;
   err = chd_open("zc_test.chd", CHD_OPEN_READWRITE, NULL, &chd);
   CHECK(err == CHDERR_NONE, "chd open for write: %d", (int)err);
   if (failures)
      return;
   CHECK(chd_compress_begin(chd) == CHDERR_NONE, "chd compress begin");
   for (hunk = 0; hunk < TEST_HUNKS; hunk++)
   {
      chd_hunk(data, hunk);
      err = chd_compress_hunk(chd, data, &ratio);
      CHECK(err == CHDERR_NONE, "chd compress hunk %d: %d", hunk, (int)err);
   }
   CHECK(chd_compress_finish(chd, TRUE) == CHDERR_NONE, "chd compress finish");
   chd_close(chd);

   err = chd_open("zc_test.chd", CHD_OPEN_READ, NULL, &chd);
   CHECK(err == CHDERR_NONE, "chd open for read: %d", (int)err);
   if (failures)
      return;
   for (hunk = 0; hunk < TEST_HUNKS; hunk++)
   {
      chd_hunk(data, hunk);
      err = chd_read(chd, hunk, got);
      CHECK(err == CHDERR_NONE && !memcmp(got, data, TEST_HUNKBYTES),
            "chd hunk %d: error %d or contents differ", hunk, (int)err);
   }

   /* the SHA-1 recomputed over the data matches the one written while
    * compressing */
   CHECK(chd_verify_begin(chd) == CHDERR_NONE, "chd verify begin");
   for (hunk = 0; hunk < TEST_HUNKS; hunk++)
   {
      err = chd_verify_hunk(chd);
      CHECK(err == CHDERR_NONE, "chd verify hunk %d: %d", hunk, (int)err);
   }
   CHECK(chd_verify_finish(chd, &verify) == CHDERR_NONE, "chd verify finish");
   CHECK(!memcmp(verify.rawsha1, chd_get_header(chd)->rawsha1, CHD_SHA1_BYTES),
         "chd raw SHA-1 differs from the header's");
   chd_close(chd);
}

int main(void)
{
   test_crc32();
   test_sha1();
   test_corefile_reads_zlib();
   test_corefile_roundtrip(FCOMPRESS_MIN);
   test_corefile_roundtrip(FCOMPRESS_MAX);
   test_zip();
   test_png();
   test_chd();

   if (failures)
   {
      fprintf(stderr, "zlib_compat_test: %d failure(s)\n", failures);
      return 1;
   }
   printf("zlib_compat_test: ok\n");
   return 0;
}
