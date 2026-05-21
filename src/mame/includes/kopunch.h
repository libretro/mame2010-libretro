/*************************************************************************

    KO Punch

*************************************************************************/

class kopunch_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, kopunch_state(machine)); }

	kopunch_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    videoram2;
	uint8_t *    colorram;
	uint8_t *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap, *fg_tilemap;
	int        gfxbank;

	/* devices */
	running_device *maincpu;
};

/*----------- defined in video/kopunch.c -----------*/

WRITE8_HANDLER( kopunch_videoram_w );
WRITE8_HANDLER( kopunch_videoram2_w );
WRITE8_HANDLER( kopunch_scroll_x_w );
WRITE8_HANDLER( kopunch_scroll_y_w );
WRITE8_HANDLER( kopunch_gfxbank_w );

PALETTE_INIT( kopunch );
VIDEO_START( kopunch );
VIDEO_UPDATE( kopunch );
