/*************************************************************************

    Contra / Gryzor

*************************************************************************/

class contra_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, contra_state(machine)); }

	contra_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *        spriteram;
	uint8_t *        spriteram_2;
	uint8_t *        paletteram;
	uint8_t *        bg_vram;
	uint8_t *        bg_cram;
	uint8_t *        fg_vram;
	uint8_t *        fg_cram;
	uint8_t *        tx_vram;
	uint8_t *        tx_cram;
	// this driver also uses a large generic spriteram region...

	/* video-related */
	tilemap_t *bg_tilemap, *fg_tilemap, *tx_tilemap;
	rectangle bg_clip, fg_clip, tx_clip;

	/* devices */
	running_device *audiocpu;
	running_device *k007121_1;
	running_device *k007121_2;
};


/*----------- defined in video/contra.c -----------*/

PALETTE_INIT( contra );

WRITE8_HANDLER( contra_fg_vram_w );
WRITE8_HANDLER( contra_fg_cram_w );
WRITE8_HANDLER( contra_bg_vram_w );
WRITE8_HANDLER( contra_bg_cram_w );
WRITE8_HANDLER( contra_text_vram_w );
WRITE8_HANDLER( contra_text_cram_w );

WRITE8_HANDLER( contra_K007121_ctrl_0_w );
WRITE8_HANDLER( contra_K007121_ctrl_1_w );

VIDEO_UPDATE( contra );
VIDEO_START( contra );
