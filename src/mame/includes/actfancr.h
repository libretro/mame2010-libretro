/*************************************************************************

    Act Fancer

*************************************************************************/

class actfancr_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, actfancr_state(machine)); }

	actfancr_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *        pf1_data;
	uint8_t *        pf2_data;
	uint8_t *        pf1_rowscroll_data;
	uint8_t *        main_ram;
//  uint8_t *        spriteram;   // currently this uses buffered_spriteram
//  uint8_t *        paletteram;  // currently this uses generic palette handling

	/* video-related */
	tilemap_t        *pf1_tilemap, *pf1_alt_tilemap, *pf2_tilemap;
	uint8_t          control_1[0x20], control_2[0x20];
	int            flipscreen;

	/* misc */
	int            trio_control_select;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/actfancr.c -----------*/

WRITE8_HANDLER( actfancr_pf1_data_w );
READ8_HANDLER( actfancr_pf1_data_r );
WRITE8_HANDLER( actfancr_pf1_control_w );
WRITE8_HANDLER( actfancr_pf2_data_w );
READ8_HANDLER( actfancr_pf2_data_r );
WRITE8_HANDLER( actfancr_pf2_control_w );

VIDEO_START( actfancr );
VIDEO_START( triothep );

VIDEO_UPDATE( actfancr );
VIDEO_UPDATE( triothep );
