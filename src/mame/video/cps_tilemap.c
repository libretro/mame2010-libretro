/***************************************************************************

    cps_tilemap.c

    Driver-owned tilemap for the CPS1/CPS2 video hardware.  See the header
    for the rationale.  The cache/rasterize/composite logic here has been
    verified bit-identical to the core tilemap engine for the CPS
    configuration.

***************************************************************************/

#include "emu.h"
#include "cps_tilemap.h"

#if defined(__SSE2__)
#include <emmintrin.h>
#define CPS_TM_HAVE_SSE2 1
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#define CPS_TM_HAVE_NEON 1
#endif

#define CPS_TM_MAX_PEN_TO_FLAGS		256
#define CPS_TM_NUM_GROUPS			8		/* CPS uses 4 groups; 8 is generous */
#define CPS_TM_FLAG_DIRTY			0xff

/* trans state, matching the core */
enum { CPS_WHOLLY_TRANSPARENT = 0, CPS_WHOLLY_OPAQUE = 1, CPS_MASKED = 2 };

struct _cps_tm
{
	int						cols, rows, tilewidth, tileheight;
	uint32_t					width, height;
	cps_tm_get_info_func	get_info;
	cps_tm_scan_func		scan;
	int *					logical_to_memory;	/* [cols*rows], built from scan */
	int *					memory_to_logical;	/* [max_memindex+1], reverse map */
	int						max_memindex;
	uint8_t *					pen_to_flags;		/* [CPS_TM_NUM_GROUPS*256] */
	bitmap_t *				pixmap;				/* INDEXED16 derived cache */
	bitmap_t *				flagsmap;			/* INDEXED8 per-pixel flags */
	uint8_t *					tileflags;			/* [cols*rows] */
	uint8_t					enable;
	uint8_t					attributes;
	uint8_t					all_tiles_dirty;
	uint32_t					scrollrows, scrollcols;
	int32_t *					rowscroll;			/* [height] */
	int32_t *					colscroll;			/* [width] */
	int32_t					dx, dx_flipped, dy, dy_flipped;
	tile_data				tileinfo;
};

/*-------------------------------------------------
    effective scroll, matching the core math
-------------------------------------------------*/

INLINE int32_t cps_tm_eff_rowscroll(cps_tm *t, int index, uint32_t screen_width)
{
	int32_t value;

	if (t->attributes & TILEMAP_FLIPY)
		index = t->scrollrows - 1 - index;

	if (!(t->attributes & TILEMAP_FLIPX))
		value = t->dx - t->rowscroll[index];
	else
		value = screen_width - t->width - (t->dx_flipped - t->rowscroll[index]);

	if (value < 0)
		value = t->width - (-value) % t->width;
	else
		value %= t->width;
	return value;
}

INLINE int32_t cps_tm_eff_colscroll(cps_tm *t, int index, uint32_t screen_height)
{
	int32_t value;

	if (t->attributes & TILEMAP_FLIPX)
		index = t->scrollcols - 1 - index;

	if (!(t->attributes & TILEMAP_FLIPY))
		value = t->dy - t->colscroll[index];
	else
		value = screen_height - t->height - (t->dy_flipped - t->colscroll[index]);

	if (value < 0)
		value = t->height - (-value) % t->height;
	else
		value %= t->height;
	return value;
}

/*-------------------------------------------------
    cps_tm_tile_draw - rasterize one tile into the
    pixmap and flags map (pen expansion + palette
    base), returning the tile's transparency mask
-------------------------------------------------*/

static uint8_t cps_tm_tile_draw(cps_tm *t, const uint8_t *pendata, uint32_t x0, uint32_t y0,
		uint32_t palette_base, uint8_t category, uint8_t group, uint8_t flags, uint8_t pen_mask)
{
	const uint8_t *penmap = t->pen_to_flags + group * CPS_TM_MAX_PEN_TO_FLAGS;
	bitmap_t *flagsmap = t->flagsmap;
	bitmap_t *pixmap = t->pixmap;
	int height = t->tileheight;
	int width = t->tilewidth;
	uint8_t andmask = ~0, ormask = 0;
	int dx0 = 1, dy0 = 1;
	int tx, ty;

	category |= flags & (TILE_FORCE_LAYER0 | TILE_FORCE_LAYER1 | TILE_FORCE_LAYER2);

	if (flags & TILE_FLIPY)
	{
		y0 += height - 1;
		dy0 = -1;
	}
	if (flags & TILE_FLIPX)
	{
		x0 += width - 1;
		dx0 = -1;
	}
	if (flags & TILE_4BPP)
		width /= 2;

	for (ty = 0; ty < height; ty++)
	{
		uint16_t *pixptr = BITMAP_ADDR16(pixmap, y0, x0);
		uint8_t *flagsptr = BITMAP_ADDR8(flagsmap, y0, x0);
		int xoffs = 0;

		y0 += dy0;

		if (!(flags & TILE_4BPP))
		{
			for (tx = 0; tx < width; tx++)
			{
				uint8_t pen = (*pendata++) & pen_mask;
				uint8_t map = penmap[pen];
				pixptr[xoffs] = palette_base + pen;
				flagsptr[xoffs] = map | category;
				andmask &= map;
				ormask |= map;
				xoffs += dx0;
			}
		}
		else
		{
			for (tx = 0; tx < width; tx++)
			{
				uint8_t data = *pendata++;
				uint8_t pen, map;

				pen = (data & 0x0f) & pen_mask;
				map = penmap[pen];
				pixptr[xoffs] = palette_base + pen;
				flagsptr[xoffs] = map | category;
				andmask &= map;
				ormask |= map;
				xoffs += dx0;

				pen = (data >> 4) & pen_mask;
				map = penmap[pen];
				pixptr[xoffs] = palette_base + pen;
				flagsptr[xoffs] = map | category;
				andmask &= map;
				ormask |= map;
				xoffs += dx0;
			}
		}
	}
	return andmask ^ ormask;
}

/*-------------------------------------------------
    cps_tm_tile_update - call the driver callback
    for a dirty tile and rasterize it
-------------------------------------------------*/

static void cps_tm_tile_update(cps_tm *t, running_machine *machine, int logindex, int col, int row)
{
	int memindex = t->logical_to_memory[logindex];
	uint32_t x0 = t->tilewidth * col;
	uint32_t y0 = t->tileheight * row;
	uint8_t flags;

	t->tileinfo.pen_data = NULL;
	t->tileinfo.mask_data = NULL;
	t->tileinfo.palette_base = 0;
	t->tileinfo.category = 0;
	t->tileinfo.group = 0;
	t->tileinfo.flags = 0;
	t->tileinfo.pen_mask = 0xff;
	t->tileinfo.gfxnum = 0xff;

	(*t->get_info)(machine, &t->tileinfo, memindex);

	flags = t->tileinfo.flags ^ (t->attributes & 0x03);

	t->tileflags[logindex] = cps_tm_tile_draw(t, t->tileinfo.pen_data, x0, y0,
			t->tileinfo.palette_base, t->tileinfo.category, t->tileinfo.group, flags, t->tileinfo.pen_mask);
}

/*-------------------------------------------------
    scanline compositors (INDEXED16): dest = src +
    pal, with optional priority blend and mask test.
    SIMD-accelerated, scalar fallback retained.
-------------------------------------------------*/

static void cps_tm_scanline_opaque(uint16_t *dest, const uint16_t *source, int count, uint8_t *pri, uint32_t pcode)
{
	int pal = pcode >> 16;
	int i = 0;

	if (pal == 0)
	{
		memcpy(dest, source, count * 2);
		if (pcode != 0xff00)
		{
#if defined(CPS_TM_HAVE_SSE2)
			__m128i va = _mm_set1_epi8((char)(pcode >> 8)), vo = _mm_set1_epi8((char)pcode);
			for ( ; i + 16 <= count; i += 16)
				_mm_storeu_si128((__m128i*)(pri+i), _mm_or_si128(_mm_and_si128(_mm_loadu_si128((const __m128i*)(pri+i)), va), vo));
#elif defined(CPS_TM_HAVE_NEON)
			uint8x16_t va = vdupq_n_u8((uint8_t)(pcode >> 8)), vo = vdupq_n_u8((uint8_t)pcode);
			for ( ; i + 16 <= count; i += 16)
				vst1q_u8(pri+i, vorrq_u8(vandq_u8(vld1q_u8(pri+i), va), vo));
#endif
			for ( ; i < count; i++)
				pri[i] = (pri[i] & (pcode >> 8)) | pcode;
		}
	}
	else if ((pcode & 0xffff) != 0xff00)
	{
#if defined(CPS_TM_HAVE_SSE2)
		__m128i vpal = _mm_set1_epi16((short)pal);
		__m128i va = _mm_set1_epi8((char)(pcode >> 8)), vo = _mm_set1_epi8((char)pcode);
		int j = 0;
		for ( ; i + 8 <= count; i += 8)
			_mm_storeu_si128((__m128i*)(dest+i), _mm_add_epi16(_mm_loadu_si128((const __m128i*)(source+i)), vpal));
		for ( ; j + 16 <= count; j += 16)
			_mm_storeu_si128((__m128i*)(pri+j), _mm_or_si128(_mm_and_si128(_mm_loadu_si128((const __m128i*)(pri+j)), va), vo));
		for ( ; j < count; j++)
			pri[j] = (pri[j] & (pcode >> 8)) | pcode;
#elif defined(CPS_TM_HAVE_NEON)
		uint16x8_t vpal = vdupq_n_u16((uint16_t)pal);
		uint8x16_t va = vdupq_n_u8((uint8_t)(pcode >> 8)), vo = vdupq_n_u8((uint8_t)pcode);
		int j = 0;
		for ( ; i + 8 <= count; i += 8)
			vst1q_u16(dest+i, vaddq_u16(vld1q_u16(source+i), vpal));
		for ( ; j + 16 <= count; j += 16)
			vst1q_u8(pri+j, vorrq_u8(vandq_u8(vld1q_u8(pri+j), va), vo));
		for ( ; j < count; j++)
			pri[j] = (pri[j] & (pcode >> 8)) | pcode;
#endif
		for ( ; i < count; i++)
		{
			dest[i] = source[i] + pal;
			pri[i] = (pri[i] & (pcode >> 8)) | pcode;
		}
	}
	else
	{
#if defined(CPS_TM_HAVE_SSE2)
		__m128i vpal = _mm_set1_epi16((short)pal);
		for ( ; i + 8 <= count; i += 8)
			_mm_storeu_si128((__m128i*)(dest+i), _mm_add_epi16(_mm_loadu_si128((const __m128i*)(source+i)), vpal));
#elif defined(CPS_TM_HAVE_NEON)
		uint16x8_t vpal = vdupq_n_u16((uint16_t)pal);
		for ( ; i + 8 <= count; i += 8)
			vst1q_u16(dest+i, vaddq_u16(vld1q_u16(source+i), vpal));
#endif
		for ( ; i < count; i++)
			dest[i] = source[i] + pal;
	}
}

static void cps_tm_scanline_masked(uint16_t *dest, const uint16_t *source, const uint8_t *maskptr,
		int mask, int value, int count, uint8_t *pri, uint32_t pcode)
{
	int pal = pcode >> 16;
	int i = 0;
	int has_pri = ((pcode & 0xffff) != 0xff00);

#if defined(CPS_TM_HAVE_SSE2)
	{
		__m128i vpal = _mm_set1_epi16((short)pal);
		__m128i vmaskb = _mm_set1_epi8((char)(mask & 0xff));
		__m128i vvalb = _mm_set1_epi8((char)(value & 0xff));
		__m128i va = _mm_set1_epi8((char)(pcode >> 8)), vo = _mm_set1_epi8((char)pcode);
		for ( ; i + 16 <= count; i += 16)
		{
			__m128i mb = _mm_loadu_si128((const __m128i*)(maskptr+i));
			__m128i testb = _mm_cmpeq_epi8(_mm_and_si128(mb, vmaskb), vvalb);
			__m128i tlo = _mm_unpacklo_epi8(testb, testb);
			__m128i thi = _mm_unpackhi_epi8(testb, testb);
			__m128i n0 = _mm_add_epi16(_mm_loadu_si128((const __m128i*)(source+i)), vpal);
			__m128i n1 = _mm_add_epi16(_mm_loadu_si128((const __m128i*)(source+i+8)), vpal);
			__m128i o0 = _mm_loadu_si128((const __m128i*)(dest+i));
			__m128i o1 = _mm_loadu_si128((const __m128i*)(dest+i+8));
			_mm_storeu_si128((__m128i*)(dest+i),   _mm_or_si128(_mm_and_si128(tlo, n0), _mm_andnot_si128(tlo, o0)));
			_mm_storeu_si128((__m128i*)(dest+i+8), _mm_or_si128(_mm_and_si128(thi, n1), _mm_andnot_si128(thi, o1)));
			if (has_pri)
			{
				__m128i p = _mm_loadu_si128((const __m128i*)(pri+i));
				__m128i np = _mm_or_si128(_mm_and_si128(p, va), vo);
				_mm_storeu_si128((__m128i*)(pri+i), _mm_or_si128(_mm_and_si128(testb, np), _mm_andnot_si128(testb, p)));
			}
		}
	}
#elif defined(CPS_TM_HAVE_NEON)
	{
		uint16x8_t vpal = vdupq_n_u16((uint16_t)pal);
		uint8x16_t vmaskb = vdupq_n_u8((uint8_t)(mask & 0xff));
		uint8x16_t vvalb = vdupq_n_u8((uint8_t)(value & 0xff));
		uint8x16_t va = vdupq_n_u8((uint8_t)(pcode >> 8)), vo = vdupq_n_u8((uint8_t)pcode);
		for ( ; i + 16 <= count; i += 16)
		{
			uint8x16_t mb = vld1q_u8(maskptr+i);
			uint8x16_t testb = vceqq_u8(vandq_u8(mb, vmaskb), vvalb);
			uint8x16x2_t z = vzipq_u8(testb, testb);
			uint16x8_t tlo = vreinterpretq_u16_u8(z.val[0]);
			uint16x8_t thi = vreinterpretq_u16_u8(z.val[1]);
			uint16x8_t n0 = vaddq_u16(vld1q_u16(source+i), vpal);
			uint16x8_t n1 = vaddq_u16(vld1q_u16(source+i+8), vpal);
			vst1q_u16(dest+i,   vbslq_u16(tlo, n0, vld1q_u16(dest+i)));
			vst1q_u16(dest+i+8, vbslq_u16(thi, n1, vld1q_u16(dest+i+8)));
			if (has_pri)
			{
				uint8x16_t p = vld1q_u8(pri+i);
				uint8x16_t np = vorrq_u8(vandq_u8(p, va), vo);
				vst1q_u8(pri+i, vbslq_u8(testb, np, p));
			}
		}
	}
#endif

	if (has_pri)
	{
		for ( ; i < count; i++)
			if ((maskptr[i] & mask) == value)
			{
				dest[i] = source[i] + pal;
				pri[i] = (pri[i] & (pcode >> 8)) | pcode;
			}
	}
	else
	{
		for ( ; i < count; i++)
			if ((maskptr[i] & mask) == value)
				dest[i] = source[i] + pal;
	}
}

/*-------------------------------------------------
    cps_tm_draw_instance - run-length composite of
    one tilemap instance at (xpos,ypos)
-------------------------------------------------*/

static void cps_tm_draw_instance(cps_tm *t, running_machine *machine, bitmap_t *dest,
		bitmap_t *prio, const rectangle *clip, uint8_t mask, uint8_t value, uint32_t pcode, int xpos, int ypos)
{
	const uint16_t *source_baseaddr;
	const uint8_t *mask_baseaddr;
	void *dest_baseaddr;
	uint8_t *priority_baseaddr;
	int dest_line_pitch_bytes, dest_bytespp;
	int mincol, maxcol;
	int x1, y1, x2, y2;
	int y, nexty;

	x1 = MAX(xpos, clip->min_x);
	x2 = MIN(xpos + (int)t->width, clip->max_x + 1);
	y1 = MAX(ypos, clip->min_y);
	y2 = MIN(ypos + (int)t->height, clip->max_y + 1);
	if (x1 >= x2 || y1 >= y2)
		return;

	priority_baseaddr = BITMAP_ADDR8(prio, y1, xpos);
	dest_bytespp = dest->bpp / 8;
	dest_line_pitch_bytes = dest->rowpixels * dest_bytespp;
	dest_baseaddr = (uint8_t *)dest->base + (y1 * dest->rowpixels + xpos) * dest_bytespp;

	x1 -= xpos;
	y1 -= ypos;
	x2 -= xpos;
	y2 -= ypos;

	source_baseaddr = BITMAP_ADDR16(t->pixmap, y1, 0);
	mask_baseaddr = BITMAP_ADDR8(t->flagsmap, y1, 0);

	mincol = x1 / t->tilewidth;
	maxcol = (x2 + t->tilewidth - 1) / t->tilewidth;

	y = y1;
	nexty = t->tileheight * (y1 / t->tileheight) + t->tileheight;
	nexty = MIN(nexty, y2);

	for (;;)
	{
		int row = y / t->tileheight;
		int prev_trans = CPS_WHOLLY_TRANSPARENT;
		int cur_trans;
		int x_start = x1;
		int column;

		for (column = mincol; column <= maxcol; column++)
		{
			int x_end;

			if (column == maxcol)
				cur_trans = CPS_WHOLLY_TRANSPARENT;
			else
			{
				int logindex = row * t->cols + column;

				if (t->tileflags[logindex] == CPS_TM_FLAG_DIRTY)
					cps_tm_tile_update(t, machine, logindex, column, row);

				if ((t->tileflags[logindex] & mask) != 0)
					cur_trans = CPS_MASKED;
				else
					cur_trans = ((mask_baseaddr[column * t->tilewidth] & mask) == value) ? CPS_WHOLLY_OPAQUE : CPS_WHOLLY_TRANSPARENT;
			}

			if (cur_trans == prev_trans)
				continue;

			x_end = column * t->tilewidth;
			x_end = MAX(x_end, x1);
			x_end = MIN(x_end, x2);

			if (prev_trans != CPS_WHOLLY_TRANSPARENT)
			{
				const uint16_t *source0 = source_baseaddr + x_start;
				void *dest0 = (uint8_t *)dest_baseaddr + x_start * dest_bytespp;
				uint8_t *pmap0 = priority_baseaddr + x_start;
				int cury;

				if (prev_trans == CPS_WHOLLY_OPAQUE)
				{
					for (cury = y; cury < nexty; cury++)
					{
						cps_tm_scanline_opaque((uint16_t *)dest0, source0, x_end - x_start, pmap0, pcode);
						dest0 = (uint8_t *)dest0 + dest_line_pitch_bytes;
						source0 += t->pixmap->rowpixels;
						pmap0 += prio->rowpixels;
					}
				}
				else
				{
					const uint8_t *mask0 = mask_baseaddr + x_start;
					for (cury = y; cury < nexty; cury++)
					{
						cps_tm_scanline_masked((uint16_t *)dest0, source0, mask0, mask, value, x_end - x_start, pmap0, pcode);
						dest0 = (uint8_t *)dest0 + dest_line_pitch_bytes;
						source0 += t->pixmap->rowpixels;
						mask0 += t->flagsmap->rowpixels;
						pmap0 += prio->rowpixels;
					}
				}
			}

			x_start = x_end;
			prev_trans = cur_trans;
		}

		if (nexty == y2)
			break;

		priority_baseaddr += prio->rowpixels * (nexty - y);
		source_baseaddr += t->pixmap->rowpixels * (nexty - y);
		mask_baseaddr += t->flagsmap->rowpixels * (nexty - y);
		dest_baseaddr = (uint8_t *)dest_baseaddr + dest_line_pitch_bytes * (nexty - y);

		y = nexty;
		nexty += t->tileheight;
		nexty = MIN(nexty, y2);
	}
}

/*-------------------------------------------------
    public API
-------------------------------------------------*/

cps_tm *cps_tm_create(running_machine *machine, cps_tm_get_info_func get_info,
		cps_tm_scan_func scan, int tilewidth, int tileheight, int cols, int rows)
{
	cps_tm *t = auto_alloc_clear(machine, cps_tm);
	int c, r;

	t->cols = cols;
	t->rows = rows;
	t->tilewidth = tilewidth;
	t->tileheight = tileheight;
	t->width = cols * tilewidth;
	t->height = rows * tileheight;
	t->get_info = get_info;
	t->scan = scan;
	t->enable = TRUE;
	t->all_tiles_dirty = TRUE;
	t->scrollrows = 1;
	t->scrollcols = 1;

	t->logical_to_memory = auto_alloc_array(machine, int, cols * rows);
	t->max_memindex = 0;
	for (r = 0; r < rows; r++)
		for (c = 0; c < cols; c++)
		{
			int mi = (*scan)(c, r);
			t->logical_to_memory[r * cols + c] = mi;
			if (mi > t->max_memindex)
				t->max_memindex = mi;
		}

	/* reverse map for O(1) mark_tile_dirty on the hot VRAM-write path */
	t->memory_to_logical = auto_alloc_array(machine, int, t->max_memindex + 1);
	for (c = 0; c <= t->max_memindex; c++)
		t->memory_to_logical[c] = -1;
	for (r = 0; r < rows; r++)
		for (c = 0; c < cols; c++)
			t->memory_to_logical[t->logical_to_memory[r * cols + c]] = r * cols + c;

	t->pen_to_flags = auto_alloc_array_clear(machine, uint8_t, CPS_TM_MAX_PEN_TO_FLAGS * CPS_TM_NUM_GROUPS);
	t->tileflags = auto_alloc_array(machine, uint8_t, cols * rows);
	memset(t->tileflags, CPS_TM_FLAG_DIRTY, cols * rows);

	t->pixmap = auto_bitmap_alloc(machine, t->width, t->height, BITMAP_FORMAT_INDEXED16);
	t->flagsmap = auto_bitmap_alloc(machine, t->width, t->height, BITMAP_FORMAT_INDEXED8);

	t->rowscroll = auto_alloc_array_clear(machine, int32_t, t->height);
	t->colscroll = auto_alloc_array_clear(machine, int32_t, t->width);

	return t;
}

void cps_tm_dispose(cps_tm *t)
{
	/* all allocations are from the machine pool and freed with it */
	(void)t;
}

void cps_tm_set_transmask(cps_tm *t, int group, uint32_t fgmask, uint32_t bgmask)
{
	uint8_t *array = t->pen_to_flags + group * CPS_TM_MAX_PEN_TO_FLAGS;
	int pen;

	for (pen = 0; pen < 32; pen++)
	{
		uint8_t fgbits = ((fgmask >> pen) & 1) ? TILEMAP_PIXEL_TRANSPARENT : TILEMAP_PIXEL_LAYER0;
		uint8_t bgbits = ((bgmask >> pen) & 1) ? TILEMAP_PIXEL_TRANSPARENT : TILEMAP_PIXEL_LAYER1;
		array[pen] = fgbits | bgbits;
	}
	t->all_tiles_dirty = TRUE;
}

void cps_tm_set_scroll_rows(cps_tm *t, uint32_t scroll_rows)
{
	t->scrollrows = scroll_rows;
}

void cps_tm_set_scrollx(cps_tm *t, int which, int value)
{
	if ((uint32_t)which < t->scrollrows)
		t->rowscroll[which] = value;
}

void cps_tm_set_scrolly(cps_tm *t, int which, int value)
{
	if ((uint32_t)which < t->scrollcols)
		t->colscroll[which] = value;
}

void cps_tm_set_enable(cps_tm *t, int enable)
{
	t->enable = enable ? TRUE : FALSE;
}

void cps_tm_set_flip(cps_tm *t, uint32_t attributes)
{
	if (t->attributes != (uint8_t)attributes)
	{
		t->attributes = (uint8_t)attributes;
		t->all_tiles_dirty = TRUE;
	}
}

void cps_tm_mark_tile_dirty(cps_tm *t, int memindex)
{
	/* O(1) reverse lookup; the CPS map is 1:1 so a memory index maps to a
	   single logical cell (or none, if out of range) */
	if (memindex >= 0 && memindex <= t->max_memindex)
	{
		int logindex = t->memory_to_logical[memindex];
		if (logindex >= 0)
			t->tileflags[logindex] = CPS_TM_FLAG_DIRTY;
	}
}

void cps_tm_mark_all_dirty(cps_tm *t)
{
	t->all_tiles_dirty = TRUE;
}

void cps_tm_draw(cps_tm *t, running_machine *machine, bitmap_t *dest,
		const rectangle *cliprect, uint32_t flags, uint8_t priority)
{
	bitmap_t *prio = machine->priority_bitmap;
	int screen_w = machine->primary_screen->width();
	int screen_h = machine->primary_screen->height();
	uint8_t mask, value;
	uint32_t pcode;
	int xpos, ypos;

	if (!t->enable)
		return;

	if (t->all_tiles_dirty)
	{
		memset(t->tileflags, CPS_TM_FLAG_DIRTY, t->cols * t->rows);
		t->all_tiles_dirty = FALSE;
	}

	/* configure mask/value the way the core's configure_blit_parameters does
	   for the LAYER0 / LAYER1 draw flags CPS uses */
	mask = TILEMAP_PIXEL_CATEGORY_MASK;
	value = 0;
	mask |= flags & (CPS_TM_DRAW_LAYER0 | CPS_TM_DRAW_LAYER1);
	value |= flags & (CPS_TM_DRAW_LAYER0 | CPS_TM_DRAW_LAYER1);

	/* CPS never sets a palette offset, so the palette field of pcode is 0;
	   priority_mask is always 0xff for the CPS draw calls */
	pcode = priority | (0xff << 8);

	if (t->scrollrows == 1 && t->scrollcols == 1)
	{
		int scrollx = cps_tm_eff_rowscroll(t, 0, screen_w);
		int scrolly = cps_tm_eff_colscroll(t, 0, screen_h);

		for (ypos = scrolly - t->height; ypos <= cliprect->max_y; ypos += t->height)
			for (xpos = scrollx - t->width; xpos <= cliprect->max_x; xpos += t->width)
				cps_tm_draw_instance(t, machine, dest, prio, cliprect, mask, value, pcode, xpos, ypos);
	}
	else if (t->scrollcols == 1)
	{
		rectangle orig = *cliprect;
		int rowheight = t->height / t->scrollrows;
		int scrolly = cps_tm_eff_colscroll(t, 0, screen_h);
		int currow, nextrow;

		for (ypos = scrolly - t->height; ypos <= orig.max_y; ypos += t->height)
		{
			int firstrow = MAX((orig.min_y - ypos) / rowheight, 0);
			int lastrow = MIN((orig.max_y - ypos) / rowheight, (int)t->scrollrows - 1);

			for (currow = firstrow; currow <= lastrow; currow = nextrow)
			{
				int scrollx = cps_tm_eff_rowscroll(t, currow, screen_w);
				rectangle rc;

				for (nextrow = currow + 1; nextrow <= lastrow; nextrow++)
					if (cps_tm_eff_rowscroll(t, nextrow, screen_w) != scrollx)
						break;

				rc.min_y = MAX(currow * rowheight + ypos, orig.min_y);
				rc.max_y = MIN(nextrow * rowheight - 1 + ypos, orig.max_y);
				rc.min_x = orig.min_x;
				rc.max_x = orig.max_x;

				for (xpos = scrollx - t->width; xpos <= orig.max_x; xpos += t->width)
					cps_tm_draw_instance(t, machine, dest, prio, &rc, mask, value, pcode, xpos, ypos);
			}
		}
	}
}
