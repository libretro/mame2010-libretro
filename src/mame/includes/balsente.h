/***************************************************************************

    Bally/Sente SAC-1 system

    driver by Aaron Giles

***************************************************************************/

#include "sound/cem3394.h"

#define BALSENTE_MASTER_CLOCK	(20000000)
#define BALSENTE_CPU_CLOCK		(BALSENTE_MASTER_CLOCK / 16)
#define BALSENTE_PIXEL_CLOCK	(BALSENTE_MASTER_CLOCK / 4)
#define BALSENTE_HTOTAL			(0x140)
#define BALSENTE_HBEND			(0x000)
#define BALSENTE_HBSTART		(0x100)
#define BALSENTE_VTOTAL			(0x108)
#define BALSENTE_VBEND			(0x010)
#define BALSENTE_VBSTART		(0x100)


#define POLY17_BITS 17
#define POLY17_SIZE ((1 << POLY17_BITS) - 1)
#define POLY17_SHL	7
#define POLY17_SHR	10
#define POLY17_ADD	0x18000


class balsente_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, balsente_state(machine)); }

	balsente_state(running_machine &machine)
		: scanline_timer(machine.device<timer_device>("scan_timer")),
		  counter_0_timer(machine.device<timer_device>("8253_0_timer"))
	{
		astring temp;
		for (int i = 0; i < ARRAY_LENGTH(cem_device); i++)
		{
			cem_device[i] = machine.device<cem3394_sound_device>(temp.format("cem%d", i+1));
			assert(cem_device[i] != NULL);
		}
	}

	/* global data */
	uint8_t shooter;
	uint8_t shooter_x;
	uint8_t shooter_y;
	uint8_t adc_shift;
	uint16_t *shrike_shared;
	uint16_t *shrike_io;

	/* 8253 counter state */
	struct
	{
		timer_device *timer;
		uint8_t timer_active;
		int32_t initial;
		int32_t count;
		uint8_t gate;
		uint8_t out;
		uint8_t mode;
		uint8_t readbyte;
		uint8_t writebyte;
	} counter[3];

	timer_device *scanline_timer;

	/* manually clocked counter 0 states */
	uint8_t counter_control;
	uint8_t counter_0_ff;
	timer_device *counter_0_timer;
	uint8_t counter_0_timer_active;

	/* random number generator states */
	uint8_t poly17[POLY17_SIZE + 1];
	uint8_t rand17[POLY17_SIZE + 1];

	/* ADC I/O states */
	int8_t analog_input_data[4];
	uint8_t adc_value;

	/* CEM3394 DAC control states */
	uint16_t dac_value;
	uint8_t dac_register;
	uint8_t chip_select;

	/* main CPU 6850 states */
	uint8_t m6850_status;
	uint8_t m6850_control;
	uint8_t m6850_input;
	uint8_t m6850_output;
	uint8_t m6850_data_ready;

	/* sound CPU 6850 states */
	uint8_t m6850_sound_status;
	uint8_t m6850_sound_control;
	uint8_t m6850_sound_input;
	uint8_t m6850_sound_output;

	/* noise generator states */
	uint32_t noise_position[6];
	cem3394_sound_device *cem_device[6];

	/* game-specific states */
	uint8_t nstocker_bits;
	uint8_t spiker_expand_color;
	uint8_t spiker_expand_bgcolor;
	uint8_t spiker_expand_bits;
	uint8_t grudge_steering_result;
	uint8_t grudge_last_steering[3];

	/* video data */
	uint8_t videoram[256 * 256];
	uint8_t *sprite_data;
	uint32_t sprite_mask;
	uint8_t *sprite_bank[2];

	uint8_t palettebank_vis;
};


/*----------- defined in machine/balsente.c -----------*/

TIMER_DEVICE_CALLBACK( balsente_interrupt_timer );

MACHINE_START( balsente );
MACHINE_RESET( balsente );

void balsente_noise_gen(running_device *device, int count, short *buffer);

WRITE8_HANDLER( balsente_random_reset_w );
READ8_HANDLER( balsente_random_num_r );

WRITE8_HANDLER( balsente_rombank_select_w );
WRITE8_HANDLER( balsente_rombank2_select_w );

WRITE8_HANDLER( balsente_misc_output_w );

READ8_HANDLER( balsente_m6850_r );
WRITE8_HANDLER( balsente_m6850_w );

READ8_HANDLER( balsente_m6850_sound_r );
WRITE8_HANDLER( balsente_m6850_sound_w );

INTERRUPT_GEN( balsente_update_analog_inputs );
READ8_HANDLER( balsente_adc_data_r );
WRITE8_HANDLER( balsente_adc_select_w );

TIMER_DEVICE_CALLBACK( balsente_counter_callback );

READ8_HANDLER( balsente_counter_8253_r );
WRITE8_HANDLER( balsente_counter_8253_w );

TIMER_DEVICE_CALLBACK( balsente_clock_counter_0_ff );

READ8_HANDLER( balsente_counter_state_r );
WRITE8_HANDLER( balsente_counter_control_w );

WRITE8_HANDLER( balsente_chip_select_w );
WRITE8_HANDLER( balsente_dac_data_w );
WRITE8_HANDLER( balsente_register_addr_w );

CUSTOM_INPUT( nstocker_bits_r );
WRITE8_HANDLER( spiker_expand_w );
READ8_HANDLER( spiker_expand_r );
READ8_HANDLER( grudge_steering_r );

READ8_HANDLER( shrike_shared_6809_r );
WRITE8_HANDLER( shrike_shared_6809_w );

READ16_HANDLER( shrike_io_68k_r );
WRITE16_HANDLER( shrike_io_68k_w );


/*----------- defined in video/balsente.c -----------*/

VIDEO_START( balsente );
VIDEO_UPDATE( balsente );

WRITE8_HANDLER( balsente_videoram_w );
WRITE8_HANDLER( balsente_paletteram_w );
WRITE8_HANDLER( balsente_palette_select_w );
WRITE8_HANDLER( shrike_sprite_select_w );
