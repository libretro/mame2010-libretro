
class flstory_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, flstory_state(machine)); }

	flstory_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  videoram;
	uint8_t *  workram;
	uint8_t *  scrlram;
	uint8_t *  spriteram;
//  uint8_t *  paletteram;    // currently this uses generic palette handling
//  uint8_t *  paletteram_2;  // currently this uses generic palette handling
	size_t   videoram_size;
	size_t   spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap;
	int      char_bank, palette_bank, flipscreen, gfxctrl;

	/* sound-related */
	uint8_t    snd_data;
	uint8_t    snd_flag;
	int      sound_nmi_enable, pending_nmi;
	int      vol_ctrl[16];
	uint8_t    snd_ctrl0;
	uint8_t    snd_ctrl1;
	uint8_t    snd_ctrl2;
	uint8_t    snd_ctrl3;

	/* protection */
	uint8_t    from_main, from_mcu;
	int      mcu_sent, main_sent;
	uint8_t    port_a_in, port_a_out, ddr_a;
	uint8_t    port_b_in, port_b_out, ddr_b;
	uint8_t    port_c_in, port_c_out, ddr_c;
	int      mcu_select;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *mcu;
};


/*----------- defined in machine/flstory.c -----------*/

READ8_HANDLER( flstory_68705_port_a_r );
WRITE8_HANDLER( flstory_68705_port_a_w );
READ8_HANDLER( flstory_68705_port_b_r );
WRITE8_HANDLER( flstory_68705_port_b_w );
READ8_HANDLER( flstory_68705_port_c_r );
WRITE8_HANDLER( flstory_68705_port_c_w );
WRITE8_HANDLER( flstory_68705_ddr_a_w );
WRITE8_HANDLER( flstory_68705_ddr_b_w );
WRITE8_HANDLER( flstory_68705_ddr_c_w );
WRITE8_HANDLER( flstory_mcu_w );
READ8_HANDLER( flstory_mcu_r );
READ8_HANDLER( flstory_mcu_status_r );
WRITE8_HANDLER( onna34ro_mcu_w );
READ8_HANDLER( onna34ro_mcu_r );
READ8_HANDLER( onna34ro_mcu_status_r );
WRITE8_HANDLER( victnine_mcu_w );
READ8_HANDLER( victnine_mcu_r );
READ8_HANDLER( victnine_mcu_status_r );


/*----------- defined in video/flstory.c -----------*/

VIDEO_START( flstory );
VIDEO_UPDATE( flstory );
VIDEO_START( victnine );
VIDEO_UPDATE( victnine );

WRITE8_HANDLER( flstory_videoram_w );
READ8_HANDLER( flstory_palette_r );
WRITE8_HANDLER( flstory_palette_w );
WRITE8_HANDLER( flstory_gfxctrl_w );
WRITE8_HANDLER( flstory_scrlram_w );
READ8_HANDLER( victnine_gfxctrl_r );
WRITE8_HANDLER( victnine_gfxctrl_w );
