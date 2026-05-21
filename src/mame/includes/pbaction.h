/*************************************************************************

    Pinball Action

*************************************************************************/

class pbaction_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, pbaction_state(machine)); }

	pbaction_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    videoram2;
	uint8_t *    colorram;
	uint8_t *    colorram2;
	uint8_t *    work_ram;
	uint8_t *    spriteram;
//  uint8_t *    paletteram;    // currently this uses generic palette handling
	size_t     spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap, *fg_tilemap;
	int        scroll;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/pbaction.c -----------*/

extern WRITE8_HANDLER( pbaction_videoram_w );
extern WRITE8_HANDLER( pbaction_colorram_w );
extern WRITE8_HANDLER( pbaction_videoram2_w );
extern WRITE8_HANDLER( pbaction_colorram2_w );
extern WRITE8_HANDLER( pbaction_flipscreen_w );
extern WRITE8_HANDLER( pbaction_scroll_w );

extern VIDEO_START( pbaction );
extern VIDEO_UPDATE( pbaction );
