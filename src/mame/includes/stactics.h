/****************************************************************************

    Sega "Space Tactics" Driver

    Frank Palazzolo (palazzol@home.com)

****************************************************************************/


class stactics_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, stactics_state(machine)); }

	stactics_state(running_machine &machine) { }

	/* machine state */
	int    vert_pos;
	int    horiz_pos;
	uint8_t *motor_on;

	/* video state */
	uint8_t *videoram_b;
	uint8_t *videoram_d;
	uint8_t *videoram_e;
	uint8_t *videoram_f;
	uint8_t *palette;
	uint8_t *display_buffer;
	uint8_t *lamps;

	uint8_t  y_scroll_d;
	uint8_t  y_scroll_e;
	uint8_t  y_scroll_f;
	uint8_t  frame_count;
	uint8_t  shot_standby;
	uint8_t  shot_arrive;
	uint16_t beam_state;
	uint16_t old_beam_state;
	uint16_t beam_states_per_frame;
};


/*----------- defined in video/stactics.c -----------*/

MACHINE_DRIVER_EXTERN( stactics_video );

WRITE8_HANDLER( stactics_scroll_ram_w );
WRITE8_HANDLER( stactics_speed_latch_w );
WRITE8_HANDLER( stactics_shot_trigger_w );
WRITE8_HANDLER( stactics_shot_flag_clear_w );
CUSTOM_INPUT( stactics_get_frame_count_d3 );
CUSTOM_INPUT( stactics_get_shot_standby );
CUSTOM_INPUT( stactics_get_not_shot_arrive );

