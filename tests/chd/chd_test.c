/* chd_test.c - checks src/lib/util/chd.c, MAME's CHD interface over
 * libretro-common's rchd, against images MAME 0.139 wrote
 * (chd_fixtures.h).
 *
 * Checks that the hard disk opens with MAME's header fields and reads
 * as generated, metadata found by tag and by index; that
 * chd_create_file and chd_write turn it into the very files MAME's did,
 * which read back, as MAME's own diff does; that the opens MAME refuses
 * are refused; and that each A/V field, read asynchronously after
 * chd_codec_config, lands in the bitmap, audio buffers and metadata as
 * encoded, and reads raw as encoded too. Files are written to the
 * current directory. Exits non-zero on the first failure.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osdcore.h"
#include "corefile.h"
#include "chd.h"
#include "bitmap.h"

#include "chd_fixtures.h"

/* as the fixtures were generated */
#define HD_HUNK    1024
#define HD_HUNKS   8
#define AV_W       32
#define AV_H       8
#define AV_CH      2
#define AV_FIELDS  4
#define AV_HUNK    1092
#define AV_SAMPLES 134

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
    the OSD layer, on stdio
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
    files and expected contents
-------------------------------------------------*/

static void write_file(const char *path, const unsigned char *data, size_t length)
{
   FILE *fp = fopen(path, "wb");
   CHECK(fp != NULL, "cannot write %s", path);
   if (!fp)
      return;
   CHECK(fwrite(data, 1, length, fp) == length, "short write to %s", path);
   fclose(fp);
}

static int file_is(const char *path, const unsigned char *data, size_t length)
{
   FILE   *fp = fopen(path, "rb");
   size_t  got;
   int     c, same = 1;

   if (!fp)
      return 0;
   for (got = 0; (c = fgetc(fp)) != EOF; got++)
      if (got >= length || (unsigned char)c != data[got])
         same = 0;
   fclose(fp);
   return same && got == length;
}

static void hd_hunk(uint8_t *p, uint32_t hunk)
{
   static const char text[] = "MAME 2010 hard disk ";
   uint32_t i, x;

   if (hunk == 6)
      hunk = 2;                  /* stored as a reference to hunk 2 */
   x = 12345u + hunk * 7919u;
   if (hunk % 4 == 3)
   {
      memset(p, 0, HD_HUNK);
      return;
   }
   for (i = 0; i < HD_HUNK; i++)
   {
      x    = x * 1103515245u + 12345u;
      p[i] = (hunk % 4 == 1) ? (uint8_t)(x >> 24)
                             : (uint8_t)(text[i % 20] ^ (i / 512));
   }
}

static const uint32_t diff_writes[4] = { 2, 0, 2, 7 };

static void diff_write_data(uint8_t *p, unsigned n)
{
   memset(p, 0xA0 + n, HD_HUNK);
   p[0] = (uint8_t)diff_writes[n];
}

static void diff_hunk(uint8_t *p, uint32_t hunk)
{
   int n, last = -1;
   for (n = 0; n < 4; n++)
      if (diff_writes[n] == hunk)
         last = n;
   if (last < 0)
      hd_hunk(p, hunk);
   else
      diff_write_data(p, (unsigned)last);
}

static int hunks_are(chd_file *chd, void (*expect)(uint8_t *, uint32_t))
{
   static uint8_t buf[HD_HUNK], want[HD_HUNK];
   uint32_t h;
   int bad = 0;

   for (h = 0; h < HD_HUNKS; h++)
   {
      expect(want, h);
      if (chd_read(chd, h, buf) != CHDERR_NONE || memcmp(buf, want, HD_HUNK))
         bad++;
   }
   return bad == 0;
}

/*-------------------------------------------------
    the hard disk and its diffs
-------------------------------------------------*/

static void test_hard_disk(void)
{
   static uint8_t buf[HD_HUNK];
   const chd_header *header;
   chd_file  *parent, *diff;
   core_file *file;
   char       meta[64];
   uint8_t    ident[600], flags = 0;
   uint32_t   len = 0, tag = 0;
   unsigned   n;
   chd_error  err;

   write_file("chd_parent.chd", mame_hd_parent, sizeof(mame_hd_parent));
   err = chd_open("chd_parent.chd", CHD_OPEN_READ, NULL, &parent);
   CHECK(err == CHDERR_NONE, "parent opens: %s", chd_error_string(err));
   if (err != CHDERR_NONE)
      return;

   header = chd_get_header(parent);
   CHECK(header->version == 4 && header->length == CHD_V4_HEADER_SIZE, "parent header version");
   CHECK(header->hunkbytes == HD_HUNK && header->totalhunks == HD_HUNKS
         && header->logicalbytes == HD_HUNK * HD_HUNKS, "parent geometry");
   CHECK(header->flags == 0 && header->compression == CHDCOMPRESSION_ZLIB, "parent flags and codec");
   CHECK(!memcmp(header->sha1, mame_hd_parent + 48, CHD_SHA1_BYTES)
         && !memcmp(header->rawsha1, mame_hd_parent + 88, CHD_SHA1_BYTES), "parent SHA-1s");

   CHECK(chd_get_metadata(parent, HARD_DISK_METADATA_TAG, 0, meta, sizeof(meta), &len, &tag, &flags) == CHDERR_NONE
         && !strcmp(meta, "CYLS:5,HEADS:2,SECS:4,BPS:512") && len == 30
         && tag == HARD_DISK_METADATA_TAG && flags == CHD_MDFLAGS_CHECKSUM, "GDDD by tag");
   CHECK(chd_get_metadata(parent, CHDMETATAG_WILDCARD, 1, ident, sizeof(ident), &len, &tag, &flags) == CHDERR_NONE
         && len == 512 && tag == HARD_DISK_IDENT_METADATA_TAG && ident[3] == 9, "IDNT by index");
   CHECK(chd_get_metadata(parent, HARD_DISK_KEY_METADATA_TAG, 0, meta, 4, &len, NULL, &flags) == CHDERR_NONE
         && len == 10 && !memcmp(meta, "unch", 4) && flags == 0, "a short read of KEY");
   CHECK(chd_get_metadata(parent, CHDMETATAG_WILDCARD, 3, meta, sizeof(meta), NULL, NULL, NULL)
         == CHDERR_METADATA_NOT_FOUND, "no fourth metadata entry");
   CHECK(hunks_are(parent, hd_hunk), "parent hunks");
   CHECK(chd_read(parent, HD_HUNKS, buf) == CHDERR_HUNK_OUT_OF_RANGE, "hunk out of range");
   CHECK(chd_write(parent, 0, buf) == CHDERR_FILE_NOT_WRITEABLE, "parent is not writeable");

   /* MAME's own diff, through the parent */
   write_file("chd_mame_diff.chd", mame_hd_diff, sizeof(mame_hd_diff));
   CHECK(chd_open("chd_mame_diff.chd", CHD_OPEN_READ, NULL, &diff) == CHDERR_REQUIRES_PARENT,
         "a diff needs its parent");
   err = chd_open("chd_mame_diff.chd", CHD_OPEN_READ, parent, &diff);
   CHECK(err == CHDERR_NONE, "MAME's diff opens: %s", chd_error_string(err));
   if (err == CHDERR_NONE)
   {
      CHECK(hunks_are(diff, diff_hunk), "MAME's diff hunks");
      chd_close(diff);
   }

   /* a diff made and written here, file for file as MAME's */
   CHECK(core_fopen("chd_diff.chd", OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE, &file) == FILERR_NONE,
         "create diff file");
   if (failures)
   {
      chd_close(parent);
      return;
   }
   CHECK(chd_create_file(file, 0, 0, CHDCOMPRESSION_ZLIB, parent) == CHDERR_NOT_SUPPORTED,
         "no compressed diffs");
   CHECK(chd_create_file(file, 0, 0, CHDCOMPRESSION_NONE, parent) == CHDERR_NONE, "chd_create_file");
   core_fclose(file);
   CHECK(file_is("chd_diff.chd", mame_hd_diff_new, sizeof(mame_hd_diff_new)),
         "new diff is the file MAME made");

   CHECK(core_fopen("chd_diff.chd", OPEN_FLAG_READ | OPEN_FLAG_WRITE, &file) == FILERR_NONE, "reopen diff");
   err = chd_open_file(file, CHD_OPEN_READWRITE, parent, &diff);
   CHECK(err == CHDERR_NONE, "diff opens writeable: %s", chd_error_string(err));
   if (err == CHDERR_NONE)
   {
      CHECK(chd_get_header(diff)->flags == (CHDFLAGS_HAS_PARENT | CHDFLAGS_IS_WRITEABLE), "diff flags");
      CHECK(chd_get_metadata(diff, HARD_DISK_METADATA_TAG, 0, meta, sizeof(meta), NULL, NULL, NULL) == CHDERR_NONE
            && !strcmp(meta, "CYLS:5,HEADS:2,SECS:4,BPS:512"), "diff carries the parent's metadata");
      CHECK(hunks_are(diff, hd_hunk), "new diff reads as its parent");
      for (n = 0; n < 4; n++)
      {
         diff_write_data(buf, n);
         CHECK(chd_write(diff, diff_writes[n], buf) == CHDERR_NONE, "write %u", n);
      }
      CHECK(hunks_are(diff, diff_hunk), "written diff reads back");
      chd_close(diff);
   }
   core_fclose(file);
   CHECK(file_is("chd_diff.chd", mame_hd_diff, sizeof(mame_hd_diff)),
         "written diff is the file MAME's writes made");

   CHECK(chd_open("chd_parent.chd", CHD_OPEN_READWRITE, NULL, &diff) == CHDERR_FILE_NOT_WRITEABLE,
         "the parent does not open writeable");
   chd_close(parent);
}

/*-------------------------------------------------
    A/V
-------------------------------------------------*/

static void av_field(uint8_t *p, uint32_t f)
{
   uint32_t samples  = AV_SAMPLES - (f & 1);
   uint32_t metasize = 7 + f;
   uint32_t i, c, x, y;

   memset(p, 0, AV_HUNK);
   p[0] = 'c'; p[1] = 'h'; p[2] = 'a'; p[3] = 'v';
   p[4] = (uint8_t)metasize;
   p[5] = AV_CH;
   p[6] = (uint8_t)(samples >> 8);
   p[7] = (uint8_t)samples;
   p[9] = AV_W;
   p[11] = AV_H;
   p += 12;
   for (i = 0; i < metasize; i++)
      *p++ = (uint8_t)(0x40 + f * 3 + i);
   for (c = 0; c < AV_CH; c++)
      for (i = 0; i < samples; i++)
      {
         int32_t  v = (int32_t)((i * (c ? 211u : 97u) + f * 1009u) % 4001u) * 4
                    - 8000 + (int32_t)(i * 37 % 11) - 5;
         uint16_t s = (uint16_t)(int16_t)v;
         *p++ = (uint8_t)(s >> 8);
         *p++ = (uint8_t)s;
      }
   for (y = 0; y < AV_H; y++)
      for (x = 0; x < AV_W; x++)
      {
         *p++ = (uint8_t)((x & 1) ? 128 + y * 3 - f : 128 - x + f);
         *p++ = (uint8_t)(16 + ((x * 3 + y * 5 + f * 11) % 200)
               + ((x * y + f) % 7));
      }
}

static void test_av(void)
{
   static uint8_t raw[AV_HUNK], want[AV_HUNK];
   av_codec_decompress_config config;
   int16_t    audio[AV_CH][AV_SAMPLES + 8];
   uint8_t    meta[64];
   uint32_t   samples = 0, metalen = 0, f, x, y, i, c;
   bitmap_t  *video;
   chd_file  *av;
   chd_error  err;
   char       text[256];
   int        bad = 0;

   write_file("chd_av.chd", mame_av, sizeof(mame_av));
   err = chd_open("chd_av.chd", CHD_OPEN_READ, NULL, &av);
   CHECK(err == CHDERR_NONE, "A/V image opens: %s", chd_error_string(err));
   if (err != CHDERR_NONE)
      return;
   CHECK(chd_get_header(av)->compression == CHDCOMPRESSION_AV, "A/V codec");
   CHECK(chd_get_metadata(av, AV_METADATA_TAG, 0, text, sizeof(text), NULL, NULL, NULL) == CHDERR_NONE
         && !strncmp(text, "FPS:29.970030 WIDTH:32 HEIGHT:8 INTERLACED:1 CHANNELS:2 SAMPLERATE:4000", 72),
         "A/V metadata");

   /* raw, before any configuration */
   for (f = 0; f < AV_FIELDS; f++)
   {
      av_field(want, f);
      if (chd_read(av, f, raw) != CHDERR_NONE || memcmp(raw, want, AV_HUNK))
         bad++;
   }
   CHECK(bad == 0, "A/V fields read raw");

   video = bitmap_alloc(AV_W + 3, AV_H + 1, BITMAP_FORMAT_YUY16);
   memset(&config, 0, sizeof(config));
   config.video         = video;
   config.maxsamples    = AV_SAMPLES + 8;
   config.actsamples    = &samples;
   config.audio[0]      = audio[0];
   config.audio[1]      = audio[1];
   config.maxmetalength = sizeof(meta);
   config.actmetalength = &metalen;
   config.metadata      = meta;
   CHECK(chd_codec_config(av, AV_CODEC_DECOMPRESS_CONFIG, &config) == CHDERR_NONE, "codec config");

   for (f = 0; f < AV_FIELDS; f++)
   {
      const uint8_t *p;

      err = chd_read_async(av, f, NULL);
      if (err == CHDERR_OPERATION_PENDING)
         err = chd_async_complete(av);
      if (err != CHDERR_NONE)
      {
         bad++;
         continue;
      }
      av_field(want, f);
      p = want + 12;
      if (metalen != 7 + f || memcmp(meta, p, metalen))
         bad++;
      p += 7 + f;
      if (samples != AV_SAMPLES - (f & 1))
         bad++;
      for (c = 0; c < AV_CH; c++)
         for (i = 0; i < samples; i++, p += 2)
            if (audio[c][i] != (int16_t)((p[0] << 8) | p[1]))
               bad++;
      for (y = 0; y < AV_H; y++)
         for (x = 0; x < AV_W; x++, p += 2)
            if (*BITMAP_ADDR16(video, y, x) != ((p[0] << 8) | p[1]))
               bad++;
   }
   CHECK(bad == 0, "A/V fields unpack as encoded");
   CHECK(chd_async_complete(av) == CHDERR_NO_ASYNC_OPERATION, "nothing left in flight");

   /* a buffer smaller than the field is refused */
   config.maxsamples = AV_SAMPLES - 2;
   chd_codec_config(av, AV_CODEC_DECOMPRESS_CONFIG, &config);
   CHECK(chd_read(av, 0, NULL) == CHDERR_DECOMPRESSION_ERROR, "too few samples refused");

   bitmap_free(video);
   chd_close(av);
}

int main(void)
{
   test_hard_disk();
   test_av();

   if (failures)
   {
      fprintf(stderr, "chd_test: %d failure(s)\n", failures);
      return 1;
   }
   printf("chd_test: ok\n");
   return 0;
}
