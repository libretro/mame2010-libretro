/*************************************************************************

    Karnov - Wonder Planet - Chelnov

*************************************************************************/

class karnov_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, karnov_state(machine)); }

	karnov_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *    videoram;
	uint16_t *    ram;
	uint16_t *    pf_data;
//  uint16_t *    spriteram;  // currently this uses generic buffered spriteram

	/* video-related */
	bitmap_t    *bitmap_f;
	tilemap_t     *fix_tilemap;
	int         flipscreen;
	uint16_t      scroll[2];

	/* misc */
	uint16_t      i8751_return, i8751_needs_ack, i8751_coin_pending, i8751_command_queue;
	int         i8751_level;	// needed by chelnov
	int         microcontroller_id, coin_mask;
	int         latch;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

enum {
	KARNOV = 0,
	KARNOVJ,
	CHELNOV,
	CHELNOVJ,
	CHELNOVW,
	WNDRPLNT
};


/*----------- defined in video/karnov.c -----------*/

WRITE16_HANDLER( karnov_playfield_swap_w );
WRITE16_HANDLER( karnov_videoram_w );

void karnov_flipscreen_w(running_machine *machine, int data);

PALETTE_INIT( karnov );
VIDEO_START( karnov );
VIDEO_START( wndrplnt );
VIDEO_UPDATE( karnov );
