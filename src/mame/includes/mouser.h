/*************************************************************************

    Mouser

*************************************************************************/

class mouser_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mouser_state(machine)); }

	mouser_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    colorram;
	uint8_t *    spriteram;
	size_t     spriteram_size;

	/* misc */
	uint8_t      sound_byte;
	uint8_t      nmi_enable;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

/*----------- defined in video/mouser.c -----------*/

WRITE8_HANDLER( mouser_flip_screen_x_w );
WRITE8_HANDLER( mouser_flip_screen_y_w );

PALETTE_INIT( mouser );
VIDEO_UPDATE( mouser );
