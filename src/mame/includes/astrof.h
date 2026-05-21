/***************************************************************************

    Astro Fighter hardware

****************************************************************************/

class astrof_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, astrof_state(machine)); }

	astrof_state(running_machine &machine) { }

	/* video-related */
	uint8_t *    videoram;
	size_t     videoram_size;

	uint8_t *    colorram;
	uint8_t *    tomahawk_protection;

	uint8_t *    astrof_color;
	uint8_t      astrof_palette_bank;
	uint8_t      red_on;
	uint8_t      flipscreen;
	uint8_t      screen_off;
	uint16_t     abattle_count;

	/* sound-related */
	uint8_t      port_1_last;
	uint8_t      port_2_last;
	uint8_t      astrof_start_explosion;
	uint8_t      astrof_death_playing;
	uint8_t      astrof_bosskill_playing;

	/* devices */
	running_device *maincpu;
	running_device *samples;	// astrof & abattle
	running_device *sn;	// tomahawk
};

/*----------- defined in audio/astrof.c -----------*/

MACHINE_DRIVER_EXTERN( astrof_audio );
WRITE8_HANDLER( astrof_audio_1_w );
WRITE8_HANDLER( astrof_audio_2_w );

MACHINE_DRIVER_EXTERN( spfghmk2_audio );
WRITE8_HANDLER( spfghmk2_audio_w );

MACHINE_DRIVER_EXTERN( tomahawk_audio );
WRITE8_HANDLER( tomahawk_audio_w );
