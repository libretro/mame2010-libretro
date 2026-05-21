/*************************************************************************


    Taito Z system

*************************************************************************/

class taitoz_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, taitoz_state(machine)); }

	taitoz_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *    spriteram;
//  uint16_t *    paletteram;    // currently this uses generic palette handling
	offs_t      spriteram_size;

	/* video-related */
	int         sci_spriteframe;
	int         road_palbank;

	/* misc */
	int         chasehq_lamps;
	int32_t       banknum;
	uint16_t      cpua_ctrl;
	int32_t       sci_int6;
	int32_t       dblaxle_int6;
	int32_t       ioc220_port;
	uint16_t      eep_latch;

//  uint8_t       pandata[4];

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *subcpu;
	running_device *eeprom;
	running_device *tc0480scp;
	running_device *tc0150rod;
	running_device *tc0100scn;
	running_device *tc0220ioc;
	running_device *tc0140syt;

	/* dblaxle motor flag */
	int	    dblaxle_vibration;
};

/*----------- defined in video/taito_z.c -----------*/

WRITE16_HANDLER( contcirc_out_w );
READ16_HANDLER ( sci_spriteframe_r );
WRITE16_HANDLER( sci_spriteframe_w );

VIDEO_START( taitoz );

VIDEO_UPDATE( contcirc );
VIDEO_UPDATE( chasehq );
VIDEO_UPDATE( bshark );
VIDEO_UPDATE( sci );
VIDEO_UPDATE( aquajack );
VIDEO_UPDATE( spacegun );
VIDEO_UPDATE( dblaxle );
