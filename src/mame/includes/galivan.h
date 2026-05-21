/***************************************************************************

    Galivan - Cosmo Police

***************************************************************************/

class galivan_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, galivan_state(machine)); }

	galivan_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *     videoram;
	uint8_t *     colorram;
	uint8_t *     spriteram;
	size_t      videoram_size;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *bg_tilemap, *tx_tilemap;
	uint8_t       scrollx[2], scrolly[2];
	uint8_t       flipscreen;
	uint8_t       write_layers, layers;
	uint8_t       ninjemak_dispdisable;
};



/*----------- defined in video/galivan.c -----------*/

WRITE8_HANDLER( galivan_scrollx_w );
WRITE8_HANDLER( galivan_scrolly_w );
WRITE8_HANDLER( galivan_videoram_w );
WRITE8_HANDLER( galivan_colorram_w );
WRITE8_HANDLER( galivan_gfxbank_w );
WRITE8_HANDLER( ninjemak_scrollx_w );
WRITE8_HANDLER( ninjemak_scrolly_w );
WRITE8_HANDLER( ninjemak_gfxbank_w );

PALETTE_INIT( galivan );

VIDEO_START( galivan );
VIDEO_START( ninjemak );
VIDEO_UPDATE( galivan );
VIDEO_UPDATE( ninjemak );
