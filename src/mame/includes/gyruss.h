/*************************************************************************

    Gyruss

*************************************************************************/

class gyruss_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gyruss_state(machine)); }

	gyruss_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    colorram;
	uint8_t *    spriteram;
	uint8_t *    flipscreen;

	/* video-related */
	tilemap_t    *tilemap;

	/* devices */
	cpu_device *audiocpu;
	cpu_device *audiocpu_2;
};


/*----------- defined in video/gyruss.c -----------*/

WRITE8_HANDLER( gyruss_spriteram_w );
READ8_HANDLER( gyruss_scanline_r );

PALETTE_INIT( gyruss );
VIDEO_START( gyruss );
VIDEO_UPDATE( gyruss );
