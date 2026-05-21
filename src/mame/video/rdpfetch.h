#ifndef _VIDEO_RDPFETCH_H_
#define _VIDEO_RDPFETCH_H_

#include "emu.h"

namespace N64
{

namespace RDP
{

class Processor;
class OtherModes;
class MiscState;
class Tile;

class TexFetch
{
	public:
		TexFetch() { }

		uint32_t				Fetch(uint32_t s, uint32_t t, Tile* tile);

		void				SetMachine(running_machine* machine);

	private:
		running_machine*	m_machine;
		Processor*			m_rdp;
		OtherModes*			m_other_modes;
		MiscState*			m_misc_state;
		Tile*				m_tiles;

		uint32_t				FetchRGBA(uint32_t s, uint32_t t, Tile* tile);
		uint32_t				FetchYUV(uint32_t s, uint32_t t, Tile* tile);
		uint32_t				FetchCI(uint32_t s, uint32_t t, Tile* tile);
		uint32_t				FetchIA(uint32_t s, uint32_t t, Tile* tile);
		uint32_t				FetchI(uint32_t s, uint32_t t, Tile* tile);
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPFETCH_H_
