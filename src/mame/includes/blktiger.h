/***************************************************************************

    Black Tiger

***************************************************************************/

class blktiger_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, blktiger_state(machine)); }

	blktiger_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t * txvideoram;
//  uint8_t * spriteram;  // currently this uses generic buffer_spriteram_w
//  uint8_t * paletteram; // currently this uses generic palette handling
//  uint8_t * paletteram2;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t *tx_tilemap, *bg_tilemap8x4, *bg_tilemap4x8;
	uint32_t  scroll_bank;
	uint8_t   scroll_x[2];
	uint8_t   scroll_y[2];
	uint8_t   *scroll_ram;
	uint8_t   screen_layout;
	uint8_t   chon, objon, bgon;

	/* mcu-related */
	uint8_t   z80_latch, i8751_latch;

	/* devices */
	running_device *mcu;
	running_device *audiocpu;
};


/*----------- defined in video/blktiger.c -----------*/

WRITE8_HANDLER( blktiger_screen_layout_w );

READ8_HANDLER( blktiger_bgvideoram_r );
WRITE8_HANDLER( blktiger_bgvideoram_w );
WRITE8_HANDLER( blktiger_txvideoram_w );
WRITE8_HANDLER( blktiger_video_control_w );
WRITE8_HANDLER( blktiger_video_enable_w );
WRITE8_HANDLER( blktiger_bgvideoram_bank_w );
WRITE8_HANDLER( blktiger_scrollx_w );
WRITE8_HANDLER( blktiger_scrolly_w );

VIDEO_START( blktiger );
VIDEO_UPDATE( blktiger );
VIDEO_EOF( blktiger );
