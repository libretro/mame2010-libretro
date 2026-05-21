/*************************************************************************

    Ambush

*************************************************************************/

class ambush_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ambush_state(machine)); }

	ambush_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    spriteram;
	uint8_t *    colorram;
	uint8_t *    scrollram;
	uint8_t *    colorbank;

	size_t     videoram_size;
	size_t     spriteram_size;
};


/*----------- defined in video/ambush.c -----------*/

PALETTE_INIT( ambush );
VIDEO_UPDATE( ambush );
