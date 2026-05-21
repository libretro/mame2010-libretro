/*************************************************************************

    Cheeky Mouse

*************************************************************************/


class cheekyms_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cheekyms_state(machine)); }

	cheekyms_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *        videoram;
	uint8_t *        spriteram;
	uint8_t *        port_80;

	/* video-related */
	tilemap_t        *cm_tilemap;
	bitmap_t       *bitmap_buffer;

	/* devices */
	running_device *maincpu;
	running_device *dac;
};


/*----------- defined in video/cheekyms.c -----------*/

PALETTE_INIT( cheekyms );
VIDEO_START( cheekyms );
VIDEO_UPDATE( cheekyms );
WRITE8_HANDLER( cheekyms_port_40_w );
WRITE8_HANDLER( cheekyms_port_80_w );
