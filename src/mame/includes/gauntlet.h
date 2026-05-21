/*************************************************************************

    Atari Gauntlet hardware

*************************************************************************/

#include "machine/atarigen.h"

class gauntlet_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gauntlet_state(machine)); }

	gauntlet_state(running_machine &machine) { }

	atarigen_state	atarigen;
	uint16_t			sound_reset_val;
	uint8_t			vindctr2_screen_refresh;
	uint8_t			playfield_tile_bank;
	uint8_t			playfield_color_bank;
};


/*----------- defined in video/gauntlet.c -----------*/

WRITE16_HANDLER( gauntlet_xscroll_w );
WRITE16_HANDLER( gauntlet_yscroll_w );

VIDEO_START( gauntlet );
VIDEO_UPDATE( gauntlet );
