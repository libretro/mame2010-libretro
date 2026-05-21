
class bigevglf_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, bigevglf_state(machine)); }

	bigevglf_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  paletteram;
	uint8_t *  spriteram1;
	uint8_t *  spriteram2;

	/* video-related */
	bitmap_t *tmp_bitmap[4];
	uint8_t    *vidram;
	uint32_t   vidram_bank, plane_selected, plane_visible;

	/* sound-related */
	int      sound_nmi_enable, pending_nmi;
	uint8_t    for_sound;
	uint8_t    from_sound;
	uint8_t    sound_state;

	/* MCU related */
	uint8_t    from_mcu;
	int      mcu_sent, main_sent;
	uint8_t    port_a_in, port_a_out, ddr_a;
	uint8_t    port_b_in, port_b_out, ddr_b;
	uint8_t    port_c_in, port_c_out, ddr_c;
	int      mcu_coin_bit5;

	/* misc */
	uint32_t   beg_bank;
	uint8_t    beg13_ls74[2];
	uint8_t    port_select;     /* for muxed controls */

	/* devices */
	running_device *audiocpu;
	running_device *mcu;
};


/*----------- defined in machine/bigevglf.c -----------*/

READ8_HANDLER( bigevglf_68705_port_a_r );
WRITE8_HANDLER( bigevglf_68705_port_a_w );
READ8_HANDLER( bigevglf_68705_port_b_r );
WRITE8_HANDLER( bigevglf_68705_port_b_w );
READ8_HANDLER( bigevglf_68705_port_c_r );
WRITE8_HANDLER( bigevglf_68705_port_c_w );
WRITE8_HANDLER( bigevglf_68705_ddr_a_w );
WRITE8_HANDLER( bigevglf_68705_ddr_b_w );
WRITE8_HANDLER( bigevglf_68705_ddr_c_w );

WRITE8_HANDLER( bigevglf_mcu_w );
READ8_HANDLER( bigevglf_mcu_r );
READ8_HANDLER( bigevglf_mcu_status_r );


/*----------- defined in video/bigevglf.c -----------*/

VIDEO_START( bigevglf );
VIDEO_UPDATE( bigevglf );

READ8_HANDLER( bigevglf_vidram_r );
WRITE8_HANDLER( bigevglf_vidram_w );
WRITE8_HANDLER( bigevglf_vidram_addr_w );

WRITE8_HANDLER( bigevglf_gfxcontrol_w );
WRITE8_HANDLER( bigevglf_palette_w );
