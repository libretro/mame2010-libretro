/***************************************************************************

    didisasm.h

    Device disassembly interfaces.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DIDISASM_H__
#define __DIDISASM_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// Disassembler constants
const uint32_t DASMFLAG_SUPPORTED		= 0x80000000;	// are disassembly flags supported?
const uint32_t DASMFLAG_STEP_OUT		= 0x40000000;	// this instruction should be the end of a step out sequence
const uint32_t DASMFLAG_STEP_OVER		= 0x20000000;	// this instruction should be stepped over by setting a breakpoint afterwards
const uint32_t DASMFLAG_OVERINSTMASK	= 0x18000000;	// number of extra instructions to skip when stepping over
const uint32_t DASMFLAG_OVERINSTSHIFT	= 27;			// bits to shift after masking to get the value
const uint32_t DASMFLAG_LENGTHMASK	= 0x0000ffff;	// the low 16-bits contain the actual length



//**************************************************************************
//  MACROS
//**************************************************************************

#define DASMFLAG_STEP_OVER_EXTRA(x)			((x) << DASMFLAG_OVERINSTSHIFT)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> device_config_disasm_interface

// class representing interface-specific configuration disasm
class device_config_disasm_interface : public device_config_interface
{
public:
	// construction/destruction
	device_config_disasm_interface(const machine_config &mconfig, device_config &device);
	virtual ~device_config_disasm_interface();

	// required configuration overrides
	uint32_t min_opcode_bytes() const { return disasm_min_opcode_bytes(); }
	uint32_t max_opcode_bytes() const { return disasm_max_opcode_bytes(); }

protected:
	// required configuration overrides
	virtual uint32_t disasm_min_opcode_bytes() const = 0;
	virtual uint32_t disasm_max_opcode_bytes() const = 0;
};



// ======================> device_disasm_interface

// class representing interface-specific live disasm
class device_disasm_interface : public device_interface
{
public:
	// construction/destruction
	device_disasm_interface(running_machine &machine, const device_config &config, device_t &device);
	virtual ~device_disasm_interface();

	// configuration access
	const device_config_disasm_interface &disasm_config() const { return m_disasm_config; }
	uint32_t min_opcode_bytes() const { return m_disasm_config.min_opcode_bytes(); }
	uint32_t max_opcode_bytes() const { return m_disasm_config.max_opcode_bytes(); }

	// interface for disassembly
	offs_t disassemble(char *buffer, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options = 0) { return disasm_disassemble(buffer, pc, oprom, opram, options); }

protected:
	// required operation overrides
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) = 0;

	const device_config_disasm_interface &	m_disasm_config;		// reference to configuration data
};


#endif	/* __DIDISASM_H__ */
