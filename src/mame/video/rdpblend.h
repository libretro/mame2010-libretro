#ifndef _VIDEO_RDPBLEND_H_
#define _VIDEO_RDPBLEND_H_

#include "emu.h"

namespace N64
{

namespace RDP
{

class OtherModes;
class MiscState;
class Processor;
class Color;

class Blender
{
	public:
		Blender() { }

		bool				Blend(void* in_fb, uint8_t* hb, Color c1, Color c2, int dith);

		void				SetOtherModes(OtherModes* other_modes) { m_other_modes = other_modes; }
		void				SetMiscState(MiscState* misc_state) { m_misc_state = misc_state; }
		void				SetMachine(running_machine* machine) { m_machine = machine; }
		void				SetProcessor(Processor* rdp) { m_rdp = rdp; }

	private:
		running_machine*	m_machine;
		OtherModes*			m_other_modes;
		MiscState*			m_misc_state;
		Processor*			m_rdp;

		bool				Blend16Bit(uint16_t* fb, uint8_t* hb, RDP::Color c1, RDP::Color c2, int dith);
		bool				Blend16Bit1Cycle(uint16_t* fb, uint8_t* hb, RDP::Color c, int dith);
		bool				Blend16Bit2Cycle(uint16_t* fb, uint8_t* hb, RDP::Color c1, RDP::Color c2, int dith);
		bool				Blend32Bit(uint32_t* fb, RDP::Color c1, RDP::Color c2);
		bool				Blend32Bit1Cycle(uint32_t* fb, RDP::Color c);
		bool				Blend32Bit2Cycle(uint32_t* fb, RDP::Color c1, RDP::Color c2);
		bool				AlphaCompare(uint8_t alpha);

		void				BlendEquation0Force(int32_t* r, int32_t* g, int32_t* b, int bsel_special);
		void				BlendEquation0NoForce(int32_t* r, int32_t* g, int32_t* b, int bsel_special);
		void				BlendEquation1Force(int32_t* r, int32_t* g, int32_t* b, int bsel_special);
		void				BlendEquation1NoForce(int32_t* r, int32_t* g, int32_t* b, int bsel_special);

		void				DitherRGB(int32_t* r, int32_t* g, int32_t* b, int dith);
};

} // namespace RDP

} // namespace N64

#endif // _VIDEO_RDPBLEND_H_
