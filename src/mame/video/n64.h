#ifndef _VIDEO_N64_H_
#define _VIDEO_N64_H_

#include "emu.h"
#include "video/rdpblend.h"
#include "video/rdptri.h"
#include "video/rdpfb.h"
#include "video/rdpfrect.h"
#include "video/rdptrect.h"
#include "video/rdptpipe.h"
#include "video/rdpspn16.h"
#include "video/rdpfetch.h"

/*****************************************************************************/

#define PIXEL_SIZE_4BIT			0
#define PIXEL_SIZE_8BIT			1
#define PIXEL_SIZE_16BIT		2
#define PIXEL_SIZE_32BIT		3

#define CYCLE_TYPE_1			0
#define CYCLE_TYPE_2			1
#define CYCLE_TYPE_COPY			2
#define CYCLE_TYPE_FILL			3

#define SAMPLE_TYPE_1x1         0
#define SAMPLE_TYPE_2x2         1

#define BYTE_ADDR_XOR		BYTE4_XOR_BE(0)
#define WORD_ADDR_XOR		(WORD_XOR_BE(0) >> 1)

#define XOR_SWAP_BYTE_SHIFT		2
#define XOR_SWAP_WORD_SHIFT		1
#define XOR_SWAP_DWORD_SHIFT	0

#define XOR_SWAP_BYTE	4
#define XOR_SWAP_WORD	2
#define XOR_SWAP_DWORD	1

//sign-extension macros
#define SIGN17(x)	(((x) & 0x10000) ? ((x) | ~0x1ffff) : ((x) & 0x1ffff))
#define SIGN16(x)	(((x) & 0x8000) ? ((x) | ~0xffff) : ((x) & 0xffff))
#define SIGN11(x)	(((x) & 0x400) ? ((x) | ~0x7ff) : ((x) & 0x7ff))

/*****************************************************************************/

namespace N64
{

namespace RDP
{

class Processor;
class MiscState;
class OtherModes;

class Color
{
	public:
		Color()
		{
			c = 0;
		}

		union
		{
			uint32_t c;
#ifdef MSB_FIRST
			struct { uint8_t r, g, b, a; } i;
#else
			struct { uint8_t a, b, g, r; } i;
#endif
		};
};

enum
{
	BIT_DEPTH_32 = 0,
	BIT_DEPTH_16,

	BIT_DEPTH_COUNT
};

class Tile
{
	public:
		int format;	// Image data format: RGBA, YUV, CI, IA, I
		int size; // Size of texel element: 4b, 8b, 16b, 32b
		uint32_t line; // Size of tile line in bytes
		uint32_t tmem; // Starting tmem address for this tile in bytes
		int palette; // Palette number for 4b CI texels
		int ct, mt, cs, ms; // Clamp / mirror enable bits for S / T direction
		int mask_t, shift_t, mask_s, shift_s; // Mask values / LOD shifts
		uint16_t sl, tl, sh, th;		// 10.2 fixed-point, starting and ending texel row / column
		int num;
};

class MiscState
{
	public:
		MiscState()
		{
			m_curpixel_cvg = 0;
			m_curpixel_overlap = 0;

			m_max_level = 0;
			m_min_level = 0;
		}

		int m_fb_format;
		int m_fb_size;
		int m_fb_width;
		int m_fb_height;
		uint32_t m_fb_address;

		uint32_t m_zb_address;

		int m_ti_format;
		int m_ti_size;
		int m_ti_width;
		uint32_t m_ti_address;

		uint32_t m_curpixel_cvg;
		uint32_t m_curpixel_overlap;

		uint8_t m_random_seed;

		int m_special_bsel0;
		int m_special_bsel1;

		uint32_t m_max_level;
		uint32_t m_min_level;

		uint16_t m_primitive_z;
		uint16_t m_primitive_delta_z;
};

class CombineModes
{
	public:
		int sub_a_rgb0;
		int sub_b_rgb0;
		int mul_rgb0;
		int add_rgb0;
		int sub_a_a0;
		int sub_b_a0;
		int mul_a0;
		int add_a0;

		int sub_a_rgb1;
		int sub_b_rgb1;
		int mul_rgb1;
		int add_rgb1;
		int sub_a_a1;
		int sub_b_a1;
		int mul_a1;
		int add_a1;
};

class OtherModes
{
	public:
		int cycle_type;
		bool persp_tex_en;
		bool detail_tex_en;
		bool sharpen_tex_en;
		bool tex_lod_en;
		bool en_tlut;
		bool tlut_type;
		bool sample_type;
		bool mid_texel;
		bool bi_lerp0;
		bool bi_lerp1;
		bool convert_one;
		bool key_en;
		int rgb_dither_sel;
		int alpha_dither_sel;
		int blend_m1a_0;
		int blend_m1a_1;
		int blend_m1b_0;
		int blend_m1b_1;
		int blend_m2a_0;
		int blend_m2a_1;
		int blend_m2b_0;
		int blend_m2b_1;
		int tex_edge;
		bool force_blend;
		bool alpha_cvg_select;
		bool cvg_times_alpha;
		int z_mode;
		int cvg_dest;
		bool color_on_cvg;
		bool image_read_en;
		bool z_update_en;
		bool z_compare_en;
		bool antialias_en;
		bool z_source_sel;
		bool dither_alpha_en;
		bool alpha_compare_en;
};

class ColorInputs
{
	public:
		// combiner inputs
		uint8_t *combiner_rgbsub_a_r[2];
		uint8_t *combiner_rgbsub_a_g[2];
		uint8_t *combiner_rgbsub_a_b[2];
		uint8_t *combiner_rgbsub_b_r[2];
		uint8_t *combiner_rgbsub_b_g[2];
		uint8_t *combiner_rgbsub_b_b[2];
		uint8_t *combiner_rgbmul_r[2];
		uint8_t *combiner_rgbmul_g[2];
		uint8_t *combiner_rgbmul_b[2];
		uint8_t *combiner_rgbadd_r[2];
		uint8_t *combiner_rgbadd_g[2];
		uint8_t *combiner_rgbadd_b[2];

		uint8_t *combiner_alphasub_a[2];
		uint8_t *combiner_alphasub_b[2];
		uint8_t *combiner_alphamul[2];
		uint8_t *combiner_alphaadd[2];

		// blender input
		uint8_t *blender1a_r[2];
		uint8_t *blender1a_g[2];
		uint8_t *blender1a_b[2];
		uint8_t *blender1b_a[2];
		uint8_t *blender2a_r[2];
		uint8_t *blender2a_g[2];
		uint8_t *blender2a_b[2];
		uint8_t *blender2b_a[2];
};

class Processor
{
	public:
		Processor()
		{
			m_cmd_ptr = 0;
			m_cmd_cur = 0;

			m_start = 0;
			m_end = 0;
			m_current = 0;
			m_status = 0x88;

			for(int i = 0; i < (1 << 16); i++)
			{
				uint8_t r = ((i >> 8) & 0xf8) | (i >> 13);
				uint8_t g = ((i >> 3) & 0xf8) | ((i >>  8) & 0x07);
				uint8_t b = ((i << 2) & 0xf8) | ((i >>  3) & 0x07);
				uint8_t a = (i & 1) ? 0xff : 0;
				m_rgb16_to_rgb32_lut[i] = (r << 24) | (g << 16) | (b << 8) | a;

				r = g = b = (i >> 8) & 0xff;
				a = i & 0xff;
				m_ia8_to_rgb32_lut[i] = (r << 24) | (g << 16) | (b << 8) | a;
			}

			for(int i = 0; i < (1 << 24); i++)
			{
				uint8_t A = (i >> 16) & 0x000000ff;
				uint8_t B = (i >> 8) & 0x000000ff;
				uint8_t C = i & 0x000000ff;
				m_cc_lut1[i] = (int16_t)((((((int32_t)A - (int32_t)B) * (int32_t)C) + 0x80) >> 8) & 0x0000ffff);
			}

			for(int i = 0; i < (1 << 16); i++)
			{
				for(int j = 0; j < (1 << 8); j++)
				{
					int32_t temp = (int32_t)((int16_t)i) + j;
					if(temp > 255)
					{
						m_cc_lut2[(i << 8) | j] = 255;
					}
					else if(temp < 0)
					{
						m_cc_lut2[(i << 8) | j] = 0;
					}
					else
					{
						m_cc_lut2[(i << 8) | j] = (uint8_t)temp;
					}
				}
			}

			for (int i = 0; i < 8; i++)
			{
				m_tiles[i].num = i;
			}

			m_one_color.c = 0xffffffff;
			m_zero_color.c = 0x00000000;

			m_color_inputs.combiner_rgbsub_a_r[0] = m_color_inputs.combiner_rgbsub_a_r[1] = &m_one_color.i.r;
			m_color_inputs.combiner_rgbsub_a_g[0] = m_color_inputs.combiner_rgbsub_a_g[1] = &m_one_color.i.g;
			m_color_inputs.combiner_rgbsub_a_b[0] = m_color_inputs.combiner_rgbsub_a_b[1] = &m_one_color.i.b;
			m_color_inputs.combiner_rgbsub_b_r[0] = m_color_inputs.combiner_rgbsub_b_r[1] = &m_one_color.i.r;
			m_color_inputs.combiner_rgbsub_b_g[0] = m_color_inputs.combiner_rgbsub_b_g[1] = &m_one_color.i.g;
			m_color_inputs.combiner_rgbsub_b_b[0] = m_color_inputs.combiner_rgbsub_b_b[1] = &m_one_color.i.b;
			m_color_inputs.combiner_rgbmul_r[0] = m_color_inputs.combiner_rgbmul_r[1] = &m_one_color.i.r;
			m_color_inputs.combiner_rgbmul_g[0] = m_color_inputs.combiner_rgbmul_g[1] = &m_one_color.i.g;
			m_color_inputs.combiner_rgbmul_b[0] = m_color_inputs.combiner_rgbmul_b[1] = &m_one_color.i.b;
			m_color_inputs.combiner_rgbadd_r[0] = m_color_inputs.combiner_rgbadd_r[1] = &m_one_color.i.r;
			m_color_inputs.combiner_rgbadd_g[0] = m_color_inputs.combiner_rgbadd_g[1] = &m_one_color.i.g;
			m_color_inputs.combiner_rgbadd_b[0] = m_color_inputs.combiner_rgbadd_b[1] = &m_one_color.i.b;

			m_color_inputs.combiner_alphasub_a[0] = m_color_inputs.combiner_alphasub_a[1] = &m_one_color.i.a;
			m_color_inputs.combiner_alphasub_b[0] = m_color_inputs.combiner_alphasub_b[1] = &m_one_color.i.a;
			m_color_inputs.combiner_alphamul[0] = m_color_inputs.combiner_alphamul[1] = &m_one_color.i.a;
			m_color_inputs.combiner_alphaadd[0] = m_color_inputs.combiner_alphaadd[1] = &m_one_color.i.a;

			m_color_inputs.blender1a_r[0] = m_color_inputs.blender1a_r[1] = &m_pixel_color.i.r;
			m_color_inputs.blender1a_g[0] = m_color_inputs.blender1a_g[1] = &m_pixel_color.i.r;
			m_color_inputs.blender1a_b[0] = m_color_inputs.blender1a_b[1] = &m_pixel_color.i.r;
			m_color_inputs.blender1b_a[0] = m_color_inputs.blender1b_a[1] = &m_pixel_color.i.r;
			m_color_inputs.blender2a_r[0] = m_color_inputs.blender2a_r[1] = &m_pixel_color.i.r;
			m_color_inputs.blender2a_g[0] = m_color_inputs.blender2a_g[1] = &m_pixel_color.i.r;
			m_color_inputs.blender2a_b[0] = m_color_inputs.blender2a_b[1] = &m_pixel_color.i.r;
			m_color_inputs.blender2b_a[0] = m_color_inputs.blender2b_a[1] = &m_pixel_color.i.r;

			m_tmem = NULL;

			m_machine = NULL;

			memset(m_hidden_bits, 3, 4194304); // Hack / fix for letters in Rayman 2

			m_prim_lod_frac = 0;
			m_lod_frac = 0;

			for (int i = 0; i < 256; i++)
			{
				m_gamma_table[i] = sqrt((float)(i << 6));
				m_gamma_table[i] <<= 1;
			}

			for (int i = 0; i < 0x4000; i++)
			{
				m_gamma_dither_table[i] = sqrt((float)i);
				m_gamma_dither_table[i] <<= 1;
			}

			BuildCompressedZTable();
		}

		~Processor() { }

		void	Dasm(char *buffer);

		void	ProcessList();
		uint32_t	ReadData(uint32_t address);

		void	InitInternalState()
		{
			if(m_machine)
			{
				m_tmem = auto_alloc_array(m_machine, uint8_t, 0x1000);
				memset(m_tmem, 0, 0x1000);

				uint8_t *normpoint = memory_region(m_machine, "normpoint");
				uint8_t *normslope = memory_region(m_machine, "normslope");

				for(int i = 0; i < 64; i++)
				{
					m_norm_point_rom[i] = (normpoint[(i << 1) + 1] << 8) | normpoint[i << 1];
					m_norm_slope_rom[i] = (normslope[(i << 1) + 1] << 8) | normslope[i << 1];
				}
			}
		}

		void		SetMachine(running_machine* machine) { m_machine = machine; }

		// CPU-visible registers
		void		SetStartReg(uint32_t val) { m_start = val; }
		uint32_t		GetStartReg() const { return m_start; }

		void		SetEndReg(uint32_t val) { m_end = val; }
		uint32_t		GetEndReg() const { return m_end; }

		void		SetCurrentReg(uint32_t val) { m_current = val; }
		uint32_t		GetCurrentReg() const { return m_current; }

		void		SetStatusReg(uint32_t val) { m_status = val; }
		uint32_t		GetStatusReg() const { return m_status; }

		// Functional blocks
		Blender*		GetBlender() { return &m_blender; }
		Framebuffer*	GetFramebuffer() { return &m_framebuffer; }
		TexturePipe*	GetTexPipe() { return &m_tex_pipe; }

		// Internal state
		OtherModes*		GetOtherModes() { return &m_other_modes; }
		ColorInputs*	GetColorInputs() { return &m_color_inputs; }
		CombineModes*	GetCombine() { return &m_combine; }
		MiscState*		GetMiscState() { return &m_misc_state; }

		// Color constants
		Color*		GetBlendColor() { return &m_blend_color; }
		void		SetBlendColor(uint32_t color) { m_blend_color.c = color; }

		Color*		GetPixelColor() { return &m_pixel_color; }
		void		SetPixelColor(uint32_t color) { m_pixel_color.c = color; }

		Color*		GetInvPixelColor() { return &m_inv_pixel_color; }
		void		SetInvPixelColor(uint32_t color) { m_inv_pixel_color.c = color; }

		Color*		GetBlendedColor() { return &m_blended_pixel_color; }
		void		SetBlendedColor(uint32_t color) { m_blended_pixel_color.c = color; }

		Color*		GetMemoryColor() { return &m_memory_color; }
		void		SetMemoryColor(uint32_t color) { m_memory_color.c = color; }

		Color*		GetPrimColor() { return &m_prim_color; }
		void		SetPrimColor(uint32_t color) { m_prim_color.c = color; }

		Color*		GetEnvColor() { return &m_env_color; }
		void		SetEnvColor(uint32_t color) { m_env_color.c = color; }

		Color*		GetFogColor() { return &m_fog_color; }
		void		SetFogColor(uint32_t color) { m_fog_color.c = color; }

		Color*		GetCombinedColor() { return &m_combined_color; }
		void		SetCombinedColor(uint32_t color) { m_combined_color.c = color; }

		Color*		GetTexel0Color() { return &m_texel0_color; }
		void		SetTexel0Color(uint32_t color) { m_texel0_color.c = color; }

		Color*		GetTexel1Color() { return &m_texel1_color; }
		void		SetTexel1Color(uint32_t color) { m_texel1_color.c = color; }

		Color*		GetShadeColor() { return &m_shade_color; }
		void		SetShadeColor(uint32_t color) { m_shade_color.c = color; }

		Color*		GetKeyScale() { return &m_key_scale; }
		void		SetKeyScale(uint32_t scale) { m_key_scale.c = scale; }

		Color*		GetNoiseColor() { return &m_noise_color; }
		void		SetNoiseColor(uint32_t color) { m_noise_color.c = color; }

		Color*		GetOne() { return &m_one_color; }
		Color*		GetZero() { return &m_zero_color; }

		uint8_t*		GetLODFrac() { return &m_lod_frac; }
		void		SetLODFrac(uint8_t lod_frac) { m_lod_frac = lod_frac; }

		uint8_t*		GetPrimLODFrac() { return &m_prim_lod_frac; }
		void		SetPrimLODFrac(uint8_t prim_lod_frac) { m_prim_lod_frac = prim_lod_frac; }

		// Color Combiner
		void		SetSubAInputRGB(uint8_t **input_r, uint8_t **input_g, uint8_t **input_b, int code);
		void		SetSubBInputRGB(uint8_t **input_r, uint8_t **input_g, uint8_t **input_b, int code);
		void		SetMulInputRGB(uint8_t **input_r, uint8_t **input_g, uint8_t **input_b, int code);
		void		SetAddInputRGB(uint8_t **input_r, uint8_t **input_g, uint8_t **input_b, int code);
		void		SetSubInputAlpha(uint8_t **input, int code);
		void		SetMulInputAlpha(uint8_t **input, int code);

		// Texture memory
		uint8_t*		GetTMEM() { return m_tmem; }
		uint16_t*		GetTMEM16() { return (uint16_t*)m_tmem; }
		uint32_t*		GetTMEM32() { return (uint32_t*)m_tmem; }
		uint16_t*		GetTLUT() { return (uint16_t*)(m_tmem + 0x800); }
		Tile*		GetTiles(){ return m_tiles; }

		// Emulation Accelerators
		uint32_t		LookUp16To32(uint16_t in) const { return m_rgb16_to_rgb32_lut[in]; }
		uint32_t		LookUpIA8To32(uint16_t in) const { return m_ia8_to_rgb32_lut[in]; }
		uint16_t*		GetCCLUT1() { return m_cc_lut1; }
		uint8_t*		GetCCLUT2() { return m_cc_lut2; }
		uint8_t		GetRandom() { return m_misc_state.m_random_seed += 0x13; }

		// YUV Factors
		void		SetYUVFactors(int32_t k0, int32_t k1, int32_t k2, int32_t k3, int32_t k4, int32_t k5) { m_k0 = k0; m_k1 = k1; m_k2 = k2; m_k3 = k3; m_k4 = k4; m_k5 = k5; }
		int32_t		GetK0() const { return m_k0; }
		int32_t		GetK1() const { return m_k1; }
		int32_t		GetK2() const { return m_k2; }
		int32_t		GetK3() const { return m_k3; }
		int32_t*		GetK4() { return &m_k4; }
		int32_t*		GetK5() { return &m_k5; }

		// Blender-related (move into RDP::Blender)
		void		SetBlenderInput(int cycle, int which, uint8_t **input_r, uint8_t **input_g, uint8_t **input_b, uint8_t **input_a, int a, int b);

		// Render-related (move into eventual drawing-related classes?)
		Rectangle*		GetScissor() { return &m_scissor; }
		void			TCDiv(int32_t ss, int32_t st, int32_t sw, int32_t* sss, int32_t* sst);
		uint32_t			GetLog2(uint32_t lod_clamp);
		void			RenderSpans(int start, int end, int tilenum, bool shade, bool texture, bool zbuffer, bool flip);
		uint8_t*			GetHiddenBits() { return m_hidden_bits; }
		void			GetAlphaCvg(uint8_t *comb_alpha);
		const uint8_t*	GetBayerMatrix() const { return s_bayer_matrix; }
		const uint8_t*	GetMagicMatrix() const { return s_magic_matrix; }
		Span*			GetSpans() { return m_span; }
		int				GetCurrFIFOIndex() const { return m_cmd_cur; }
		uint32_t			GetFillColor32() const { return m_fill_color; }

		void			ZStore(uint16_t* zb, uint8_t* zhb, uint32_t z, uint32_t deltaz);
		uint32_t			DecompressZ(uint16_t *zb);
		uint16_t			DecompressDZ(uint16_t* zb, uint8_t* zhb);
		int32_t			NormalizeDZPix(int32_t sum);
		bool			ZCompare(void* fb, uint8_t* hb, uint16_t* zb, uint8_t* zhb, uint32_t sz, uint16_t dzpix);

		// Fullscreen update-related
		void			VideoUpdate(bitmap_t *bitmap);

		// Commands
		void		CmdInvalid(uint32_t w1, uint32_t w2);
		void		CmdNoOp(uint32_t w1, uint32_t w2);
		void		CmdTriangle(uint32_t w1, uint32_t w2);
		void		CmdTriangleZ(uint32_t w1, uint32_t w2);
		void		CmdTriangleT(uint32_t w1, uint32_t w2);
		void		CmdTriangleTZ(uint32_t w1, uint32_t w2);
		void		CmdTriangleS(uint32_t w1, uint32_t w2);
		void		CmdTriangleSZ(uint32_t w1, uint32_t w2);
		void		CmdTriangleST(uint32_t w1, uint32_t w2);
		void		CmdTriangleSTZ(uint32_t w1, uint32_t w2);
		void		CmdTexRect(uint32_t w1, uint32_t w2);
		void		CmdTexRectFlip(uint32_t w1, uint32_t w2);
		void		CmdSyncLoad(uint32_t w1, uint32_t w2);
		void		CmdSyncPipe(uint32_t w1, uint32_t w2);
		void		CmdSyncTile(uint32_t w1, uint32_t w2);
		void		CmdSyncFull(uint32_t w1, uint32_t w2);
		void		CmdSetKeyGB(uint32_t w1, uint32_t w2);
		void		CmdSetKeyR(uint32_t w1, uint32_t w2);
		void		CmdSetFillColor32(uint32_t w1, uint32_t w2);
		void		CmdSetConvert(uint32_t w1, uint32_t w2);
		void		CmdSetScissor(uint32_t w1, uint32_t w2);
		void		CmdSetPrimDepth(uint32_t w1, uint32_t w2);
		void		CmdSetOtherModes(uint32_t w1, uint32_t w2);
		void		CmdLoadTLUT(uint32_t w1, uint32_t w2);
		void		CmdSetTileSize(uint32_t w1, uint32_t w2);
		void		CmdLoadBlock(uint32_t w1, uint32_t w2);
		void		CmdLoadTile(uint32_t w1, uint32_t w2);
		void		CmdFillRect(uint32_t w1, uint32_t w2);
		void		CmdSetTile(uint32_t w1, uint32_t w2);
		void		CmdSetFogColor(uint32_t w1, uint32_t w2);
		void		CmdSetBlendColor(uint32_t w1, uint32_t w2);
		void		CmdSetPrimColor(uint32_t w1, uint32_t w2);
		void		CmdSetEnvColor(uint32_t w1, uint32_t w2);
		void		CmdSetCombine(uint32_t w1, uint32_t w2);
		void		CmdSetTextureImage(uint32_t w1, uint32_t w2);
		void		CmdSetMaskImage(uint32_t w1, uint32_t w2);
		void		CmdSetColorImage(uint32_t w1, uint32_t w2);

		void		Triangle(bool shade, bool texture, bool zbuffer);
		uint32_t		AddRightCvg(uint32_t x, uint32_t k);
		uint32_t		AddLeftCvg(uint32_t x, uint32_t k);

		uint32_t*		GetCommandData() { return m_cmd_data; }

	protected:
		Blender			m_blender;
		Framebuffer		m_framebuffer;
		TexturePipe		m_tex_pipe;

		OtherModes		m_other_modes;
		MiscState		m_misc_state;
		ColorInputs		m_color_inputs;
		CombineModes	m_combine;

		Color		m_pixel_color;
		Color		m_inv_pixel_color;
		Color		m_blended_pixel_color;
		Color		m_memory_color;
		Color		m_blend_color;

		Color		m_prim_color;
		Color		m_env_color;
		Color		m_fog_color;
		Color		m_combined_color;
		Color		m_texel0_color;
		Color		m_texel1_color;
		Color		m_shade_color;
		Color		m_key_scale;
		Color		m_noise_color;
		uint8_t		m_lod_frac;
		uint8_t		m_prim_lod_frac;

		Color		m_one_color;
		Color		m_zero_color;

		uint32_t		m_fill_color;

		uint16_t		m_cc_lut1[(1<<24)];
		uint8_t		m_cc_lut2[(1<<24)];
		uint32_t		m_rgb16_to_rgb32_lut[(1 << 16)];
		uint32_t		m_ia8_to_rgb32_lut[(1 << 16)];

		uint32_t		m_cmd_data[0x1000];
		int 		m_cmd_ptr;
		int 		m_cmd_cur;

		uint32_t		m_start;
		uint32_t		m_end;
		uint32_t		m_current;
		uint32_t		m_status;

		uint8_t*		m_tmem;

		Tile		m_tiles[8];

		running_machine* m_machine;

		// YUV factors
		int32_t m_k0;
		int32_t m_k1;
		int32_t m_k2;
		int32_t m_k3;
		int32_t m_k4;
		int32_t m_k5;

		// Render-related (move into eventual drawing-related classes?)
		Rectangle m_scissor;

		// Texture perspective division
		int32_t m_norm_point_rom[64];
		int32_t m_norm_slope_rom[64];

		uint8_t m_hidden_bits[0x800000];

		Span m_span[4096];

		static const uint8_t s_bayer_matrix[16];
		static const uint8_t s_magic_matrix[16];

		int32_t m_gamma_table[256];
		int32_t m_gamma_dither_table[0x4000];

		class ZDecompressEntry
		{
			public:
				uint32_t shift;
				uint32_t add;
		};

		void BuildCompressedZTable();
		uint16_t m_z_compress_table[0x40000];
		static const ZDecompressEntry m_z_decompress_table[8];

		// Internal screen-update functions
		void			VideoUpdate16(bitmap_t *bitmap);
		void			VideoUpdate32(bitmap_t *bitmap);

		typedef void (*Command)(uint32_t w1, uint32_t w2);

		static const Command m_commands[0x40];
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_N64_H_
