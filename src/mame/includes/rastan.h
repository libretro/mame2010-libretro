/*************************************************************************

    Rastan

*************************************************************************/

class rastan_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, rastan_state(machine)); }

	rastan_state(running_machine &machine) { }

	/* memory pointers */
//  uint16_t *    paletteram; // this currently uses generic palette handlers

	/* video-related */
	uint16_t      sprite_ctrl;
	uint16_t      sprites_flipscreen;

	/* misc */
	int         adpcm_pos;
	int         adpcm_data;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *pc090oj;
	running_device *pc080sn;
};


/*----------- defined in video/rastan.c -----------*/

WRITE16_HANDLER( rastan_spritectrl_w );

VIDEO_UPDATE( rastan );
