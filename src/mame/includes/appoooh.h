

class appoooh_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, appoooh_state(machine)); }

	appoooh_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  bg_videoram;
	uint8_t *  bg_colorram;
	uint8_t *  fg_videoram;
	uint8_t *  fg_colorram;
	uint8_t *  spriteram;
	uint8_t *  spriteram_2;

	/* video-related */
	tilemap_t  *fg_tilemap, *bg_tilemap;
	int scroll_x;
	int priority;

	/* sound-related */
	uint32_t   adpcm_data;
	uint32_t   adpcm_address;

	/* devices */
	running_device *adpcm;
};

#define CHR1_OFST   0x00  /* palette page of char set #1 */
#define CHR2_OFST   0x10  /* palette page of char set #2 */


/* ----------- defined in video/appoooh.c -----------*/

WRITE8_HANDLER( appoooh_fg_videoram_w );
WRITE8_HANDLER( appoooh_fg_colorram_w );
WRITE8_HANDLER( appoooh_bg_videoram_w );
WRITE8_HANDLER( appoooh_bg_colorram_w );
PALETTE_INIT( appoooh );
PALETTE_INIT( robowres );
WRITE8_HANDLER( appoooh_scroll_w );
WRITE8_HANDLER( appoooh_out_w );
VIDEO_START( appoooh );
VIDEO_UPDATE( appoooh );
VIDEO_UPDATE( robowres );
