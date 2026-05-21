/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"

class offtwall_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, offtwall_state(machine)); }

	offtwall_state(running_machine &machine) { }

	atarigen_state	atarigen;

	uint16_t *bankswitch_base;
	uint16_t *bankrom_base;
	uint32_t bank_offset;

	uint16_t *spritecache_count;
	uint16_t *unknown_verify_base;
};


/*----------- defined in video/offtwall.c -----------*/

VIDEO_START( offtwall );
VIDEO_UPDATE( offtwall );
