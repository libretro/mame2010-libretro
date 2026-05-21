class toypop_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, toypop_state(machine)); }

	toypop_state(running_machine &machine) { }

	uint8_t *videoram;
	uint8_t *spriteram;
	uint16_t *bg_image;
	uint8_t *m68000_sharedram;
	tilemap_t *bg_tilemap;

	int bitmapflip;
	int palettebank;
	int interrupt_enable_68k;
};


/*----------- defined in video/toypop.c -----------*/

WRITE8_HANDLER( toypop_videoram_w );
READ16_HANDLER( toypop_merged_background_r );
WRITE16_HANDLER( toypop_merged_background_w );
WRITE8_HANDLER( toypop_palettebank_w );
WRITE16_HANDLER( toypop_flipscreen_w );
VIDEO_START( toypop );
VIDEO_UPDATE( toypop );
PALETTE_INIT( toypop );
