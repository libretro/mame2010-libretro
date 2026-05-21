/*****************************************************************************

    h6280.h Portable Hu6280 emulator interface

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.

******************************************************************************/

#pragma once

#ifndef __H6280_H__
#define __H6280_H__


enum
{
	H6280_PC=1, H6280_S, H6280_P, H6280_A, H6280_X, H6280_Y,
	H6280_IRQ_MASK, H6280_TIMER_STATE,
	H6280_NMI_STATE, H6280_IRQ1_STATE, H6280_IRQ2_STATE, H6280_IRQT_STATE,
	H6280_M1, H6280_M2, H6280_M3, H6280_M4,
	H6280_M5, H6280_M6, H6280_M7, H6280_M8
};

#define LAZY_FLAGS  0

#define H6280_RESET_VEC	0xfffe
#define H6280_NMI_VEC	0xfffc
#define H6280_TIMER_VEC	0xfffa
#define H6280_IRQ1_VEC	0xfff8
#define H6280_IRQ2_VEC	0xfff6			/* Aka BRK vector */


/****************************************************************************
 * The 6280 registers.
 ****************************************************************************/
typedef struct
{
	int ICount;

	PAIR  ppc;			/* previous program counter */
    PAIR  pc;           /* program counter */
    PAIR  sp;           /* stack pointer (always 100 - 1FF) */
    PAIR  zp;           /* zero page address */
    PAIR  ea;           /* effective address */
    uint8_t a;            /* Accumulator */
    uint8_t x;            /* X index register */
    uint8_t y;            /* Y index register */
    uint8_t p;            /* Processor status */
    uint8_t mmr[8];       /* Hu6280 memory mapper registers */
    uint8_t irq_mask;     /* interrupt enable/disable */
    uint8_t timer_status; /* timer status */
	uint8_t timer_ack;	/* timer acknowledge */
    uint8_t clocks_per_cycle; /* 4 = low speed mode, 1 = high speed mode */
    int32_t timer_value;    /* timer interrupt */
    int32_t timer_load;		/* reload value */
    uint8_t nmi_state;
    uint8_t irq_state[3];
	uint8_t irq_pending;
	device_irq_callback irq_callback;
	legacy_cpu_device *device;
	const address_space *program;
	const address_space *io;

#if LAZY_FLAGS
    int32_t NZ;			/* last value (lazy N and Z flag) */
#endif
	uint8_t io_buffer;	/* last value written to the PSG, timer, and interrupt pages */
} h6280_Regs;


DECLARE_LEGACY_CPU_DEVICE(H6280, h6280);

READ8_HANDLER( h6280_irq_status_r );
WRITE8_HANDLER( h6280_irq_status_w );

READ8_HANDLER( h6280_timer_r );
WRITE8_HANDLER( h6280_timer_w );

/* functions for use by the PSG and joypad port only! */
uint8_t h6280io_get_buffer(running_device*);
void h6280io_set_buffer(running_device*, uint8_t);

CPU_DISASSEMBLE( h6280 );

#endif /* __H6280_H__ */
