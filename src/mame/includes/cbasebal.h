/*************************************************************************

    Capcom Baseball

*************************************************************************/

class cbasebal_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cbasebal_state(machine)); }

	cbasebal_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    spriteram;
//  uint8_t *    paletteram;    // currently this uses generic palette handling
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *fg_tilemap, *bg_tilemap;
	uint8_t      *textram, *scrollram;
	uint8_t      scroll_x[2], scroll_y[2];
	int        tilebank, spritebank;
	int        text_on, bg_on, obj_on;
	int        flipscreen;

	/* misc */
	uint8_t      rambank;
};

/*----------- defined in video/cbasebal.c -----------*/

WRITE8_HANDLER( cbasebal_textram_w );
READ8_HANDLER( cbasebal_textram_r );
WRITE8_HANDLER( cbasebal_scrollram_w );
READ8_HANDLER( cbasebal_scrollram_r );
WRITE8_HANDLER( cbasebal_gfxctrl_w );
WRITE8_HANDLER( cbasebal_scrollx_w );
WRITE8_HANDLER( cbasebal_scrolly_w );

VIDEO_START( cbasebal );
VIDEO_UPDATE( cbasebal );
