/*************************************************************************

    Mikie

*************************************************************************/

class mikie_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mikie_state(machine)); }

	mikie_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    colorram;
	uint8_t *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap;
	int        palettebank;

	/* misc */
	int        last_irq;

	/* devices */
	cpu_device *maincpu;
	cpu_device *audiocpu;
};


/*----------- defined in video/mikie.c -----------*/

WRITE8_HANDLER( mikie_videoram_w );
WRITE8_HANDLER( mikie_colorram_w );
WRITE8_HANDLER( mikie_palettebank_w );
WRITE8_HANDLER( mikie_flipscreen_w );

PALETTE_INIT( mikie );
VIDEO_START( mikie );
VIDEO_UPDATE( mikie );
