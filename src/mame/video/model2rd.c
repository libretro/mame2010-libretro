/********************************************************************

    Sega Model 2 3D rasterization functions

********************************************************************/

#undef MODEL2_CHECKER
#undef MODEL2_TEXTURED
#undef MODEL2_TRANSLUCENT

#ifndef MODEL2_FUNC
#error "Model 2 renderer: No function defined!"
#endif

#ifndef MODEL2_FUNC_NAME
#error "Model 2 renderer: No function name defined!"
#endif

#if MODEL2_FUNC == 0
#undef MODEL2_CHECKER
#undef MODEL2_TEXTURED
#undef MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 1
#undef MODEL2_CHECKER
#undef MODEL2_TEXTURED
#define MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 2
#undef MODEL2_CHECKER
#define MODEL2_TEXTURED
#undef MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 3
#undef MODEL2_CHECKER
#define MODEL2_TEXTURED
#define MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 4
#define MODEL2_CHECKER
#undef MODEL2_TEXTURED
#undef MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 5
#define MODEL2_CHECKER
#undef MODEL2_TEXTURED
#define MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 6
#define MODEL2_CHECKER
#define MODEL2_TEXTURED
#undef MODEL2_TRANSLUCENT
#elif MODEL2_FUNC == 7
#define MODEL2_CHECKER
#define MODEL2_TEXTURED
#define MODEL2_TRANSLUCENT
#else
#error "Model 2 renderer: Invalif function selected!"
#endif

#ifndef MODEL2_TEXTURED
/* non-textured render path */
static void MODEL2_FUNC_NAME(void *dest, int32_t scanline, const poly_extent *extent, const void *extradata, int threadid)
{
#if !defined( MODEL2_TRANSLUCENT)
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_t *destmap = (bitmap_t *)dest;
	uint32_t *p = BITMAP_ADDR32(destmap, scanline, 0);

	/* extract color information */
	const uint16_t *colortable_r = (const uint16_t *)&model2_colorxlat[0x0000/4];
	const uint16_t *colortable_g = (const uint16_t *)&model2_colorxlat[0x4000/4];
	const uint16_t *colortable_b = (const uint16_t *)&model2_colorxlat[0x8000/4];
	const uint16_t *palram = (const uint16_t *)model2_paletteram32;
	uint32_t	color = extra->colorbase;
	uint8_t	luma;
	uint32_t	tr, tg, tb;
	int		x;
#endif
	/* if it's translucent, there's nothing to render */
#if defined( MODEL2_TRANSLUCENT)
	return;
#else

	/* flat-shaded polygons use the per-polygon luma directly (shifted to
	 * 6 bits to match the colortable index width); the previous fixed
	 * (0xf << 3) lumaram lookup produced the same brightness for every
	 * solid polygon regardless of lighting. */
	luma = extra->luma >> 2;

	color = palram[BYTE_XOR_LE(color + 0x1000)] & 0x7fff;

	colortable_r += ((color >>  0) & 0x1f) << 8;
	colortable_g += ((color >>  5) & 0x1f) << 8;
	colortable_b += ((color >> 10) & 0x1f) << 8;

	/* we have the 6 bits of luma information along with 5 bits per color component */
	/* now build and index into the master color lookup table and extract the raw RGB values */

	tr = colortable_r[BYTE_XOR_LE(luma)] & 0xff;
	tg = colortable_g[BYTE_XOR_LE(luma)] & 0xff;
	tb = colortable_b[BYTE_XOR_LE(luma)] & 0xff;

	tr = model2_gamma_table[tr];
	tg = model2_gamma_table[tg];
	tb = model2_gamma_table[tb];

	/* build the final color */
	color = MAKE_RGB(tr, tg, tb);

	for(x = extent->startx; x < extent->stopx; x++)
#if defined(MODEL2_CHECKER)
		if ((x^scanline) & 1) p[x] = color;
#else
		p[x] = color;
#endif
#endif
}

#else
/* textured render path */
static void MODEL2_FUNC_NAME(void *dest, int32_t scanline, const poly_extent *extent, const void *extradata, int threadid)
{
	const poly_extra_data *extra = (const poly_extra_data *)extradata;
	bitmap_t *destmap = (bitmap_t *)dest;
	uint32_t *p = BITMAP_ADDR32(destmap, scanline, 0);

	/* extract color information */
	const uint16_t *colortable_r = (const uint16_t *)&model2_colorxlat[0x0000/4];
	const uint16_t *colortable_g = (const uint16_t *)&model2_colorxlat[0x4000/4];
	const uint16_t *colortable_b = (const uint16_t *)&model2_colorxlat[0x8000/4];
	const uint16_t *lumaram = (const uint16_t *)model2_lumaram;
	const uint16_t *palram = (const uint16_t *)model2_paletteram32;
	uint32_t	colorbase = extra->colorbase;
	uint32_t	lumabase = extra->lumabase;
	uint32_t	poly_luma = extra->luma;
	float ooz = extent->param[0].start;
	float uoz = extent->param[1].start;
	float voz = extent->param[2].start;
	float dooz = extent->param[0].dpdx;
	float duoz = extent->param[1].dpdx;
	float dvoz = extent->param[2].dpdx;
	/* maximum mip level: go down to 2x2 (clz of min(w,h) tells us how
	 * many doublings we need; 30 because clz(2) = 30 for the 32-bit
	 * representation we use) */
	uint32_t	min_dim = (extra->texwidth < extra->texheight) ? extra->texwidth : extra->texheight;
	int32_t		max_level = 30 - model2_clz32(min_dim);
	int		x;
#if defined(MODEL2_TRANSLUCENT)
	const int	translucent = 1;
#else
	const int	translucent = 0;
#endif

	colorbase = palram[BYTE_XOR_LE(colorbase + 0x1000)] & 0x7fff;

	colortable_r += ((colorbase >>  0) & 0x1f) << 8;
	colortable_g += ((colorbase >>  5) & 0x1f) << 8;
	colortable_b += ((colorbase >> 10) & 0x1f) << 8;

	for(x = extent->startx; x < extent->stopx; x++, uoz += duoz, voz += dvoz, ooz += dooz)
	{
		float    z;
		int32_t  u, v;
		int32_t  mml, level, frac;
		uint32_t t, t2;
		uint32_t lv;
		uint32_t tr, tg, tb;
		uint8_t  luma;

#if defined(MODEL2_CHECKER)
		if ( ((x^scanline) & 1) == 0 )
			continue;
#endif
		/* perspective-correct: 1/(1/z) -> z, then derive a mip level
		 * from log2(z) and the polygon's texlod offset.  Larger z
		 * (farther) -> higher mip level -> smaller texture. */
		z = 1.0f / ooz;
		mml = -extra->texlod + model2_fast_log2(z);
		level = mml >> 7;
		if (level < 0) level = 0;
		if (level > max_level) level = max_level;

		/* texture coords as 24.8 fixed point */
		u = (int32_t)(uoz * z * 256.0f);
		v = (int32_t)(voz * z * 256.0f);

		t = fetch_bilinear_texel(extra, level, u, v, translucent);

		/* if we're not at the smallest level and the LOD calculation
		 * has fractional bits, blend with the next coarser level for
		 * trilinear-style smoothing between mips */
		if (mml > 0 && level < max_level)
		{
			t2 = fetch_bilinear_texel(extra, level + 1, u, v, translucent);
			frac = (mml & 127) << 1;
			t = (uint32_t)model2_lerp((int32_t)t, (int32_t)t2, (uint32_t)frac);
		}

#if defined(MODEL2_TRANSLUCENT)
		/* alpha under 50% -> drop the pixel */
		if (t < 0x00400000u)
			continue;
		/* strip the alpha tag before consuming as a luma index */
		t &= 0xff;
#endif

		/* bilinear gives an 8-bit smooth texel value; the luma table
		 * has 128 (7-bit) entries per texture, so the lookup index is
		 * (t >> 1).  Then scale by the per-polygon luma to apply the
		 * geometry engine's continuous shading and clamp to the 6-bit
		 * colortable index range. */
		lv = (uint32_t)(lumaram[BYTE_XOR_LE(lumabase + (t >> 1))] & 0x3f);
		lv = (lv * poly_luma) >> 8;
		if (lv > 0x3f) lv = 0x3f;
		luma = (uint8_t)lv;

		tr = colortable_r[BYTE_XOR_LE(luma)] & 0xff;
		tg = colortable_g[BYTE_XOR_LE(luma)] & 0xff;
		tb = colortable_b[BYTE_XOR_LE(luma)] & 0xff;

		tr = model2_gamma_table[tr];
		tg = model2_gamma_table[tg];
		tb = model2_gamma_table[tb];

		p[x] = MAKE_RGB(tr, tg, tb);
	}
}

#endif
