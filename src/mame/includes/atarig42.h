/*************************************************************************

    Atari G42 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarig42_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, atarig42_state(machine)); }

	atarig42_state(running_machine &machine) { }

	atarigen_state	atarigen;
	uint16_t			playfield_base;
	uint16_t			motion_object_base;
	uint16_t			motion_object_mask;

	uint16_t			current_control;
	uint8_t			playfield_tile_bank;
	uint8_t			playfield_color_bank;
	uint16_t			playfield_xscroll;
	uint16_t			playfield_yscroll;

	uint8_t			analog_data;
	uint16_t *		mo_command;

	int 			sloop_bank;
	int 			sloop_next_bank;
	int 			sloop_offset;
	int 			sloop_state;
	uint16_t *		sloop_base;
};


/*----------- defined in video/atarig42.c -----------*/

VIDEO_START( atarig42 );
VIDEO_UPDATE( atarig42 );

WRITE16_HANDLER( atarig42_mo_control_w );

void atarig42_scanline_update(screen_device &screen, int scanline);

