/***************************************************************************

    Bionic Commando

***************************************************************************/

class bionicc_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, bionicc_state(machine)); }

	bionicc_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *  bgvideoram;
	uint16_t *  fgvideoram;
	uint16_t *  txvideoram;
	uint16_t *  paletteram;
//  uint16_t *  spriteram;  // needed for EOF, but currently handled through buffer_spriteram16

	/* video-related */
	tilemap_t   *tx_tilemap, *bg_tilemap, *fg_tilemap;
	uint16_t    scroll[4];

	uint16_t    inp[3];
	uint16_t    soundcommand;
};


/*----------- defined in video/bionicc.c -----------*/

WRITE16_HANDLER( bionicc_fgvideoram_w );
WRITE16_HANDLER( bionicc_bgvideoram_w );
WRITE16_HANDLER( bionicc_txvideoram_w );
WRITE16_HANDLER( bionicc_paletteram_w );
WRITE16_HANDLER( bionicc_scroll_w );
WRITE16_HANDLER( bionicc_gfxctrl_w );

VIDEO_START( bionicc );
VIDEO_UPDATE( bionicc );
VIDEO_EOF( bionicc );
