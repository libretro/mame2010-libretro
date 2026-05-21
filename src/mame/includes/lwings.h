
class lwings_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, lwings_state(machine)); }

	lwings_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  fgvideoram;
	uint8_t *  bg1videoram;
	uint8_t *  soundlatch2;
//      uint8_t *  spriteram; // currently this uses generic buffered spriteram
//      uint8_t *  paletteram;    // currently this uses generic palette handling
//      uint8_t *  paletteram2;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *fg_tilemap, *bg1_tilemap, *bg2_tilemap;
	uint8_t    bg2_image;
	int      bg2_avenger_hw;
	uint8_t    scroll_x[2], scroll_y[2];

	/* misc */
	uint8_t    param[4];
	int      palette_pen;
	uint8_t    soundstate;
	uint8_t    adpcm;
};


/*----------- defined in video/lwings.c -----------*/

WRITE8_HANDLER( lwings_fgvideoram_w );
WRITE8_HANDLER( lwings_bg1videoram_w );
WRITE8_HANDLER( lwings_bg1_scrollx_w );
WRITE8_HANDLER( lwings_bg1_scrolly_w );
WRITE8_HANDLER( trojan_bg2_scrollx_w );
WRITE8_HANDLER( trojan_bg2_image_w );

VIDEO_START( lwings );
VIDEO_START( trojan );
VIDEO_START( avengers );
VIDEO_UPDATE( lwings );
VIDEO_UPDATE( trojan );
VIDEO_EOF( lwings );
