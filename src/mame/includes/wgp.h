/*************************************************************************

    World Grand Prix

*************************************************************************/

class wgp_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, wgp_state(machine)); }

	wgp_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *    spritemap;
	uint16_t *    spriteram;
	uint16_t *    pivram;
	uint16_t *    piv_ctrlram;
	uint16_t *    sharedram;
//  uint16_t *    paletteram;    // currently this uses generic palette handling
	size_t      sharedram_size;
	size_t      spritemap_size;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t   *piv_tilemap[3];
	uint16_t      piv_ctrl_reg;
	uint16_t      piv_zoom[3], piv_scrollx[3], piv_scrolly[3];
	uint16_t      rotate_ctrl[8];
	int         piv_xoffs, piv_yoffs;

	/* misc */
	uint16_t      cpua_ctrl;
	uint16_t      port_sel;
	int32_t       banknum;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *subcpu;
	running_device *tc0100scn;
	running_device *tc0140syt;
};


/*----------- defined in video/wgp.c -----------*/

READ16_HANDLER ( wgp_pivram_word_r );
WRITE16_HANDLER( wgp_pivram_word_w );

READ16_HANDLER ( wgp_piv_ctrl_word_r );
WRITE16_HANDLER( wgp_piv_ctrl_word_w );

VIDEO_START( wgp );
VIDEO_START( wgp2 );
VIDEO_UPDATE( wgp );
