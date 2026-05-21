
// later, this might be merged with segas1x_state in segas16.h

class segas1x_bootleg_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, segas1x_bootleg_state(machine)); }

	segas1x_bootleg_state(running_machine &machine) { }

	uint16_t *    bg0_tileram;
	uint16_t *    bg1_tileram;
	uint16_t *    textram;
	uint16_t *    tileram;

	uint16_t coinctrl;

	/* game specific */
	int passht4b_io1_val;
	int passht4b_io2_val;
	int passht4b_io3_val;

	int beautyb_unkx;

	int shinobl_kludge;

	uint16_t* goldnaxeb2_fgpage;
	uint16_t* goldnaxeb2_bgpage;

	int eswat_tilebank0;


	/* video-related */
	tilemap_t *background, *foreground, *text_layer;
	tilemap_t *background2, *foreground2;
	tilemap_t *bg_tilemaps[2];
	tilemap_t *text_tilemap;
	double weights[2][3][6];

	int spritebank_type;
	int back_yscroll;
	int fore_yscroll;
	int text_yscroll;

	int bg1_trans; // alien syn + sys18

	int tile_bank1;
	int tile_bank0;
	int bg_page[4];
	int fg_page[4];

	uint16_t datsu_page[4];

	int bg2_page[4];
	int fg2_page[4];

	int old_bg_page[4],old_fg_page[4], old_tile_bank1, old_tile_bank0;
	int old_bg2_page[4], old_fg2_page[4];

	int bg_scrollx, bg_scrolly;
	int fg_scrollx, fg_scrolly;
	uint16_t tilemapselect;

	int textlayer_lo_min;
	int textlayer_lo_max;
	int textlayer_hi_min;
	int textlayer_hi_max;

	int tilebank_switch;


	/* sound-related */
	int sample_buffer;
	int sample_select;

	uint8_t *soundbank_ptr;		/* Pointer to currently selected portion of ROM */

	/* sys18 */
	uint8_t *sound_bank;
	uint16_t *splittab_bg_x;
	uint16_t *splittab_bg_y;
	uint16_t *splittab_fg_x;
	uint16_t *splittab_fg_y;
	int     sound_info[4*2];
	int     refreshenable;
	int     system18;

	uint8_t *decrypted_region;	// goldnaxeb1 & bayrouteb1

	/* devices */
	running_device *maincpu;
	running_device *soundcpu;
};

/*----------- defined in video/system16.c -----------*/

extern VIDEO_START( s16a_bootleg );
extern VIDEO_START( s16a_bootleg_wb3bl );
extern VIDEO_START( s16a_bootleg_shinobi );
extern VIDEO_START( s16a_bootleg_passsht );
extern VIDEO_UPDATE( s16a_bootleg );
extern VIDEO_UPDATE( s16a_bootleg_passht4b );
extern WRITE16_HANDLER( s16a_bootleg_tilemapselect_w );
extern WRITE16_HANDLER( s16a_bootleg_bgscrolly_w );
extern WRITE16_HANDLER( s16a_bootleg_bgscrollx_w );
extern WRITE16_HANDLER( s16a_bootleg_fgscrolly_w );
extern WRITE16_HANDLER( s16a_bootleg_fgscrollx_w );

/* video hardware */
extern WRITE16_HANDLER( sys16_tileram_w );
extern WRITE16_HANDLER( sys16_textram_w );

/* "normal" video hardware */
extern VIDEO_START( system16 );
extern VIDEO_UPDATE( system16 );

/* system18 video hardware */
extern VIDEO_START( system18old );
extern VIDEO_UPDATE( system18old );
