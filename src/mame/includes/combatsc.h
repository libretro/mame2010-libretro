/*************************************************************************

    Combat School

*************************************************************************/

class combatsc_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, combatsc_state(machine)); }

	combatsc_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    scrollram;
	uint8_t *    io_ram;
	uint8_t *    paletteram;
	uint8_t *    spriteram[2];

	/* video-related */
	tilemap_t *bg_tilemap[2], *textlayer;
	uint8_t scrollram0[0x40];
	uint8_t scrollram1[0x40];
	int priority;

	int  vreg;
	int  bank_select; /* 0x00..0x1f */
	int  video_circuit; /* 0 or 1 */
	uint8_t *page[2];

	/* misc */
	uint8_t pos[4],sign[4];
	int prot[2];
	int boost;
	emu_timer *interleave_timer;


	/* devices */
	cpu_device *audiocpu;
	running_device *k007121_1;
	running_device *k007121_2;
};


/*----------- defined in video/combatsc.c -----------*/

READ8_HANDLER( combatsc_video_r );
WRITE8_HANDLER( combatsc_video_w );

WRITE8_HANDLER( combatsc_pf_control_w );
READ8_HANDLER( combatsc_scrollram_r );
WRITE8_HANDLER( combatsc_scrollram_w );

PALETTE_INIT( combatsc );
PALETTE_INIT( combatscb );
VIDEO_START( combatsc );
VIDEO_START( combatscb );
VIDEO_UPDATE( combatscb );
VIDEO_UPDATE( combatsc );
