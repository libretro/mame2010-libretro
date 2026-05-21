/***************************************************************************

    Game Driver for Video System Mahjong series and Pipe Dream.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -
    and Bryan McPhail, Nicola Salmoria, Aaron Giles

***************************************************************************/

class fromance_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, fromance_state(machine)); }

	fromance_state(running_machine &machine) { }

	/* memory pointers (used by pipedrm) */
	uint8_t *  videoram;
	uint8_t *  spriteram;
//  uint8_t *  paletteram;    // currently this uses generic palette handling
	size_t   videoram_size;
	size_t   spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap,*fg_tilemap;
	uint8_t    *local_videoram[2];
	uint8_t    *local_paletteram;
	uint8_t    selected_videoram, selected_paletteram;
	uint32_t   scrollx[2], scrolly[2];
	uint8_t    gfxreg;
	uint8_t    flipscreen, flipscreen_old;
	uint32_t   scrolly_ofs, scrollx_ofs;

	uint8_t    crtc_register;
	uint8_t    crtc_data[0x10];
	emu_timer *crtc_timer;

	/* misc */
	uint8_t    directionflag, commanddata, portselect;
	uint8_t    adpcm_reset, adpcm_data, vclk_left;
	uint8_t    pending_command, sound_command;

	/* devices */
	running_device *subcpu;
};


/*----------- defined in video/fromance.c -----------*/

VIDEO_START( fromance );
VIDEO_START( nekkyoku );
VIDEO_START( pipedrm );
VIDEO_START( hatris );
VIDEO_UPDATE( fromance );
VIDEO_UPDATE( pipedrm );

WRITE8_HANDLER( fromance_crtc_data_w );
WRITE8_HANDLER( fromance_crtc_register_w );

WRITE8_HANDLER( fromance_gfxreg_w );

WRITE8_HANDLER( fromance_scroll_w );

READ8_HANDLER( fromance_paletteram_r );
WRITE8_HANDLER( fromance_paletteram_w );

READ8_HANDLER( fromance_videoram_r );
WRITE8_HANDLER( fromance_videoram_w );
