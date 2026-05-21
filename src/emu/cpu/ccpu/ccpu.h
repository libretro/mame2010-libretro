/***************************************************************************

    ccpu.h
    Core implementation for the portable Cinematronics CPU emulator.

    Written by Aaron Giles
    Special thanks to Zonn Moore for his detailed documentation.

***************************************************************************/

#pragma once

#ifndef __CCPU_H__
#define	__CCPU_H__


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	CCPU_PC=1,
	CCPU_FLAGS,
	CCPU_A,
	CCPU_B,
	CCPU_I,
	CCPU_J,
	CCPU_P,
	CCPU_X,
	CCPU_Y,
	CCPU_T
};



/***************************************************************************
    CONFIG STRUCTURE
***************************************************************************/

typedef uint8_t (*ccpu_input_func)(running_device *device);
typedef void (*ccpu_vector_func)(running_device *device, int16_t sx, int16_t sy, int16_t ex, int16_t ey, uint8_t shift);

typedef struct _ccpu_config ccpu_config;
struct _ccpu_config
{
	ccpu_input_func		external_input;		/* if NULL, assume JMI jumper is present */
	ccpu_vector_func	vector_callback;
};



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

DECLARE_LEGACY_CPU_DEVICE(CCPU, ccpu);

void ccpu_wdt_timer_trigger(running_device *device);

CPU_DISASSEMBLE( ccpu );

#endif /* __CCPU_H__ */
