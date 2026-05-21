/***************************************************************************

    Track'n'Field - Hyper Sports - Yie Ar Kung-Fu - Super Basketball
     (these drivers share sound hardware handling)

***************************************************************************/

#include "sound/msm5205.h"


class trackfld_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, trackfld_state(machine)); }

	trackfld_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  videoram;	// trackfld, hyperspt, yiear, sbasketb
	uint8_t *  colorram;	// trackfld, hyperspt, sbasketb
	uint8_t *  scroll;		// trackfld, hyperspt
	uint8_t *  scroll2;		// trackfld
	uint8_t *  spriteram;
	uint8_t *  spriteram2;
//  uint8_t *  nvram;     // currently this uses generic nvram handling (trackfld & hyperspt)
	size_t   spriteram_size;
	uint8_t *  palettebank;		// sbasketb
	uint8_t *  spriteram_select;	// sbasketb

	/* video-related */
	tilemap_t  *bg_tilemap;
	int      bg_bank, sprite_bank1, sprite_bank2;	// trackfld
	int      old_gfx_bank;					// needed by atlantol


	/* sound-related */
	int      SN76496_latch;
	int      last_addr;
	int      last_irq;

	/* game specific */
	uint8_t    hyprolyb_adpcm_ready;	// only bootlegs
	uint8_t    hyprolyb_adpcm_busy;
	uint8_t    hyprolyb_vck_ready;
	int      yiear_nmi_enable;		// yiear

	/* devices */
	cpu_device *audiocpu;
	running_device *vlm;
};


/*----------- defined in audio/trackfld.c -----------*/

WRITE8_HANDLER( konami_sh_irqtrigger_w );
READ8_HANDLER( trackfld_sh_timer_r );
READ8_DEVICE_HANDLER( trackfld_speech_r );
WRITE8_DEVICE_HANDLER( trackfld_sound_w );
READ8_HANDLER( hyperspt_sh_timer_r );
WRITE8_DEVICE_HANDLER( hyperspt_sound_w );
WRITE8_HANDLER( konami_SN76496_latch_w );
WRITE8_DEVICE_HANDLER( konami_SN76496_w );


/*----------- defined in drivers/trackfld.c -----------*/
/*-------------- (needed by hypersptb) ----------------*/

extern const msm5205_interface hyprolyb_msm5205_config;
extern WRITE8_HANDLER( hyprolyb_adpcm_w );
ADDRESS_MAP_EXTERN( hyprolyb_adpcm_map, 8 );


/*----------- defined in video/trackfld.c -----------*/

WRITE8_HANDLER( trackfld_videoram_w );
WRITE8_HANDLER( trackfld_colorram_w );
WRITE8_HANDLER( trackfld_flipscreen_w );
WRITE8_HANDLER( atlantol_gfxbank_w );

PALETTE_INIT( trackfld );
VIDEO_START( trackfld );
VIDEO_UPDATE( trackfld );


/*----------- defined in video/hyperspt.c -----------*/

WRITE8_HANDLER( hyperspt_videoram_w );
WRITE8_HANDLER( hyperspt_colorram_w );
WRITE8_HANDLER( hyperspt_flipscreen_w );

PALETTE_INIT( hyperspt );
VIDEO_START( hyperspt );
VIDEO_UPDATE( hyperspt );
VIDEO_START( roadf );


/*----------- defined in video/sbasketb.c -----------*/

WRITE8_HANDLER( sbasketb_videoram_w );
WRITE8_HANDLER( sbasketb_colorram_w );
WRITE8_HANDLER( sbasketb_flipscreen_w );

PALETTE_INIT( sbasketb );
VIDEO_START( sbasketb );
VIDEO_UPDATE( sbasketb );


/*----------- defined in video/yiear.c -----------*/

WRITE8_HANDLER( yiear_videoram_w );
WRITE8_HANDLER( yiear_control_w );

PALETTE_INIT( yiear );
VIDEO_START( yiear );
VIDEO_UPDATE( yiear );
