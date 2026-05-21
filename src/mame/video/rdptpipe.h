#ifndef _VIDEO_RDPTEXPIPE_H_
#define _VIDEO_RDPTEXPIPE_H_

#include "emu.h"
#include "video/rdpfetch.h"

namespace N64
{

namespace RDP
{

class OtherModes;
class MiscState;
class Processor;
class Color;

class TexturePipe
{
	public:
		TexturePipe()
		{
			m_maskbits_table[0] = 0x3ff;
			for(int i = 1; i < 16; i++)
			{
				m_maskbits_table[i] = ((uint16_t)(0xffff) >> (16 - i)) & 0x3ff;
			}
		}

		uint32_t				Fetch(int32_t SSS, int32_t SST, Tile* tile);
		void				CalculateClampDiffs(uint32_t prim_tile);

		void				SetMachine(running_machine* machine);

	private:
		void				Mask(int32_t* S, int32_t* T, Tile* tile);
		void				TexShift(int32_t* S, int32_t* T, bool* maxs, bool* maxt, Tile *tile);
		void				Clamp(int32_t* S, int32_t* T, int32_t* SFRAC, int32_t* TFRAC, bool maxs, bool maxt, Tile* tile);
		void				ClampLight(int32_t* S, int32_t* T, bool maxs, bool maxt, Tile* tile);

		running_machine*	m_machine;
		OtherModes*			m_other_modes;
		Processor*			m_rdp;
		TexFetch			m_tex_fetch;

		int32_t				m_maskbits_table[16];
		int32_t				m_clamp_t_diff[8];
		int32_t				m_clamp_s_diff[8];
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPTEXPIPE_H_
