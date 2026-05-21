/*************************************************************************

    Macross Plus

*************************************************************************/

class macrossp_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, macrossp_state(machine)); }

	macrossp_state(running_machine &machine) { }

	/* memory pointers */
	uint32_t *         mainram;
	uint32_t *         scra_videoram;
	uint32_t *         scra_videoregs;
	uint32_t *         scrb_videoram;
	uint32_t *         scrb_videoregs;
	uint32_t *         scrc_videoram;
	uint32_t *         scrc_videoregs;
	uint32_t *         text_videoram;
	uint32_t *         text_videoregs;
	uint32_t *         spriteram;
	uint32_t *         spriteram_old;
	uint32_t *         spriteram_old2;
	uint32_t *         paletteram;
	size_t           spriteram_size;

	/* video-related */
	tilemap_t  *scra_tilemap, *scrb_tilemap, *scrc_tilemap, *text_tilemap;

	/* misc */
	int              sndpending;
	int              snd_toggle;
	int32_t            fade_effect, old_fade;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

/*----------- defined in video/macrossp.c -----------*/

WRITE32_HANDLER( macrossp_scra_videoram_w );
WRITE32_HANDLER( macrossp_scrb_videoram_w );
WRITE32_HANDLER( macrossp_scrc_videoram_w );
WRITE32_HANDLER( macrossp_text_videoram_w );

VIDEO_START(macrossp);
VIDEO_UPDATE(macrossp);
VIDEO_EOF(macrossp);
