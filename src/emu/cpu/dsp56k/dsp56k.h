/***************************************************************************

    dsp56k.h
    Interface file for the portable Motorola/Freescale DSP56k emulator.
    Written by Andrew Gardner

***************************************************************************/


#pragma once

#ifndef __DSP56K_H__
#define __DSP56K_H__


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/
enum
{
	// PCU
	DSP56K_PC=1,
	DSP56K_SR,
	DSP56K_LC,
	DSP56K_LA,
	DSP56K_SP,
	DSP56K_OMR,

	// ALU
	DSP56K_X, DSP56K_Y,
	DSP56K_A, DSP56K_B,

	// AGU
	DSP56K_R0,DSP56K_R1,DSP56K_R2,DSP56K_R3,
	DSP56K_N0,DSP56K_N1,DSP56K_N2,DSP56K_N3,
	DSP56K_M0,DSP56K_M1,DSP56K_M2,DSP56K_M3,
	DSP56K_TEMP,
	DSP56K_STATUS,

	// CPU STACK
	DSP56K_ST0,
	DSP56K_ST1,
	DSP56K_ST2,
	DSP56K_ST3,
	DSP56K_ST4,
	DSP56K_ST5,
	DSP56K_ST6,
	DSP56K_ST7,
	DSP56K_ST8,
	DSP56K_ST9,
	DSP56K_ST10,
	DSP56K_ST11,
	DSP56K_ST12,
	DSP56K_ST13,
	DSP56K_ST14,
	DSP56K_ST15
};

// IRQ Lines
// MODA and MODB are also known as IRQA and IRQB
#define DSP56K_IRQ_MODA  0
#define DSP56K_IRQ_MODB  1
#define DSP56K_IRQ_MODC  2
#define DSP56K_IRQ_RESET 3	/* Is this needed? */

// Needed for MAME
DECLARE_LEGACY_CPU_DEVICE(DSP56156, dsp56k);


/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/
// 5-4 Host Interface
typedef struct
{
	// **** Dsp56k side **** //
	// Host Control Register
	uint16_t* hcr;

	// Host Status Register
	uint16_t* hsr;

	// Host Transmit/Receive Data
	uint16_t* htrx;

	// **** Host CPU side **** //
	// Interrupt Control Register
	uint8_t icr;

	// Command Vector Register
	uint8_t cvr;

	// Interrupt Status Register
	uint8_t isr;

	// Interrupt Vector Register
	uint8_t ivr;

	// Transmit / Receive Registers
	uint8_t trxh;
	uint8_t trxl;

	// HACK - Host interface bootstrap write offset
	uint16_t bootstrap_offset;

} dsp56k_host_interface;

// 1-9 ALU
typedef struct
{
	// Four 16-bit input registers (can be accessed as 2 32-bit registers)
	PAIR x;
	PAIR y;

	// Two 32-bit accumulator registers + 8-bit accumulator extension registers
	PAIR64 a;
	PAIR64 b;

	// An accumulation shifter
	// One data bus shifter/limiter
	// A parallel, single cycle, non-pipelined Multiply-Accumulator (MAC) unit
	// Basics
} dsp56k_data_alu;

// 1-10 Address Generation Unit (AGU)
typedef struct
{
	// Four address registers
	uint16_t r0;
	uint16_t r1;
	uint16_t r2;
	uint16_t r3;

	// Four offset registers
	uint16_t n0;
	uint16_t n1;
	uint16_t n2;
	uint16_t n3;

	// Four modifier registers
	uint16_t m0;
	uint16_t m1;
	uint16_t m2;
	uint16_t m3;

	// Used in loop processing
	uint16_t temp;

	// FM.4-5 - hmmm?
	// uint8_t status;

	// Basics
} dsp56k_agu;

// 1-11 Program Control Unit (PCU)
typedef struct
{
	// Program Counter
	uint16_t pc;

	// Loop Address
	uint16_t la;

	// Loop Counter
	uint16_t lc;

	// Status Register
	uint16_t sr;

	// Operating Mode Register
	uint16_t omr;

	// Stack Pointer
	uint16_t sp;

	// Stack (TODO: 15-level?)
	PAIR ss[16];

	// Controls IRQ processing
	void (*service_interrupts)(void);

	// A list of pending interrupts (indices into dsp56k_interrupt_sources array)
	int8_t pending_interrupts[32];

	// Basics

	// Other PCU internals
	uint16_t reset_vector;

} dsp56k_pcu;

// 1-8 The dsp56156 CORE
typedef struct
{
	// PROGRAM CONTROLLER
	dsp56k_pcu PCU;

	// ADR ALU (AGU)
	dsp56k_agu AGU;

	// CLOCK GEN
	//static emu_timer *dsp56k_timer;   // 1-5, 1-8 - Clock gen

	// DATA ALU
	dsp56k_data_alu ALU;

	// OnCE

	// IBS and BITFIELD UNIT

	// Host Interface
	dsp56k_host_interface HI;

	// IRQ line states
	uint8_t modA_state;
	uint8_t modB_state;
	uint8_t modC_state;
	uint8_t reset_state;

	// HACK - Bootstrap mode state variable.
	uint8_t bootstrap_mode;

	uint8_t	repFlag;	// Knowing if we're in a 'repeat' state (dunno how the processor does this)
	uint32_t	repAddr;	// The address of the instruction to repeat...


	/* MAME internal stuff */
	int icount;

	uint32_t			ppc;
	uint32_t			op;
	int				interrupt_cycles;
	void			(*output_pins_changed)(uint32_t pins);
	legacy_cpu_device *device;
	const address_space *program;
	const address_space *data;
} dsp56k_core;


/***************************************************************************
    PUBLIC FUNCTIONS - ACCESSIBLE TO DRIVERS
***************************************************************************/
void  dsp56k_host_interface_write(running_device* device, uint8_t offset, uint8_t data);
uint8_t dsp56k_host_interface_read(running_device* device, uint8_t offset);

uint16_t dsp56k_get_peripheral_memory(running_device* device, uint16_t addr);

#endif /* __DSP56K_H__ */
