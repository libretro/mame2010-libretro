class welltris_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, welltris_state(machine)); }

	welltris_state(running_machine &machine) { }

	int pending_command;

	uint16_t *spriteram;
	uint16_t *pixelram;
	uint16_t *charvideoram;

	tilemap_t *char_tilemap;
	uint8_t gfxbank[8];
	uint16_t charpalettebank;
	uint16_t spritepalettebank;
	uint16_t pixelpalettebank;
	int scrollx;
	int scrolly;
};


/*----------- defined in video/welltris.c -----------*/

//READ16_HANDLER( welltris_spriteram_r );
WRITE16_HANDLER( welltris_spriteram_w );
WRITE16_HANDLER( welltris_palette_bank_w );
WRITE16_HANDLER( welltris_gfxbank_w );
WRITE16_HANDLER( welltris_charvideoram_w );
WRITE16_HANDLER( welltris_scrollreg_w );

VIDEO_START( welltris );
VIDEO_UPDATE( welltris );
