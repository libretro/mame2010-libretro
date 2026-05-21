/*************************************************************************

    Super Slams

*************************************************************************/

class suprslam_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, suprslam_state(machine)); }

	suprslam_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *    screen_videoram;
	uint16_t *    bg_videoram;
	uint16_t *    sp_videoram;
	uint16_t *    spriteram;
//  uint16_t *    paletteram; // this currently uses generic palette handling

	/* video-related */
	tilemap_t     *screen_tilemap, *bg_tilemap;
	uint16_t      screen_bank, bg_bank;
	uint16_t      *spr_ctrl;
	uint16_t      *screen_vregs;

	/* misc */
	int         pending_command;

	/* devices */
	running_device *audiocpu;
	running_device *k053936;
};


/*----------- defined in video/suprslam.c -----------*/

WRITE16_HANDLER( suprslam_screen_videoram_w );
WRITE16_HANDLER( suprslam_bg_videoram_w );
WRITE16_HANDLER( suprslam_bank_w );

VIDEO_START( suprslam );
VIDEO_UPDATE( suprslam );
