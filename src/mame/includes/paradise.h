
class paradise_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, paradise_state(machine)); }

	paradise_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  vram_0;
	uint8_t *  vram_1;
	uint8_t *  vram_2;
	uint8_t *  videoram;
	uint8_t *  paletteram;
	uint8_t *  spriteram;
	size_t   spriteram_size;

	/* video-related */
	tilemap_t *tilemap_0, *tilemap_1, *tilemap_2;
	bitmap_t *tmpbitmap;
	uint8_t palbank, priority;
	int sprite_inc;
};

/*----------- defined in video/paradise.c -----------*/

WRITE8_HANDLER( paradise_vram_0_w );
WRITE8_HANDLER( paradise_vram_1_w );
WRITE8_HANDLER( paradise_vram_2_w );

WRITE8_HANDLER( paradise_flipscreen_w );
WRITE8_HANDLER( tgtball_flipscreen_w );
WRITE8_HANDLER( paradise_palette_w );
WRITE8_HANDLER( paradise_pixmap_w );

WRITE8_HANDLER( paradise_priority_w );
WRITE8_HANDLER( paradise_palbank_w );

VIDEO_START( paradise );

VIDEO_UPDATE( paradise );
VIDEO_UPDATE( torus );
VIDEO_UPDATE( madball );
