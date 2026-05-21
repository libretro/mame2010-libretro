/*************************************************************************

    Success Joe / Ashita no Joe

*************************************************************************/

class ashnojoe_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ashnojoe_state(machine)); }

	ashnojoe_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *    tileram;
	uint16_t *    tileram_1;
	uint16_t *    tileram_2;
	uint16_t *    tileram_3;
	uint16_t *    tileram_4;
	uint16_t *    tileram_5;
	uint16_t *    tileram_6;
	uint16_t *    tileram_7;
	uint16_t *    tilemap_reg;
//  uint16_t *    paletteram; // currently this uses generic palette handling

	/* video-related */
	tilemap_t     *joetilemap, *joetilemap2, *joetilemap3, *joetilemap4, *joetilemap5, *joetilemap6, *joetilemap7;

	/* sound-related */
	uint8_t       adpcm_byte;
	int         soundlatch_status;
	int         msm5205_vclk_toggle;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/ashnojoe.c -----------*/

WRITE16_HANDLER( ashnojoe_tileram_w );
WRITE16_HANDLER( ashnojoe_tileram2_w );
WRITE16_HANDLER( ashnojoe_tileram3_w );
WRITE16_HANDLER( ashnojoe_tileram4_w );
WRITE16_HANDLER( ashnojoe_tileram5_w );
WRITE16_HANDLER( ashnojoe_tileram6_w );
WRITE16_HANDLER( ashnojoe_tileram7_w );
WRITE16_HANDLER( joe_tilemaps_xscroll_w );
WRITE16_HANDLER( joe_tilemaps_yscroll_w );

VIDEO_START( ashnojoe );
VIDEO_UPDATE( ashnojoe );
