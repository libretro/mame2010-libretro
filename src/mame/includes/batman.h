/*************************************************************************

    Atari Batman hardware

*************************************************************************/

#include "machine/atarigen.h"

class batman_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, batman_state(machine)); }

	batman_state(running_machine &machine) { }

	atarigen_state	atarigen;

	uint16_t			latch_data;

	uint8_t			alpha_tile_bank;
};


/*----------- defined in video/batman.c -----------*/

VIDEO_START( batman );
VIDEO_UPDATE( batman );

void batman_scanline_update(screen_device &screen, int scanline);
