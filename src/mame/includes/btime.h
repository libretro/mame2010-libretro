
class btime_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, btime_state(machine)); }

	btime_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  videoram;
	uint8_t *  colorram;
//  uint8_t *  paletteram;    // currently this uses generic palette handling
	uint8_t *  lnc_charbank;
	uint8_t *  bnj_backgroundram;
	uint8_t *  zoar_scrollram;
	uint8_t *  deco_charram;
	uint8_t *  spriteram;	// used by disco
//  uint8_t *  decrypted;
	uint8_t *  rambase;
	uint8_t *  audio_rambase;
	size_t   videoram_size;
	size_t   spriteram_size;
	size_t   bnj_backgroundram_size;

	/* video-related */
	bitmap_t *background_bitmap;
	uint8_t    btime_palette;
	uint8_t    bnj_scroll1;
	uint8_t    bnj_scroll2;
	uint8_t    btime_tilemap[4];

	/* audio-related */
	uint8_t    audio_nmi_enable_type;
	uint8_t    audio_nmi_enabled;
	uint8_t    audio_nmi_state;

	/* protection-related (for mmonkey) */
	int      protection_command;
	int      protection_status;
	int      protection_value;
	int      protection_ret;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in machine/btime.c -----------*/

READ8_HANDLER( mmonkey_protection_r );
WRITE8_HANDLER( mmonkey_protection_w );


/*----------- defined in video/btime.c -----------*/

PALETTE_INIT( btime );
PALETTE_INIT( lnc );

VIDEO_START( btime );
VIDEO_START( bnj );

VIDEO_UPDATE( btime );
VIDEO_UPDATE( cookrace );
VIDEO_UPDATE( bnj );
VIDEO_UPDATE( lnc );
VIDEO_UPDATE( zoar );
VIDEO_UPDATE( disco );
VIDEO_UPDATE( eggs );

WRITE8_HANDLER( btime_paletteram_w );
WRITE8_HANDLER( bnj_background_w );
WRITE8_HANDLER( bnj_scroll1_w );
WRITE8_HANDLER( bnj_scroll2_w );
READ8_HANDLER( btime_mirrorvideoram_r );
WRITE8_HANDLER( btime_mirrorvideoram_w );
READ8_HANDLER( btime_mirrorcolorram_r );
WRITE8_HANDLER( btime_mirrorcolorram_w );
WRITE8_HANDLER( lnc_videoram_w );
WRITE8_HANDLER( lnc_mirrorvideoram_w );
WRITE8_HANDLER( deco_charram_w );

WRITE8_HANDLER( zoar_video_control_w );
WRITE8_HANDLER( btime_video_control_w );
WRITE8_HANDLER( bnj_video_control_w );
WRITE8_HANDLER( disco_video_control_w );
