/*************************************************************************

    IronHorse

*************************************************************************/

class ironhors_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ironhors_state(machine)); }

	ironhors_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    colorram;
	uint8_t *    spriteram;
	uint8_t *    spriteram2;
	uint8_t *    scroll;
	uint8_t *    interrupt_enable;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;
	int        palettebank, charbank, spriterambank;

	/* devices */
	running_device *soundcpu;
};


/*----------- defined in video/ironhors.c -----------*/

WRITE8_HANDLER( ironhors_videoram_w );
WRITE8_HANDLER( ironhors_colorram_w );
WRITE8_HANDLER( ironhors_palettebank_w );
WRITE8_HANDLER( ironhors_charbank_w );
WRITE8_HANDLER( ironhors_flipscreen_w );

PALETTE_INIT( ironhors );
VIDEO_START( ironhors );
VIDEO_UPDATE( ironhors );
VIDEO_START( farwest );
VIDEO_UPDATE( farwest );
