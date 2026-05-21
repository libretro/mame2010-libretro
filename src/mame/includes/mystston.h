/***************************************************************************

    Technos Mysterious Stones hardware

    driver by Nicola Salmoria

***************************************************************************/


#define	MYSTSTON_MASTER_CLOCK	(XTAL_12MHz)


class mystston_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mystston_state(machine)); }

	mystston_state(running_machine &machine) { }

	/* machine state */
	uint8_t *ay8910_data;
	uint8_t *ay8910_select;

	/* video state */
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
	emu_timer *interrupt_timer;
	uint8_t *bg_videoram;
	uint8_t *fg_videoram;
	uint8_t *spriteram;
	uint8_t *paletteram;
	uint8_t *scroll;
	uint8_t *video_control;
};


/*----------- defined in drivers/mystston.c -----------*/

void mystston_on_scanline_interrupt(running_machine *machine);


/*----------- defined in video/mystston.c -----------*/

MACHINE_DRIVER_EXTERN( mystston_video );
WRITE8_HANDLER( mystston_video_control_w );
