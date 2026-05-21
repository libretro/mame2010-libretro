/*************************************************************************

    Atari Bad Lands hardware

*************************************************************************/

#include "machine/atarigen.h"

class badlands_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, badlands_state(machine)); }

	badlands_state(running_machine &machine) { }

	atarigen_state	atarigen;

	uint8_t			pedal_value[2];

	uint8_t *			bank_base;
	uint8_t *			bank_source_data;

	uint8_t			playfield_tile_bank;
};


/*----------- defined in video/badlands.c -----------*/

WRITE16_HANDLER( badlands_pf_bank_w );

VIDEO_START( badlands );
VIDEO_UPDATE( badlands );
