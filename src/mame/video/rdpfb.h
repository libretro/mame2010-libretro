#ifndef _VIDEO_RDPFB_H_
#define _VIDEO_RDPFB_H_

#include "emu.h"

namespace N64
{

namespace RDP
{

class OtherModes;
class MiscState;

class Framebuffer
{
	public:
		Framebuffer() { }

		bool				Write(void* fb, uint8_t* hb, uint32_t r, uint32_t g, uint32_t b);

		void				SetOtherModes(OtherModes* other_modes) { m_other_modes = other_modes; }
		void				SetMiscState(MiscState* misc_state) { m_misc_state = misc_state; }

	private:
		OtherModes*			m_other_modes;
		MiscState*			m_misc_state;

		bool				Write16Bit(uint16_t* fb, uint8_t* hb, uint32_t r, uint32_t g, uint32_t b);
		bool				Write32Bit(uint32_t* fb, uint32_t r, uint32_t g, uint32_t b);
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPFB_H_
