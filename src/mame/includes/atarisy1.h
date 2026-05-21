/*************************************************************************

    Atari System 1 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarisy1_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, atarisy1_state(machine)); }

	atarisy1_state(running_machine &machine)
		: joystick_timer(machine.device<timer_device>("joystick_timer")),
		  yscroll_reset_timer(machine.device<timer_device>("yreset_timer")),
		  scanline_timer(machine.device<timer_device>("scan_timer")),
		  int3off_timer(machine.device<timer_device>("int3off_timer")) { }

	atarigen_state	atarigen;

	uint16_t *		bankselect;

	uint8_t			joystick_type;
	uint8_t			trackball_type;

	timer_device *	joystick_timer;
	uint8_t			joystick_int;
	uint8_t			joystick_int_enable;
	uint8_t			joystick_value;

	/* playfield parameters */
	uint16_t			playfield_lookup[256];
	uint8_t			playfield_tile_bank;
	uint16_t			playfield_priority_pens;
	timer_device *	yscroll_reset_timer;

	/* INT3 tracking */
	int 			next_timer_scanline;
	timer_device *	scanline_timer;
	timer_device *	int3off_timer;

	/* graphics bank tracking */
	uint8_t			bank_gfx[3][8];
	uint8_t			bank_color_shift[MAX_GFX_ELEMENTS];
};


/*----------- defined in video/atarisy1.c -----------*/

TIMER_DEVICE_CALLBACK( atarisy1_int3_callback );
TIMER_DEVICE_CALLBACK( atarisy1_int3off_callback );
TIMER_DEVICE_CALLBACK( atarisy1_reset_yscroll_callback );

READ16_HANDLER( atarisy1_int3state_r );

WRITE16_HANDLER( atarisy1_spriteram_w );
WRITE16_HANDLER( atarisy1_bankselect_w );
WRITE16_HANDLER( atarisy1_xscroll_w );
WRITE16_HANDLER( atarisy1_yscroll_w );
WRITE16_HANDLER( atarisy1_priority_w );

VIDEO_START( atarisy1 );
VIDEO_UPDATE( atarisy1 );
