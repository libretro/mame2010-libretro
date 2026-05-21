/*************************************************************************

    King of Boxer - Ring King

*************************************************************************/

class kingofb_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, kingofb_state(machine)); }

	kingofb_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    videoram2;
	uint8_t *    colorram;
	uint8_t *    colorram2;
	uint8_t *    spriteram;
	uint8_t *    scroll_y;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap, *fg_tilemap;
	int        palette_bank;

	/* misc */
	int        nmi_enable;

	/* devices */
	running_device *video_cpu;
	running_device *sprite_cpu;
	running_device *audio_cpu;
};


/*----------- defined in video/kingobox.c -----------*/

WRITE8_HANDLER( kingofb_videoram_w );
WRITE8_HANDLER( kingofb_colorram_w );
WRITE8_HANDLER( kingofb_videoram2_w );
WRITE8_HANDLER( kingofb_colorram2_w );
WRITE8_HANDLER( kingofb_f800_w );

PALETTE_INIT( kingofb );
VIDEO_START( kingofb );
VIDEO_UPDATE( kingofb );

PALETTE_INIT( ringking );
VIDEO_START( ringking );
VIDEO_UPDATE( ringking );
