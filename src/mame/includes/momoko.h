/*************************************************************************

    Momoko 120%

*************************************************************************/

class momoko_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, momoko_state(machine)); }

	momoko_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *        bg_scrollx;
	uint8_t *        bg_scrolly;
	uint8_t *        videoram;
	uint8_t *        spriteram;
//  uint8_t *        paletteram;    // currently this uses generic palette handling
	size_t         spriteram_size;
	size_t         videoram_size;

	/* video-related */
	uint8_t          fg_scrollx;
	uint8_t          fg_scrolly;
	uint8_t          fg_select;
	uint8_t          text_scrolly;
	uint8_t          text_mode;
	uint8_t          bg_select;
	uint8_t          bg_priority;
	uint8_t          bg_mask;
	uint8_t          fg_mask;
	uint8_t          flipscreen;
};


/*----------- defined in video/momoko.c -----------*/

WRITE8_HANDLER( momoko_fg_scrollx_w );
WRITE8_HANDLER( momoko_fg_scrolly_w );
WRITE8_HANDLER( momoko_text_scrolly_w );
WRITE8_HANDLER( momoko_text_mode_w );
WRITE8_HANDLER( momoko_bg_scrollx_w );
WRITE8_HANDLER( momoko_bg_scrolly_w );
WRITE8_HANDLER( momoko_flipscreen_w );
WRITE8_HANDLER( momoko_fg_select_w);
WRITE8_HANDLER( momoko_bg_select_w);
WRITE8_HANDLER( momoko_bg_priority_w);

VIDEO_UPDATE( momoko );
