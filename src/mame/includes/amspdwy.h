/*************************************************************************

    American Speedway

*************************************************************************/

class amspdwy_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, amspdwy_state(machine)); }

	amspdwy_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    spriteram;
	uint8_t *    colorram;
//  uint8_t *    paletteram;  // currently this uses generic palette handling
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;
	int        flipscreen;

	/* misc */
	uint8_t      wheel_old[2];
	uint8_t      wheel_return[2];

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/amspdwy.c -----------*/

WRITE8_HANDLER( amspdwy_videoram_w );
WRITE8_HANDLER( amspdwy_colorram_w );
WRITE8_HANDLER( amspdwy_paletteram_w );
WRITE8_HANDLER( amspdwy_flipscreen_w );

VIDEO_START( amspdwy );
VIDEO_UPDATE( amspdwy );
