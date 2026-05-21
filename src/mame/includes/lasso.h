/***************************************************************************

 Lasso and similar hardware

***************************************************************************/

class lasso_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, lasso_state(machine)); }

	lasso_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  videoram;
	uint8_t *  colorram;
	uint8_t *  spriteram;
	uint8_t *  bitmap_ram;	/* 0x2000 bytes for a 256 x 256 x 1 bitmap */
	uint8_t *  back_color;
	uint8_t *  chip_data;
	uint8_t *  track_scroll;
	uint8_t *  last_colors;
	size_t   spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap, *track_tilemap;
	uint8_t    gfxbank;		/* used by lasso, chameleo, wwjgtin and pinbo */
	uint8_t    track_enable;	/* used by wwjgtin */

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *sn_1;
	running_device *sn_2;
};


/*----------- defined in video/lasso.c -----------*/

WRITE8_HANDLER( lasso_videoram_w );
WRITE8_HANDLER( lasso_colorram_w );
WRITE8_HANDLER( lasso_video_control_w );
WRITE8_HANDLER( wwjgtin_video_control_w );
WRITE8_HANDLER( pinbo_video_control_w );

PALETTE_INIT( lasso );
PALETTE_INIT( wwjgtin );

VIDEO_START( lasso );
VIDEO_START( wwjgtin );
VIDEO_START( pinbo );

VIDEO_UPDATE( lasso );
VIDEO_UPDATE( chameleo );
VIDEO_UPDATE( wwjgtin );
VIDEO_UPDATE( pinbo );
