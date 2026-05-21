/*************************************************************************

    Gun Dealer

*************************************************************************/

class gundealr_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gundealr_state(machine)); }

	gundealr_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    bg_videoram;
	uint8_t *    fg_videoram;
	uint8_t *    rambase;
	uint8_t *    paletteram;

	/* video-related */
	tilemap_t    *bg_tilemap;
	tilemap_t    *fg_tilemap;
	int        flipscreen;
	uint8_t      scroll[4];

	/* misc */
	int        input_ports_hack;
};



/*----------- defined in video/gundealr.c -----------*/

WRITE8_HANDLER( gundealr_paletteram_w );
WRITE8_HANDLER( gundealr_bg_videoram_w );
WRITE8_HANDLER( gundealr_fg_videoram_w );
WRITE8_HANDLER( gundealr_fg_scroll_w );
WRITE8_HANDLER( yamyam_fg_scroll_w );
WRITE8_HANDLER( gundealr_flipscreen_w );

VIDEO_UPDATE( gundealr );
VIDEO_START( gundealr );
