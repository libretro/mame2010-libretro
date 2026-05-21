/*************************************************************************

    Atari Cloud 9 (prototype) hardware

*************************************************************************/

class cloud9_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cloud9_state(machine)); }

	cloud9_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *     videoram;
	uint8_t *     spriteram;
	uint8_t *     paletteram;
//  uint8_t *     nvram_stage;    // currently this uses generic nvram handlers
//  uint8_t *     nvram;      // currently this uses generic nvram handlers

	/* video-related */
	const uint8_t *syncprom;
	const uint8_t *wpprom;
	const uint8_t *priprom;
	bitmap_t    *spritebitmap;
	double      rweights[3], gweights[3], bweights[3];
	uint8_t       video_control[8];
	uint8_t       bitmode_addr[2];

	/* misc */
	int         vblank_start;
	int         vblank_end;
	emu_timer   *irq_timer;
	uint8_t       irq_state;

	/* devices */
	running_device *maincpu;
};


/*----------- defined in video/cloud9.c -----------*/

VIDEO_START( cloud9 );
VIDEO_UPDATE( cloud9 );

WRITE8_HANDLER( cloud9_video_control_w );

WRITE8_HANDLER( cloud9_paletteram_w );
WRITE8_HANDLER( cloud9_videoram_w );

READ8_HANDLER( cloud9_bitmode_r );
WRITE8_HANDLER( cloud9_bitmode_w );
WRITE8_HANDLER( cloud9_bitmode_addr_w );
