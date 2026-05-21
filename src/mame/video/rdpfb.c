#include "emu.h"
#include "includes/n64.h"
#include "video/n64.h"

namespace N64
{

bool RDP::Framebuffer::Write(void *fb, uint8_t* hb, uint32_t r, uint32_t g, uint32_t b)
{
	switch(m_misc_state->m_fb_size)
	{
		case PIXEL_SIZE_16BIT:
			return Write16Bit((uint16_t*)fb, hb, r, g, b);

		case PIXEL_SIZE_32BIT:
			return Write32Bit((uint32_t*)fb, r, g, b);

		default:
			fatalerror("Unsupported bit depth: %d\n", m_misc_state->m_fb_size);
			break;
	}

	return false;
}

bool RDP::Framebuffer::Write16Bit(uint16_t *fb, uint8_t* hb, uint32_t r, uint32_t g, uint32_t b)
{
#undef CVG_DRAW
//#define CVG_DRAW
#ifdef CVG_DRAW
	int covdraw;
	if (m_misc_state->m_curpixel_cvg == 8)
	{
		covdraw = 255;
	}
	else
	{
		covdraw = m_misc_state->m_curpixel_cvg << 5;
	}
	r = covdraw;
	g = covdraw;
	b = covdraw;
#endif

	if (!m_other_modes->z_compare_en)
	{
		m_misc_state->m_curpixel_overlap = 0;
	}

	uint32_t memory_cvg = 8;
	if (m_other_modes->image_read_en)
	{
		memory_cvg = ((*fb & 1) << 2) + (*hb & 3) + 1;
	}

	uint32_t newcvg = m_misc_state->m_curpixel_cvg + memory_cvg;
	bool wrapped = newcvg > 8;

	uint16_t finalcolor = ((r >> 3) << 11) | ((g >> 3) << 6) | ((b >> 3) << 1);

	uint32_t clamped_cvg = wrapped ? 8 : newcvg;
	newcvg = wrapped ? (newcvg - 8) : newcvg;

	m_misc_state->m_curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clamped_cvg--;

	if (m_other_modes->color_on_cvg && !wrapped)
	{
		*fb &= 0xfffe;
		*fb |= ((newcvg >> 2) & 1);
		*hb = (newcvg & 3);
		return false;
	}

	switch(m_other_modes->cvg_dest)
	{
		case 0:
			if (!m_other_modes->force_blend && !m_misc_state->m_curpixel_overlap)
			{
				*fb = finalcolor | ((m_misc_state->m_curpixel_cvg >> 2) & 1);
				*hb = (m_misc_state->m_curpixel_cvg & 3);
			}
			else
			{
				*fb = finalcolor | ((clamped_cvg >> 2) & 1);
				*hb = (clamped_cvg & 3);
			}
			break;

		case 1:
			*fb = finalcolor | ((newcvg >> 2) & 1);
			*hb = (newcvg & 3);
			break;

		case 2:
			*fb = finalcolor | 1;
			*hb = 3;
			break;

		case 3:
			*fb = finalcolor | ((memory_cvg >> 2) & 1);
			*hb = (memory_cvg & 3);
			break;
	}

	return true;
}

bool RDP::Framebuffer::Write32Bit(uint32_t *fb, uint32_t r, uint32_t g, uint32_t b)
{
	uint32_t finalcolor = (r << 24) | (g << 16) | (b << 8);
	uint32_t memory_alphachannel = *fb & 0xff;

	uint32_t memory_cvg = 8;
	if (m_other_modes->image_read_en)
	{
		memory_cvg = ((*fb >>5) & 7) + 1;
	}

	uint32_t newcvg = m_misc_state->m_curpixel_cvg + memory_cvg;
	bool wrapped = (newcvg > 8);
	uint32_t clamped_cvg = wrapped ? 8 : newcvg;
	newcvg = (wrapped) ? (newcvg - 8) : newcvg;

	m_misc_state->m_curpixel_cvg--;
	newcvg--;
	memory_cvg--;
	clamped_cvg--;

	if (m_other_modes->color_on_cvg && !wrapped)
	{
		*fb &= 0xffffff00;
		*fb |= ((newcvg << 5) & 0xff);
		return 0;
	}

	switch(m_other_modes->cvg_dest)
	{
		case 0:
			if (!m_other_modes->force_blend && !m_misc_state->m_curpixel_overlap)
			{
				*fb = finalcolor|(m_misc_state->m_curpixel_cvg << 5);
			}
			else
			{
				*fb = finalcolor|(clamped_cvg << 5);
			}
			break;
		case 1:
			*fb = finalcolor | (newcvg << 5);
			break;
		case 2:
			*fb = finalcolor | 0xE0;
			break;
		case 3:
			*fb = finalcolor | memory_alphachannel;
			break;
	}

	return true;
}

} // namespace N64
