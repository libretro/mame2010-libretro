/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"

class relief_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, relief_state(machine)); }

	relief_state(running_machine &machine) { }

	atarigen_state	atarigen;

	uint8_t			ym2413_volume;
	uint8_t			overall_volume;
	uint32_t			adpcm_bank_base;
};


/*----------- defined in video/relief.c -----------*/

VIDEO_START( relief );
VIDEO_UPDATE( relief );
