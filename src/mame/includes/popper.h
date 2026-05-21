/***************************************************************************

    Popper

***************************************************************************/

class popper_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, popper_state(machine)); }

	popper_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t * videoram;
	uint8_t * ol_videoram;
	uint8_t * attribram;
	uint8_t * ol_attribram;
	uint8_t * spriteram;
	size_t  spriteram_size;

	/* video-related */
	tilemap_t *p123_tilemap, *p0_tilemap;
	tilemap_t *ol_p123_tilemap, *ol_p0_tilemap;
	int32_t flipscreen, e002, gfx_bank;
	rectangle tilemap_clip;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/popper.c -----------*/

WRITE8_HANDLER( popper_videoram_w );
WRITE8_HANDLER( popper_attribram_w );
WRITE8_HANDLER( popper_ol_videoram_w );
WRITE8_HANDLER( popper_ol_attribram_w );
WRITE8_HANDLER( popper_flipscreen_w );
WRITE8_HANDLER( popper_e002_w );
WRITE8_HANDLER( popper_gfx_bank_w );

PALETTE_INIT( popper );
VIDEO_START( popper );
VIDEO_UPDATE( popper );

