/***************************************************************************

    Sun Electronics Arabian hardware

    driver by Dan Boris

***************************************************************************/

class arabian_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, arabian_state(machine)); }

	arabian_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  blitter;
	uint8_t *  custom_cpu_ram;

	uint8_t *  main_bitmap;
	uint8_t *  converted_gfx;

	/* video-related */
	uint8_t    video_control;
	uint8_t    flip_screen;

	/* misc */
	uint8_t    custom_cpu_reset;
	uint8_t    custom_cpu_busy;
};


/*----------- defined in video/arabian.c -----------*/

WRITE8_HANDLER( arabian_blitter_w );
WRITE8_HANDLER( arabian_videoram_w );

PALETTE_INIT( arabian );
VIDEO_START( arabian );
VIDEO_UPDATE( arabian );
