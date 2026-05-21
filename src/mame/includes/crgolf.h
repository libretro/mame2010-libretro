/*************************************************************************

    Kitco Crowns Golf hardware

**************************************************************************/

#define MASTER_CLOCK		18432000


class crgolf_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, crgolf_state(machine)); }

	crgolf_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  videoram_a;
	uint8_t *  videoram_b;
	uint8_t *  color_select;
	uint8_t *  screen_flip;
	uint8_t *  screen_select;
	uint8_t *  screena_enable;
	uint8_t *  screenb_enable;

	/* misc */
	uint8_t    port_select;
	uint8_t    main_to_sound_data, sound_to_main_data;
	uint16_t   sample_offset;
	uint8_t    sample_count;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

/*----------- defined in video/crgolf.c -----------*/

WRITE8_HANDLER( crgolf_videoram_w );
READ8_HANDLER( crgolf_videoram_r );

MACHINE_DRIVER_EXTERN( crgolf_video );
