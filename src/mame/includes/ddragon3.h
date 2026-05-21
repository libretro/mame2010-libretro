/*************************************************************************

    Double Dragon 3 & The Combatribes

*************************************************************************/


class ddragon3_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ddragon3_state(machine)); }

	ddragon3_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *        bg_videoram;
	uint16_t *        fg_videoram;
	uint16_t *        spriteram;
//  uint16_t *        paletteram; // currently this uses generic palette handling

	/* video-related */
	tilemap_t         *fg_tilemap, *bg_tilemap;
	uint16_t          vreg;
	uint16_t          bg_scrollx;
	uint16_t          bg_scrolly;
	uint16_t          fg_scrollx;
	uint16_t          fg_scrolly;
	uint16_t          bg_tilebase;

	/* misc */
	uint16_t          io_reg[8];

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/ddragon3.c -----------*/

extern WRITE16_HANDLER( ddragon3_bg_videoram_w );
extern WRITE16_HANDLER( ddragon3_fg_videoram_w );
extern WRITE16_HANDLER( ddragon3_scroll_w );
extern READ16_HANDLER( ddragon3_scroll_r );

extern VIDEO_START( ddragon3 );
extern VIDEO_UPDATE( ddragon3 );
extern VIDEO_UPDATE( ctribe );
