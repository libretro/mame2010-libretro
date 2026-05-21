/***************************************************************************

    Kyugo hardware games

***************************************************************************/

class kyugo_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, kyugo_state(machine)); }

	kyugo_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *     fgvideoram;
	uint8_t *     bgvideoram;
	uint8_t *     bgattribram;
	uint8_t *     spriteram_1;
	uint8_t *     spriteram_2;
	uint8_t *     shared_ram;

	/* video-related */
	tilemap_t     *bg_tilemap, *fg_tilemap;
	uint8_t       scroll_x_lo, scroll_x_hi, scroll_y;
	int         bgpalbank, fgcolor;
	int         flipscreen;
	const uint8_t *color_codes;

	/* devices */
	running_device *maincpu;
	running_device *subcpu;
};


/*----------- defined in video/kyugo.c -----------*/

READ8_HANDLER( kyugo_spriteram_2_r );

WRITE8_HANDLER( kyugo_fgvideoram_w );
WRITE8_HANDLER( kyugo_bgvideoram_w );
WRITE8_HANDLER( kyugo_bgattribram_w );
WRITE8_HANDLER( kyugo_scroll_x_lo_w );
WRITE8_HANDLER( kyugo_gfxctrl_w );
WRITE8_HANDLER( kyugo_scroll_y_w );
WRITE8_HANDLER( kyugo_flipscreen_w );

VIDEO_START( kyugo );
VIDEO_UPDATE( kyugo );
