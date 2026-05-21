/*************************************************************************

    Atari G1 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarig1_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, atarig1_state(machine)); }

	atarig1_state(running_machine &machine) { }

	atarigen_state	atarigen;
	uint8_t			is_pitfight;

	uint8_t			which_input;
	uint16_t *		mo_command;

	uint16_t *		bslapstic_base;
	void *			bslapstic_bank0;
	uint8_t			bslapstic_bank;
	uint8_t			bslapstic_primed;

	int 			pfscroll_xoffset;
	uint16_t			current_control;
	uint8_t			playfield_tile_bank;
	uint16_t			playfield_xscroll;
	uint16_t			playfield_yscroll;
};


/*----------- defined in video/atarig1.c -----------*/

WRITE16_HANDLER( atarig1_mo_control_w );

VIDEO_START( atarig1 );
VIDEO_UPDATE( atarig1 );

void atarig1_scanline_update(screen_device &screen, int scanline);
