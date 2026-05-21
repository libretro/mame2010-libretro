/*************************************************************************

    Street Fighter

*************************************************************************/

class sf_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, sf_state(machine)); }

	sf_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *    videoram;
	uint16_t *    objectram;
//  uint16_t *    paletteram;    // currently this uses generic palette handling
	size_t      videoram_size;

	/* video-related */
	tilemap_t     *bg_tilemap, *fg_tilemap, *tx_tilemap;
	int         sf_active;
	uint16_t      bgscroll, fgscroll;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/sf.c -----------*/

WRITE16_HANDLER( sf_bg_scroll_w );
WRITE16_HANDLER( sf_fg_scroll_w );
WRITE16_HANDLER( sf_videoram_w );
WRITE16_HANDLER( sf_gfxctrl_w );

VIDEO_START( sf );
VIDEO_UPDATE( sf );
