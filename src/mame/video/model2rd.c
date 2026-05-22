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
	const uint16_t *lumaram = (const uint16_t *)model2_lumaram;
	const uint16_t *palram = (const uint16_t *)model2_paletteram32;
	uint32_t	lumabase = extra->lumabase;
	uint32_t	color = extra->colorbase;
	uint8_t	luma;
	uint32_t	tr, tg, tb;
	int		x;
#endif
	/* if it's translucent, there's nothing to render */
#if defined( MODEL2_TRANSLUCENT)
	return;
#else

	luma = lumaram[BYTE_XOR_LE(lumabase + (0xf << 3))] & 0x3F;

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

	uint32_t	tex_width = extra->texwidth;
	uint32_t	tex_height = extra->texheight;

	/* extract color information */
	const uint16_t *colortable_r = (const uint16_t *)&model2_colorxlat[0x0000/4];
	const uint16_t *colortable_g = (const uint16_t *)&model2_colorxlat[0x4000/4];
	const uint16_t *colortable_b = (const uint16_t *)&model2_colorxlat[0x8000/4];
	const uint16_t *lumaram = (const uint16_t *)model2_lumaram;
	const uint16_t *palram = (const uint16_t *)model2_paletteram32;
	uint32_t	colorbase = extra->colorbase;
	uint32_t	lumabase = extra->lumabase;
	uint32_t	tex_x = extra->texx;
	uint32_t	tex_y = extra->texy;
	uint32_t	tex_x_mask, tex_y_mask;
	uint32_t	tex_mirr_x = extra->texmirrorx;
	uint32_t	tex_mirr_y = extra->texmirrory;
	uint32_t *sheet = extra->texsheet;
	float ooz = extent->param[0].start;
	float uoz = extent->param[1].start;
	float voz = extent->param[2].start;
	float dooz = extent->param[0].dpdx;
	float duoz = extent->param[1].dpdx;
	float dvoz = extent->param[2].dpdx;
	int		x;

	tex_x_mask	= tex_width - 1;
	tex_y_mask	= tex_height - 1;

	colorbase = palram[BYTE_XOR_LE(colorbase + 0x1000)] & 0x7fff;

	colortable_r += ((colorbase >>  0) & 0x1f) << 8;
	colortable_g += ((colorbase >>  5) & 0x1f) << 8;
	colortable_b += ((colorbase >> 10) & 0x1f) << 8;

	for(x = extent->startx; x < extent->stopx; x++, uoz += duoz, voz += dvoz, ooz += dooz)
	{
		float z = recip_approx(ooz) * 256.0f;
		int32_t u = uoz * z;
		int32_t v = voz * z;
		uint32_t	tr, tg, tb;
		uint16_t	t;
		uint8_t luma;
		int u2;
		int v2;

#if defined(MODEL2_CHECKER)
		if ( ((x^scanline) & 1) == 0 )
			continue;
#endif
		u2 = (u >> 8) & tex_x_mask;
		v2 = (v >> 8) & tex_y_mask;

		if ( tex_mirr_x )
			u2 = ( tex_width - 1 ) - u2;

		if ( tex_mirr_y )
			v2 = ( tex_height - 1 ) - v2;

		t = get_texel( tex_x, tex_y, u2, v2, sheet );

#if defined(MODEL2_TRANSLUCENT)
		if ( t == 0x0f )
			continue;
#endif
		luma = lumaram[BYTE_XOR_LE(lumabase + (t << 3))] & 0x3f;

		/* we have the 6 bits of luma information along with 5 bits per color component */
		/* now build and index into the master color lookup table and extract the raw RGB values */

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
