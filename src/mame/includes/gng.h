/*************************************************************************

    Ghosts'n Goblins

*************************************************************************/

class gng_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gng_state(machine)); }

	gng_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    bgvideoram;
	uint8_t *    fgvideoram;
//  uint8_t *    paletteram;  // currently this uses generic palette handling
//  uint8_t *    paletteram2; // currently this uses generic palette handling
//  uint8_t *    spriteram;   // currently this uses generic buffered spriteram

	/* video-related */
	tilemap_t    *bg_tilemap, *fg_tilemap;
	uint8_t      scrollx[2];
	uint8_t      scrolly[2];
};


/*----------- defined in video/gng.c -----------*/

WRITE8_HANDLER( gng_fgvideoram_w );
WRITE8_HANDLER( gng_bgvideoram_w );
WRITE8_HANDLER( gng_bgscrollx_w );
WRITE8_HANDLER( gng_bgscrolly_w );
WRITE8_HANDLER( gng_flipscreen_w );

VIDEO_START( gng );
VIDEO_UPDATE( gng );
VIDEO_EOF( gng );
