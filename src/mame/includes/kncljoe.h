/*************************************************************************

    Knuckle Joe

*************************************************************************/

class kncljoe_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, kncljoe_state(machine)); }

	kncljoe_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    spriteram;
	uint8_t *    scrollregs;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;
	int        tile_bank, sprite_bank;
	int        flipscreen;

	/* misc */
	uint8_t      port1, port2;

	/* devices */
	running_device *soundcpu;
};



/*----------- defined in video/kncljoe.c -----------*/

WRITE8_HANDLER(kncljoe_videoram_w);
WRITE8_HANDLER(kncljoe_control_w);
WRITE8_HANDLER(kncljoe_scroll_w);

PALETTE_INIT( kncljoe );
VIDEO_START( kncljoe );
VIDEO_UPDATE( kncljoe );
