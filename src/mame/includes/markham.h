/*************************************************************************

    Markham

*************************************************************************/

class markham_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, markham_state(machine)); }

	markham_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    spriteram;
	uint8_t *    xscroll;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap;
};


/*----------- defined in video/markham.c -----------*/

WRITE8_HANDLER( markham_videoram_w );
WRITE8_HANDLER( markham_flipscreen_w );

PALETTE_INIT( markham );
VIDEO_START( markham );
VIDEO_UPDATE( markham );
