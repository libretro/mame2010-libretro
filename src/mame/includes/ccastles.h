/*************************************************************************

    Atari Crystal Castles hardware

*************************************************************************/

class ccastles_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ccastles_state(machine)); }

	ccastles_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  videoram;
	uint8_t *  spriteram;
//  uint8_t *  nvram_stage;   // currently this uses generic nvram handlers
//  uint8_t *  nvram;     // currently this uses generic nvram handlers

	/* video-related */
	const uint8_t *syncprom;
	const uint8_t *wpprom;
	const uint8_t *priprom;
	bitmap_t *spritebitmap;
	double rweights[3], gweights[3], bweights[3];
	uint8_t video_control[8];
	uint8_t bitmode_addr[2];
	uint8_t hscroll;
	uint8_t vscroll;

	/* misc */
	int      vblank_start;
	int      vblank_end;
	emu_timer *irq_timer;
	uint8_t    irq_state;
	uint8_t    nvram_store[2];

	/* devices */
	running_device *maincpu;
};


/*----------- defined in video/ccastles.c -----------*/


VIDEO_START( ccastles );
VIDEO_UPDATE( ccastles );

WRITE8_HANDLER( ccastles_hscroll_w );
WRITE8_HANDLER( ccastles_vscroll_w );
WRITE8_HANDLER( ccastles_video_control_w );

WRITE8_HANDLER( ccastles_paletteram_w );
WRITE8_HANDLER( ccastles_videoram_w );

READ8_HANDLER( ccastles_bitmode_r );
WRITE8_HANDLER( ccastles_bitmode_w );
WRITE8_HANDLER( ccastles_bitmode_addr_w );
