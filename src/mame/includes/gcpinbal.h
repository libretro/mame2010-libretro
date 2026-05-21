
#include "sound/okim6295.h"
#include "sound/msm5205.h"

class gcpinbal_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gcpinbal_state(machine)); }

	gcpinbal_state(running_machine &machine)
		: maincpu(machine.device<cpu_device>("maincpu")),
		  oki(machine.device<okim6295_device>("oki")),
		  msm(machine.device<msm5205_sound_device>("msm")) { }

	/* memory pointers */
	uint16_t *    tilemapram;
	uint16_t *    ioc_ram;
	uint16_t *    spriteram;
//  uint16_t *    paletteram; // currently this uses generic palette handling
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *tilemap[3];
	uint16_t      scrollx[3], scrolly[3];
	uint16_t      bg0_gfxset, bg1_gfxset;
#ifdef MAME_DEBUG
	uint8_t       dislayer[4];
#endif

	/* sound-related */
	uint32_t      msm_start, msm_end, msm_bank;
	uint32_t      adpcm_start, adpcm_end, adpcm_idle;
	uint8_t       adpcm_trigger, adpcm_data;

	/* devices */
	cpu_device *maincpu;
	okim6295_device *oki;
	msm5205_sound_device *msm;
};


/*----------- defined in video/gcpinbal.c -----------*/

VIDEO_START( gcpinbal );
VIDEO_UPDATE( gcpinbal );

READ16_HANDLER ( gcpinbal_tilemaps_word_r );
WRITE16_HANDLER( gcpinbal_tilemaps_word_w );
