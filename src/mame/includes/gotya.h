
class gotya_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gotya_state(machine)); }

	gotya_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  videoram;
	uint8_t *  videoram2;
	uint8_t *  colorram;
	uint8_t *  spriteram;
	uint8_t *  scroll;

	/* video-related */
	tilemap_t  *bg_tilemap;
	int      scroll_bit_8;

	/* sound-related */
	int      theme_playing;

	/* devices */
	running_device *samples;
};


/*----------- defined in audio/gotya.c -----------*/

WRITE8_HANDLER( gotya_soundlatch_w );


/*----------- defined in video/gotya.c -----------*/

WRITE8_HANDLER( gotya_videoram_w );
WRITE8_HANDLER( gotya_colorram_w );
WRITE8_HANDLER( gotya_video_control_w );

PALETTE_INIT( gotya );
VIDEO_START( gotya );
VIDEO_UPDATE( gotya );
