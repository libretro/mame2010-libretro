/*************************************************************************

    Labyrinth Runner

*************************************************************************/

class labyrunr_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, labyrunr_state(machine)); }

	labyrunr_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram1;
	uint8_t *    videoram2;
	uint8_t *    scrollram;
	uint8_t *    spriteram;
	uint8_t *    paletteram;

	/* video-related */
	tilemap_t    *layer0, *layer1;
	rectangle  clip0, clip1;

	/* devices */
	running_device *k007121;
};


/*----------- defined in video/labyrunr.c -----------*/


WRITE8_HANDLER( labyrunr_vram1_w );
WRITE8_HANDLER( labyrunr_vram2_w );

PALETTE_INIT( labyrunr );
VIDEO_START( labyrunr );
VIDEO_UPDATE( labyrunr );
