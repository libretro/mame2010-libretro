class sidearms_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, sidearms_state(machine)); }

	sidearms_state(running_machine &machine) { }

	int gameid;

	uint8_t *videoram;
	uint8_t *colorram;
	uint8_t *bg_scrollx;
	uint8_t *bg_scrolly;
	uint8_t *tilerom;
	tilemap_t *bg_tilemap;
	tilemap_t *fg_tilemap;

	int bgon;
	int objon;
	int staron;
	int charon;
	int flipon;

	uint32_t hflop_74a_n;
	uint32_t hcount_191;
	uint32_t vcount_191;
	uint32_t latch_374;
};

/*----------- defined in video/sidearms.c -----------*/

WRITE8_HANDLER( sidearms_videoram_w );
WRITE8_HANDLER( sidearms_colorram_w );
WRITE8_HANDLER( sidearms_star_scrollx_w );
WRITE8_HANDLER( sidearms_star_scrolly_w );
WRITE8_HANDLER( sidearms_c804_w );
WRITE8_HANDLER( sidearms_gfxctrl_w );

VIDEO_START( sidearms );
VIDEO_UPDATE( sidearms );
VIDEO_EOF( sidearms );
