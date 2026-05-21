
class segas1x_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, segas1x_state(machine)); }

	segas1x_state(running_machine &machine)
		: interrupt_timer(machine.device<timer_device>("int_timer")) { }

	/* memory pointers */
//  uint16_t *  workram;  // this is used in the nvram handler, hence it cannot be added here
//  uint16_t *  paletteram;   // this is used in the segaic16 mapper, hence it cannot be added here (yet)
//  uint16_t *  tileram_0;    // this is used in the segaic16 mapper, hence it cannot be added here (yet)
//  uint16_t *  textram_0;    // this is used in the segaic16 mapper, hence it cannot be added here (yet)
//  uint16_t *  spriteram_0;  // this is used in the segaic16 mapper, hence it cannot be added here (yet)

	/* misc video */
	uint8_t road_priority;		// segaxbd
	bitmap_t *tmp_bitmap;		// segaybd & segas18
	uint8_t grayscale_enable;		// segas18
	uint8_t vdp_enable;			// segas18
	uint8_t vdp_mixing;			// segas18

	/* misc common */
	uint8_t rom_board;			// segas16b
	uint8_t mj_input_num;		// segas16a & segas16b
	uint8_t mj_last_val;		// segas16b
	uint8_t adc_select;			// segahang & segaorun
	uint8_t timer_irq_state;		// segaxbd & segaybd
	uint8_t vblank_irq_state;		// segaorun, segaxbd & segaybd
	uint8_t misc_io_data[0x10];	// system18 & segaybd

	void (*i8751_vblank_hook)(running_machine *machine);
	const uint8_t *i8751_initial_config;

	read16_space_func custom_io_r;
	write16_space_func custom_io_w;

	/* misc system 16b */
	uint8_t atomicp_sound_divisor;
	uint8_t atomicp_sound_count;
	uint8_t disable_screen_blanking;
	uint8_t hwc_input_value;


	/* misc system 16a */
	uint8_t video_control;
	uint8_t mcu_control;
	uint8_t n7751_command;
	uint32_t n7751_rom_address;
	uint8_t last_buttons1;
	uint8_t last_buttons2;
	int read_port;

	void (*lamp_changed_w)(running_machine *machine, uint8_t changed, uint8_t newval);

	/* misc system 18 */
	uint8_t mcu_data;

	uint8_t wwally_last_x[3], wwally_last_y[3];
	uint8_t lghost_value, lghost_select;

	/* misc segaorun */
	uint8_t irq2_state;
	const uint8_t *custom_map;

	/* misc yboard */
	uint8_t analog_data[4];
	int irq2_scanline;

	/* misc xboard */
	uint8_t iochip_regs[2][8];
	uint8_t iochip_force_input;

	uint8_t (*iochip_custom_io_r[2])(offs_t offset, uint8_t portdata);	// currently unused
	void (*iochip_custom_io_w[2])(offs_t offset, uint8_t data);	// currently unused

	uint8_t adc_reverse[8];

	uint8_t gprider_hack;
	uint16_t *loffire_sync;


	/* devices */
	running_device *maincpu;
	running_device *soundcpu;
	running_device *subcpu;
	running_device *subx;
	running_device *suby;
	running_device *mcu;
	running_device *ymsnd;
	running_device *ppi8255;
	running_device *n7751;
	running_device *ppi8255_1;
	running_device *ppi8255_2;
	timer_device *interrupt_timer;
	running_device *_315_5248_1;
	running_device *_315_5250_1;
	running_device *_315_5250_2;
};


/*----------- defined in video/segahang.c -----------*/

VIDEO_START( hangon );
VIDEO_START( sharrier );
VIDEO_UPDATE( hangon );

/*----------- defined in video/segas16a.c -----------*/

VIDEO_START( system16a );
VIDEO_UPDATE( system16a );

/*----------- defined in video/segas16b.c -----------*/

VIDEO_START( system16b );
VIDEO_START( timscanr );
VIDEO_UPDATE( system16b );

/*----------- defined in video/segas18.c -----------*/

VIDEO_START( system18 );
VIDEO_UPDATE( system18 );

void system18_set_grayscale(running_machine *machine, int enable);
void system18_set_vdp_enable(running_machine *machine, int eanble);
void system18_set_vdp_mixing(running_machine *machine, int mixing);

/*----------- defined in video/segaorun.c -----------*/

VIDEO_START( outrun );
VIDEO_START( shangon );
VIDEO_UPDATE( outrun );
VIDEO_UPDATE( shangon );

/*----------- defined in video/segaxbd.c -----------*/

VIDEO_START( xboard );
VIDEO_UPDATE( xboard );

/*----------- defined in video/segaybd.c -----------*/

VIDEO_START( yboard );
VIDEO_UPDATE( yboard );


/*----------- defined in machine/s16fd.c -----------*/

void *fd1094_get_decrypted_base(void);
void fd1094_machine_init(running_device *device);
void fd1094_driver_init(running_machine *machine, const char* tag, void (*set_decrypted)(running_machine *, uint8_t *));
