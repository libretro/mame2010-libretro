/*************************************************************************

    Atari System 2 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarisy2_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, atarisy2_state(machine)); }

	atarisy2_state(running_machine &machine) { }

	atarigen_state	atarigen;

	uint16_t *		slapstic_base;

	uint8_t			interrupt_enable;
	uint16_t *		bankselect;

	int8_t			pedal_count;

	uint8_t			has_tms5220;

	uint8_t			which_adc;

	uint8_t			p2portwr_state;
	uint8_t			p2portrd_state;

	uint16_t *		rombank1;
	uint16_t *		rombank2;

	uint8_t			sound_reset_state;

	emu_timer *		yscroll_reset_timer;
	uint32_t			playfield_tile_bank[2];
	uint32_t			videobank;
	uint16_t			vram[0x8000/2];
};


/*----------- defined in video/atarisy2.c -----------*/

READ16_HANDLER( atarisy2_slapstic_r );
READ16_HANDLER( atarisy2_videoram_r );

WRITE16_HANDLER( atarisy2_slapstic_w );
WRITE16_HANDLER( atarisy2_yscroll_w );
WRITE16_HANDLER( atarisy2_xscroll_w );
WRITE16_HANDLER( atarisy2_videoram_w );
WRITE16_HANDLER( atarisy2_paletteram_w );

VIDEO_START( atarisy2 );
VIDEO_UPDATE( atarisy2 );
