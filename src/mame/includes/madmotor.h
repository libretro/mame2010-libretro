/*************************************************************************

    Mad Motor

*************************************************************************/

class madmotor_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, madmotor_state(machine)); }

	madmotor_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *        pf1_rowscroll;
	uint16_t *        pf1_data;
	uint16_t *        pf2_data;
	uint16_t *        pf3_data;
	uint16_t *        pf1_control;
	uint16_t *        pf2_control;
	uint16_t *        pf3_control;
	uint16_t *        spriteram;
//  uint16_t *        paletteram;     // this currently uses generic palette handlers
	size_t          spriteram_size;

	/* video-related */
	tilemap_t       *pf1_tilemap, *pf2_tilemap, *pf3_tilemap, *pf3a_tilemap;
	int             flipscreen;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/madmotor.c -----------*/

WRITE16_HANDLER( madmotor_pf1_data_w );
WRITE16_HANDLER( madmotor_pf2_data_w );
WRITE16_HANDLER( madmotor_pf3_data_w );

VIDEO_START( madmotor );
VIDEO_UPDATE( madmotor );
