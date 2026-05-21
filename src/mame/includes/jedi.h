/*************************************************************************

    Atari Return of the Jedi hardware

*************************************************************************/


/* oscillators and clocks */
#define JEDI_MAIN_CPU_OSC		(XTAL_10MHz)
#define JEDI_AUDIO_CPU_OSC		(XTAL_12_096MHz)
#define JEDI_MAIN_CPU_CLOCK		(JEDI_MAIN_CPU_OSC / 4)
#define JEDI_AUDIO_CPU_CLOCK	(JEDI_AUDIO_CPU_OSC / 8)
#define JEDI_POKEY_CLOCK		(JEDI_AUDIO_CPU_CLOCK)
#define JEDI_TMS5220_CLOCK		(JEDI_AUDIO_CPU_OSC / 2 / 9) /* div by 9 is via a binary counter that counts from 7 to 16 */


class jedi_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, jedi_state(machine)); }

	jedi_state(running_machine &machine) { }

	/* machine state */
	uint8_t  a2d_select;
	uint8_t  nvram_enabled;
	emu_timer *interrupt_timer;

	/* video state */
	uint8_t *foregroundram;
	uint8_t *backgroundram;
	uint8_t *spriteram;
	uint8_t *paletteram;
	uint8_t *foreground_bank;
	uint8_t *video_off;
	uint8_t *smoothing_table;
	uint32_t vscroll;
	uint32_t hscroll;

	/* audio state */
	uint8_t  audio_latch;
	uint8_t  audio_ack_latch;
	uint8_t *audio_comm_stat;
	uint8_t *speech_data;
	uint8_t  speech_strobe_state;
};


/*----------- defined in audio/jedi.c -----------*/

MACHINE_DRIVER_EXTERN( jedi_audio );

WRITE8_HANDLER( jedi_audio_reset_w );
WRITE8_HANDLER( jedi_audio_latch_w );
READ8_HANDLER( jedi_audio_ack_latch_r );
CUSTOM_INPUT( jedi_audio_comm_stat_r );


/*----------- defined in video/jedi.c -----------*/

MACHINE_DRIVER_EXTERN( jedi_video );

WRITE8_HANDLER( jedi_vscroll_w );
WRITE8_HANDLER( jedi_hscroll_w );
