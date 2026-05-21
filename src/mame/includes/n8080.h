
class n8080_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, n8080_state(machine)); }

	n8080_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t * videoram;
	uint8_t * colorram;	// for helifire

	/* video-related */
	emu_timer* cannon_timer;
	int spacefev_red_screen;
	int spacefev_red_cannon;
	int sheriff_color_mode;
	int sheriff_color_data;
	int helifire_flash;
	uint8_t helifire_LSFR[63];
	unsigned helifire_mv;
	unsigned helifire_sc; /* IC56 */

	/* sound-related */
	int n8080_hardware;
	emu_timer* sound_timer[3];
	int helifire_dac_phase;
	double helifire_dac_volume;
	double helifire_dac_timing;
	uint16_t prev_sound_pins;
	uint16_t curr_sound_pins;
	int mono_flop[3];
	uint8_t prev_snd_data;

	/* other */
	unsigned shift_data;
	unsigned shift_bits;
	int inte;

	/* devices */
	running_device *maincpu;
};



/*----------- defined in video/n8080.c -----------*/

WRITE8_HANDLER( n8080_video_control_w );

PALETTE_INIT( n8080 );
PALETTE_INIT( helifire );

VIDEO_START( spacefev );
VIDEO_START( sheriff );
VIDEO_START( helifire );
VIDEO_UPDATE( spacefev );
VIDEO_UPDATE( sheriff );
VIDEO_UPDATE( helifire );
VIDEO_EOF( helifire );

void spacefev_start_red_cannon(running_machine *machine);

/*----------- defined in audio/n8080.c -----------*/

MACHINE_DRIVER_EXTERN( spacefev_sound );
MACHINE_DRIVER_EXTERN( sheriff_sound );
MACHINE_DRIVER_EXTERN( helifire_sound );

MACHINE_START( spacefev_sound );
MACHINE_START( sheriff_sound );
MACHINE_START( helifire_sound );
MACHINE_RESET( spacefev_sound );
MACHINE_RESET( sheriff_sound );
MACHINE_RESET( helifire_sound );

WRITE8_HANDLER( n8080_sound_1_w );
WRITE8_HANDLER( n8080_sound_2_w );
