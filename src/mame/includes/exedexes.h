/*************************************************************************

    Exed Exes

*************************************************************************/


class exedexes_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, exedexes_state(machine)); }

	exedexes_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *        videoram;
	uint8_t *        colorram;
	uint8_t *        bg_scroll;
	uint8_t *        nbg_yscroll;
	uint8_t *        nbg_xscroll;
//  uint8_t *        spriteram;   // currently this uses generic buffered_spriteram

	/* video-related */
	tilemap_t        *bg_tilemap, *fg_tilemap, *tx_tilemap;
	int            chon, objon, sc1on, sc2on;
};



/*----------- defined in video/exedexes.c -----------*/

extern WRITE8_HANDLER( exedexes_videoram_w );
extern WRITE8_HANDLER( exedexes_colorram_w );
extern WRITE8_HANDLER( exedexes_c804_w );
extern WRITE8_HANDLER( exedexes_gfxctrl_w );

extern PALETTE_INIT( exedexes );
extern VIDEO_START( exedexes );
extern VIDEO_UPDATE( exedexes );
extern VIDEO_EOF( exedexes );
