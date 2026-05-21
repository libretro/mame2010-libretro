/*************************************************************************

    Commando

*************************************************************************/

class commando_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, commando_state(machine)); }

	commando_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  videoram;
	uint8_t *  colorram;
	uint8_t *  videoram2;
	uint8_t *  colorram2;
//  uint8_t *  spriteram; // currently this uses generic buffered_spriteram

	/* video-related */
	tilemap_t  *bg_tilemap, *fg_tilemap;
	uint8_t scroll_x[2];
	uint8_t scroll_y[2];

	/* devices */
	running_device *audiocpu;
};



/*----------- defined in video/commando.c -----------*/

WRITE8_HANDLER( commando_videoram_w );
WRITE8_HANDLER( commando_colorram_w );
WRITE8_HANDLER( commando_videoram2_w );
WRITE8_HANDLER( commando_colorram2_w );
WRITE8_HANDLER( commando_scrollx_w );
WRITE8_HANDLER( commando_scrolly_w );
WRITE8_HANDLER( commando_c804_w );

VIDEO_START( commando );
VIDEO_UPDATE( commando );
VIDEO_EOF( commando );
