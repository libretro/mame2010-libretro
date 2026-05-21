class stlforce_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, stlforce_state(machine)); }

	stlforce_state(running_machine &machine) { }

	tilemap_t *bg_tilemap;
	tilemap_t *mlow_tilemap;
	tilemap_t *mhigh_tilemap;
	tilemap_t *tx_tilemap;

	uint16_t *bg_videoram;
	uint16_t *mlow_videoram;
	uint16_t *mhigh_videoram;
	uint16_t *tx_videoram;
	uint16_t *bg_scrollram;
	uint16_t *mlow_scrollram;
	uint16_t *mhigh_scrollram;
	uint16_t *vidattrram;

	uint16_t *spriteram;

	int sprxoffs;
};


/*----------- defined in video/stlforce.c -----------*/

VIDEO_START( stlforce );
VIDEO_UPDATE( stlforce );
WRITE16_HANDLER( stlforce_tx_videoram_w );
WRITE16_HANDLER( stlforce_mhigh_videoram_w );
WRITE16_HANDLER( stlforce_mlow_videoram_w );
WRITE16_HANDLER( stlforce_bg_videoram_w );
