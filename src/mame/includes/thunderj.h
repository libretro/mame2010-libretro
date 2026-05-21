/*************************************************************************

    Atari ThunderJaws hardware

*************************************************************************/

#include "machine/atarigen.h"

class thunderj_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, thunderj_state(machine)); }

	thunderj_state(running_machine &machine) { }

	atarigen_state	atarigen;
	uint8_t			alpha_tile_bank;
};


/*----------- defined in video/thunderj.c -----------*/

VIDEO_START( thunderj );
VIDEO_UPDATE( thunderj );

void thunderj_mark_high_palette(bitmap_t *bitmap, uint16_t *pf, uint16_t *mo, int x, int y);
