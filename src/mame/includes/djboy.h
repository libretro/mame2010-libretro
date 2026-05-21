/*************************************************************************

    DJ Boy

*************************************************************************/

#define PROT_OUTPUT_BUFFER_SIZE 8

class djboy_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, djboy_state(machine)); }

	djboy_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t		*videoram;
	uint8_t		*paletteram;

	/* ROM banking */
	uint8_t		bankxor;
	uint8_t		addr;

	/* video-related */
	tilemap_t	*background;
	uint8_t		videoreg, scrollx, scrolly;

	/* Kaneko BEAST state */
	uint8_t		data_to_beast;
	uint8_t		data_to_z80;
	uint8_t		beast_to_z80_full;
	uint8_t		z80_to_beast_full;
	uint8_t		beast_int0_l;
	uint8_t		beast_p0;
	uint8_t		beast_p1;
	uint8_t		beast_p2;
	uint8_t		beast_p3;

	/* devices */
	running_device *maincpu;
	running_device *cpu1;
	running_device *cpu2;
	running_device *pandora;
	running_device *beast;
};


/*----------- defined in video/djboy.c -----------*/

WRITE8_HANDLER( djboy_scrollx_w );
WRITE8_HANDLER( djboy_scrolly_w );
WRITE8_HANDLER( djboy_videoram_w );
WRITE8_HANDLER( djboy_paletteram_w );

VIDEO_START( djboy );
VIDEO_UPDATE( djboy );
VIDEO_EOF( djboy );
