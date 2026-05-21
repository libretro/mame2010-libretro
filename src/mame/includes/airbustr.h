/*************************************************************************

    Air Buster

*************************************************************************/

class airbustr_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, airbustr_state(machine)); }

	airbustr_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    videoram2;
	uint8_t *    colorram;
	uint8_t *    colorram2;
	uint8_t *    paletteram;
	uint8_t *    devram;

	/* video-related */
	tilemap_t    *bg_tilemap, *fg_tilemap;
	bitmap_t   *sprites_bitmap;
	int        bg_scrollx, bg_scrolly, fg_scrollx, fg_scrolly, highbits;

	/* misc */
	int        soundlatch_status, soundlatch2_status;
	int        master_addr;
	int        slave_addr;

	/* devices */
	running_device *master;
	running_device *slave;
	running_device *audiocpu;
	running_device *pandora;
};


/*----------- defined in video/airbustr.c -----------*/

WRITE8_HANDLER( airbustr_videoram_w );
WRITE8_HANDLER( airbustr_colorram_w );
WRITE8_HANDLER( airbustr_videoram2_w );
WRITE8_HANDLER( airbustr_colorram2_w );
WRITE8_HANDLER( airbustr_scrollregs_w );

VIDEO_START( airbustr );
VIDEO_UPDATE( airbustr );
VIDEO_EOF( airbustr );
