
class playmark_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, playmark_state(machine)); }

	playmark_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *     bgvideoram;
	uint16_t *     videoram1;
	uint16_t *     videoram2;
	uint16_t *     videoram3;
	uint16_t *     rowscroll;
	uint16_t *     spriteram;
//      uint16_t *     paletteram;    // currently this uses generic palette handling
	size_t       spriteram_size;

	/* video-related */
	tilemap_t   *tx_tilemap, *fg_tilemap, *bg_tilemap;
	int         bgscrollx, bgscrolly, bg_enable, bg_full_size;
	int         fgscrollx, fg_rowscroll_enable;

	int         xoffset;
	int         yoffset;
	int         txt_tile_offset;
	int         pri_masks[3];
	uint16_t      scroll[7];

	/* powerbal-specific */
	int         tilebank;
	int         bg_yoffset;

	/* misc */
	uint16_t      snd_command;
	uint16_t      snd_flag;
	uint8_t       oki_control;
	uint8_t       oki_command;
	int         old_oki_bank;

	/* devices */
	running_device *oki;
	running_device *eeprom;
};

/*----------- defined in video/playmark.c -----------*/

WRITE16_HANDLER( wbeachvl_txvideoram_w );
WRITE16_HANDLER( wbeachvl_fgvideoram_w );
WRITE16_HANDLER( wbeachvl_bgvideoram_w );
WRITE16_HANDLER( hrdtimes_txvideoram_w );
WRITE16_HANDLER( hrdtimes_fgvideoram_w );
WRITE16_HANDLER( hrdtimes_bgvideoram_w );
WRITE16_HANDLER( bigtwin_paletteram_w );
WRITE16_HANDLER( bigtwin_scroll_w );
WRITE16_HANDLER( wbeachvl_scroll_w );
WRITE16_HANDLER( excelsr_scroll_w );
WRITE16_HANDLER( hrdtimes_scroll_w );

VIDEO_START( bigtwin );
VIDEO_START( wbeachvl );
VIDEO_START( excelsr );
VIDEO_START( hotmind );
VIDEO_START( hrdtimes );

VIDEO_UPDATE( bigtwin );
VIDEO_UPDATE( wbeachvl );
VIDEO_UPDATE( excelsr );
VIDEO_UPDATE( hrdtimes );
