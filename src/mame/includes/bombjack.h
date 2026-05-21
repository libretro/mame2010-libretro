/*************************************************************************

    Bomb Jack

*************************************************************************/

class bombjack_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, bombjack_state(machine)); }

	bombjack_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    colorram;
	uint8_t *    spriteram;
//  uint8_t *    paletteram;  // currently this uses generic palette handling
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *fg_tilemap, *bg_tilemap;
	uint8_t      background_image;

	/* sound-related */
	uint8_t      latch;
};


/*----------- defined in video/bombjack.c -----------*/

WRITE8_HANDLER( bombjack_videoram_w );
WRITE8_HANDLER( bombjack_colorram_w );
WRITE8_HANDLER( bombjack_background_w );
WRITE8_HANDLER( bombjack_flipscreen_w );

VIDEO_START( bombjack );
VIDEO_UPDATE( bombjack );
