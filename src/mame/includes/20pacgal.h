/***************************************************************************

    Ms.Pac-Man/Galaga - 20 Year Reunion hardware

    driver by Nicola Salmoria

***************************************************************************/


class _20pacgal_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, _20pacgal_state(machine)); }

	_20pacgal_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *char_gfx_ram;
	uint8_t *sprite_gfx_ram;
	uint8_t *video_ram;
	uint8_t *sprite_ram;
	uint8_t *sprite_color_lookup;
	uint8_t *flip;
	uint8_t *stars_seed;
	uint8_t *stars_ctrl;

	/* machine state */
	uint8_t game_selected;	/* 0 = Ms. Pac-Man, 1 = Galaga */

	/* devices */
	running_device *maincpu;
	running_device *eeprom;

	/* bank support */
	uint8_t *ram_48000;
};



/*----------- defined in video/20pacgal.c -----------*/

MACHINE_DRIVER_EXTERN( 20pacgal_video );
