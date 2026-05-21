/*************************************************************************

    Yun Sung 16 Bit Games

*************************************************************************/

class yunsun16_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, yunsun16_state(machine)); }

	yunsun16_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *    vram_0;
	uint16_t *    vram_1;
	uint16_t *    scrollram_0;
	uint16_t *    scrollram_1;
	uint16_t *    priorityram;
//  uint16_t *    paletteram; // currently this uses generic palette handling
	uint16_t *    spriteram;
	size_t      spriteram_size;

	/* other video-related elements */
	tilemap_t     *tilemap_0, *tilemap_1;
	int         sprites_scrolldx, sprites_scrolldy;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/yunsun16.c -----------*/

WRITE16_HANDLER( yunsun16_vram_0_w );
WRITE16_HANDLER( yunsun16_vram_1_w );

VIDEO_START( yunsun16 );
VIDEO_UPDATE( yunsun16 );
