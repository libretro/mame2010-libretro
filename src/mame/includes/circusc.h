/*************************************************************************

    Circus Charlie

*************************************************************************/

class circusc_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, circusc_state(machine)); }

	circusc_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *        videoram;
	uint8_t *        colorram;
	uint8_t *        spriteram;
	uint8_t *        spriteram_2;
	uint8_t *        spritebank;
	uint8_t *        scroll;
	size_t         spriteram_size;

	/* video-related */
	tilemap_t        *bg_tilemap;

	/* sound-related */
	uint8_t          sn_latch;

	/* devices */
	cpu_device *audiocpu;
	running_device *sn1;
	running_device *sn2;
	running_device *dac;
	running_device *discrete;
};


/*----------- defined in video/circusc.c -----------*/

WRITE8_HANDLER( circusc_videoram_w );
WRITE8_HANDLER( circusc_colorram_w );

VIDEO_START( circusc );
WRITE8_HANDLER( circusc_flipscreen_w );
PALETTE_INIT( circusc );
VIDEO_UPDATE( circusc );
