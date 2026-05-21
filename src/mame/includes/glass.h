/*************************************************************************

    Glass

*************************************************************************/

class glass_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, glass_state(machine)); }

	glass_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *    videoram;
	uint16_t *    vregs;
	uint16_t *    spriteram;
//      uint16_t *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t     *pant[2];
	bitmap_t    *screen_bitmap;

	/* misc */
	int         current_bit, current_command, cause_interrupt;
	int         blitter_serial_buffer[5];
};


/*----------- defined in video/glass.c -----------*/

WRITE16_HANDLER( glass_vram_w );
WRITE16_HANDLER( glass_blitter_w );

VIDEO_START( glass );
VIDEO_UPDATE( glass );
