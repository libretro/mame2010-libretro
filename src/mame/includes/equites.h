
#define POPDRUMKIT 0


class equites_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, equites_state(machine)); }

	equites_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *  bg_videoram;
	uint8_t  *  fg_videoram;	// 8bits
	uint16_t *  spriteram;
	uint16_t *  spriteram_2;
	uint16_t *  workram;
	uint8_t  *  mcu_ram;	// 8bits
//  uint16_t *  nvram;    // currently this uses generic nvram handling

	/* video-related */
	tilemap_t   *fg_tilemap, *bg_tilemap;
	int       fg_char_bank;
	uint8_t     bgcolor;
	uint16_t    splndrbt_bg_scrollx, splndrbt_bg_scrolly;

	/* misc */
	int       sound_prom_address;
	uint8_t     dac_latch;
	uint8_t     eq8155_port_b;
	uint8_t     eq8155_port_a,eq8155_port_c,ay_port_a,ay_port_b,eq_cymbal_ctrl;
	emu_timer *nmi_timer, *adjuster_timer;
	float     cymvol,hihatvol;
	int       timer_count;
	int       unknown_bit;	// Gekisou special handling
#if POPDRUMKIT
	int       hihat,cymbal;
#endif

	/* devices */
	running_device *mcu;
	running_device *audio_cpu;
	running_device *msm;
	running_device *dac_1;
	running_device *dac_2;
};


/*----------- defined in video/equites.c -----------*/

extern READ16_HANDLER(equites_fg_videoram_r);
extern WRITE16_HANDLER(equites_fg_videoram_w);
extern WRITE16_HANDLER(equites_bg_videoram_w);
extern WRITE16_HANDLER(equites_scrollreg_w);
extern WRITE16_HANDLER(equites_bgcolor_w);
extern WRITE16_HANDLER(splndrbt_selchar0_w);
extern WRITE16_HANDLER(splndrbt_selchar1_w);
extern WRITE16_HANDLER(equites_flip0_w);
extern WRITE16_HANDLER(equites_flip1_w);
extern WRITE16_HANDLER(splndrbt_flip0_w);
extern WRITE16_HANDLER(splndrbt_flip1_w);
extern WRITE16_HANDLER(splndrbt_bg_scrollx_w);
extern WRITE16_HANDLER(splndrbt_bg_scrolly_w);

extern PALETTE_INIT( equites );
extern VIDEO_START( equites );
extern VIDEO_UPDATE( equites );
extern PALETTE_INIT( splndrbt );
extern VIDEO_START( splndrbt );
extern VIDEO_UPDATE( splndrbt );
