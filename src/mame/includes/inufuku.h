
class inufuku_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, inufuku_state(machine)); }

	inufuku_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *  bg_videoram;
	uint16_t *  bg_rasterram;
	uint16_t *  tx_videoram;
	uint16_t *  spriteram1;
	uint16_t *  spriteram2;
//      uint16_t *  paletteram;    // currently this uses generic palette handling
	size_t    spriteram1_size;

	/* video-related */
	tilemap_t  *bg_tilemap,*tx_tilemap;
	int       bg_scrollx, bg_scrolly;
	int       tx_scrollx, tx_scrolly;
	int       bg_raster;
	int       bg_palettebank, tx_palettebank;

	/* misc */
	uint16_t    pending_command;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/inufuku.c -----------*/

READ16_HANDLER( inufuku_bg_videoram_r );
WRITE16_HANDLER( inufuku_bg_videoram_w );
READ16_HANDLER( inufuku_tx_videoram_r );
WRITE16_HANDLER( inufuku_tx_videoram_w );
WRITE16_HANDLER( inufuku_palettereg_w );
WRITE16_HANDLER( inufuku_scrollreg_w );

VIDEO_UPDATE( inufuku );
VIDEO_START( inufuku );
