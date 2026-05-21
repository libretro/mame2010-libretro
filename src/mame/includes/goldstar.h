class goldstar_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, goldstar_state(machine)); }

	goldstar_state(running_machine &machine) { }

	int dataoffset;

	uint8_t *nvram;
	size_t nvram_size;

	uint8_t *atrram;
	uint8_t *fg_atrram;
	uint8_t *fg_vidram;

	uint8_t *reel1_scroll;
	uint8_t *reel2_scroll;
	uint8_t *reel3_scroll;

	uint8_t *reel1_ram;
	uint8_t *reel2_ram;
	uint8_t *reel3_ram;

	/* reelx_attrram for unkch sets */
	uint8_t *reel1_attrram;
	uint8_t *reel2_attrram;
	uint8_t *reel3_attrram;
	uint8_t unkch_vidreg;

	tilemap_t *reel1_tilemap;
	tilemap_t *reel2_tilemap;
	tilemap_t *reel3_tilemap;

	int bgcolor;
	tilemap_t *fg_tilemap;
	uint8_t cmaster_girl_num;
	uint8_t cmaster_girl_pal;
	uint8_t cm_enable_reg;
	uint8_t cm_girl_scroll;
	uint8_t lucky8_nmi_enable;
	int tile_bank;

};


/*----------- defined in video/goldstar.c -----------*/

WRITE8_HANDLER( goldstar_reel1_ram_w );
WRITE8_HANDLER( goldstar_reel2_ram_w );
WRITE8_HANDLER( goldstar_reel3_ram_w );

WRITE8_HANDLER( unkch_reel1_attrram_w );
WRITE8_HANDLER( unkch_reel2_attrram_w );
WRITE8_HANDLER( unkch_reel3_attrram_w );

WRITE8_HANDLER( goldstar_fg_vidram_w );
WRITE8_HANDLER( goldstar_fg_atrram_w );
WRITE8_HANDLER( cm_girl_scroll_w );

WRITE8_HANDLER( goldstar_fa00_w );
WRITE8_HANDLER( cm_background_col_w );
WRITE8_HANDLER( cm_outport0_w );
VIDEO_START( goldstar );
VIDEO_START( cherrym );
VIDEO_START( unkch );
VIDEO_START( magical );
VIDEO_UPDATE( goldstar );
VIDEO_UPDATE( cmast91 );
VIDEO_UPDATE( amcoe1a );
VIDEO_UPDATE( unkch );
VIDEO_UPDATE( magical );
