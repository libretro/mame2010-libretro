/***************************************************************************

    Double Dribble

***************************************************************************/

class ddribble_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ddribble_state(machine)); }

	ddribble_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *     sharedram;
	uint8_t *     snd_sharedram;
	uint8_t *     spriteram_1;
	uint8_t *     spriteram_2;
	uint8_t *     bg_videoram;
	uint8_t *     fg_videoram;
	uint8_t *     paletteram;

	/* video-related */
	tilemap_t     *fg_tilemap,*bg_tilemap;
	int         vregs[2][5];
	int         charbank[2];

	/* misc */
	int         int_enable_0, int_enable_1;

	/* devices */
	running_device *filter1;
	running_device *filter2;
	running_device *filter3;
};

/*----------- defined in video/ddribble.c -----------*/

WRITE8_HANDLER( ddribble_fg_videoram_w );
WRITE8_HANDLER( ddribble_bg_videoram_w );
WRITE8_HANDLER( K005885_0_w );
WRITE8_HANDLER( K005885_1_w );

PALETTE_INIT( ddribble );
VIDEO_START( ddribble );
VIDEO_UPDATE( ddribble );
