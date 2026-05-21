/*************************************************************************

    Operation Wolf

*************************************************************************/

class opwolf_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, opwolf_state(machine)); }

	opwolf_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *      cchip_ram;

	/* video-related */
	uint16_t       sprite_ctrl;
	uint16_t       sprites_flipscreen;

	/* misc */
	uint8_t        adpcm_b[0x08];
	uint8_t        adpcm_c[0x08];
	uint32_t       adpcm_pos[2], adpcm_end[2];
	int          adpcm_data[2];

	int          opwolf_gun_xoffs, opwolf_gun_yoffs;

	/* c-chip */
	int          opwolf_region;

	uint8_t        current_bank;
	uint8_t        current_cmd;
	uint8_t        cchip_last_7a;
	uint8_t        cchip_last_04;
	uint8_t        cchip_last_05;
	uint8_t        cchip_coins_for_credit[2];
	uint8_t        cchip_credits_for_coin[2];
	uint8_t        cchip_coins[2];
	uint8_t        c588, c589, c58a; // These variables derived from the bootleg

	/* devices */
	cpu_device *maincpu;
	cpu_device *audiocpu;
	running_device *pc080sn;
	running_device *pc090oj;
	running_device *msm1;
	running_device *msm2;
};


/*----------- defined in machine/opwolf.c -----------*/

void opwolf_cchip_init(running_machine *machine);

READ16_HANDLER( opwolf_cchip_status_r );
READ16_HANDLER( opwolf_cchip_data_r );
WRITE16_HANDLER( opwolf_cchip_status_w );
WRITE16_HANDLER( opwolf_cchip_data_w );
WRITE16_HANDLER( opwolf_cchip_bank_w );


/*----------- defined in video/rastan.c -----------*/

WRITE16_HANDLER( opwolf_spritectrl_w );

VIDEO_UPDATE( opwolf );
