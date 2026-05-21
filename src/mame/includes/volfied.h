/*************************************************************************

    Volfied

*************************************************************************/

class volfied_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, volfied_state(machine)); }

	volfied_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *    video_ram;
	uint8_t  *    cchip_ram;
//  uint16_t *    paletteram; // this currently uses generic palette handlers

	/* video-related */
	uint16_t      video_ctrl;
	uint16_t      video_mask;

	/* c-chip */
	uint8_t       current_bank;
	uint8_t       current_flag;
	uint8_t       cc_port;
	uint8_t       current_cmd;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *pc090oj;
};


/*----------- defined in machine/volfied.c -----------*/

void volfied_cchip_init(running_machine *machine);
void volfied_cchip_reset(running_machine *machine);

READ16_HANDLER( volfied_cchip_ctrl_r );
READ16_HANDLER( volfied_cchip_ram_r );
WRITE16_HANDLER( volfied_cchip_ctrl_w );
WRITE16_HANDLER( volfied_cchip_bank_w );
WRITE16_HANDLER( volfied_cchip_ram_w );


/*----------- defined in video/volfied.c -----------*/

WRITE16_HANDLER( volfied_sprite_ctrl_w );
WRITE16_HANDLER( volfied_video_ram_w );
WRITE16_HANDLER( volfied_video_ctrl_w );
WRITE16_HANDLER( volfied_video_mask_w );

READ16_HANDLER( volfied_video_ram_r );
READ16_HANDLER( volfied_video_ctrl_r );

VIDEO_UPDATE( volfied );
VIDEO_START( volfied );
