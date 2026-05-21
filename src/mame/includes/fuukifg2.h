

class fuuki16_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, fuuki16_state(machine)); }

	fuuki16_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *    vram_0;
	uint16_t *    vram_1;
	uint16_t *    vram_2;
	uint16_t *    vram_3;
	uint16_t *    vregs;
	uint16_t *    priority;
	uint16_t *    unknown;
	uint16_t *    spriteram;
//  uint16_t *    paletteram; // currently this uses generic palette handling
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *tilemap_0, *tilemap_1, *tilemap_2, *tilemap_3;

	/* misc */
	emu_timer   *raster_interrupt_timer;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/fuukifg2.c -----------*/

WRITE16_HANDLER( fuuki16_vram_0_w );
WRITE16_HANDLER( fuuki16_vram_1_w );
WRITE16_HANDLER( fuuki16_vram_2_w );
WRITE16_HANDLER( fuuki16_vram_3_w );

VIDEO_START( fuuki16 );
VIDEO_UPDATE( fuuki16 );
