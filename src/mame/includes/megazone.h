/*************************************************************************

    Megazone

*************************************************************************/

class megazone_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, megazone_state(machine)); }

	megazone_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *       scrollx;
	uint8_t *       scrolly;
	uint8_t *       videoram;
	uint8_t *       colorram;
	uint8_t *       videoram2;
	uint8_t *       colorram2;
	uint8_t *       spriteram;
	size_t        spriteram_size;
	size_t        videoram_size;
	size_t        videoram2_size;

	/* video-related */
	bitmap_t      *tmpbitmap;
	int           flipscreen;

	/* misc */
	int           i8039_status;

	/* devices */
	cpu_device *maincpu;
	cpu_device *audiocpu;
	cpu_device *daccpu;
};



/*----------- defined in video/megazone.c -----------*/

WRITE8_HANDLER( megazone_flipscreen_w );

PALETTE_INIT( megazone );
VIDEO_START( megazone );
VIDEO_UPDATE( megazone );
