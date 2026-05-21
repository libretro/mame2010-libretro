/***************************************************************************

    Ninja Gaiden

***************************************************************************/

class gaiden_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gaiden_state(machine)); }

	gaiden_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *    videoram;
	uint16_t *    videoram2;
	uint16_t *    videoram3;
	uint16_t *    spriteram;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t   *text_layer,*foreground,*background;
	bitmap_t    *sprite_bitmap, *tile_bitmap_bg, *tile_bitmap_fg;
	uint16_t      tx_scroll_x, tx_scroll_y;
	uint16_t      bg_scroll_x, bg_scroll_y;
	uint16_t      fg_scroll_x, fg_scroll_y;
	int8_t		tx_offset_y, bg_offset_y, fg_offset_y, spr_offset_y;

	/* misc */
	int         sprite_sizey;
	int         prot, jumpcode;
	const int   *raiga_jumppoints;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/gaiden.c -----------*/

VIDEO_START( gaiden );
VIDEO_START( raiga );
VIDEO_START( drgnbowl );
VIDEO_START( mastninj );

VIDEO_UPDATE( gaiden );
VIDEO_UPDATE( raiga );
VIDEO_UPDATE( drgnbowl );

WRITE16_HANDLER( gaiden_videoram_w );
WRITE16_HANDLER( gaiden_videoram2_w );
READ16_HANDLER( gaiden_videoram2_r );
WRITE16_HANDLER( gaiden_videoram3_w );
READ16_HANDLER( gaiden_videoram3_r );

WRITE16_HANDLER( gaiden_flip_w );
WRITE16_HANDLER( gaiden_txscrollx_w );
WRITE16_HANDLER( gaiden_txscrolly_w );
WRITE16_HANDLER( gaiden_fgscrollx_w );
WRITE16_HANDLER( gaiden_fgscrolly_w );
WRITE16_HANDLER( gaiden_bgscrollx_w );
WRITE16_HANDLER( gaiden_bgscrolly_w );
WRITE16_HANDLER( gaiden_txoffsety_w );
WRITE16_HANDLER( gaiden_fgoffsety_w );
WRITE16_HANDLER( gaiden_bgoffsety_w );
WRITE16_HANDLER( gaiden_sproffsety_w );
