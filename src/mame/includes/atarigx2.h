/*************************************************************************

    Atari GX2 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarigx2_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, atarigx2_state(machine)); }

	atarigx2_state(running_machine &machine) { }

	atarigen_state	atarigen;
	uint16_t			playfield_base;
	uint16_t			motion_object_base;
	uint16_t			motion_object_mask;

	uint32_t *		mo_command;
	uint32_t *		protection_base;

	uint16_t			current_control;
	uint8_t			playfield_tile_bank;
	uint8_t			playfield_color_bank;
	uint16_t			playfield_xscroll;
	uint16_t			playfield_yscroll;

	uint16_t			last_write;
	uint16_t			last_write_offset;
};


/*----------- defined in video/atarigx2.c -----------*/

VIDEO_START( atarigx2 );
VIDEO_UPDATE( atarigx2 );

WRITE16_HANDLER( atarigx2_mo_control_w );

void atarigx2_scanline_update(screen_device &screen, int scanline);
