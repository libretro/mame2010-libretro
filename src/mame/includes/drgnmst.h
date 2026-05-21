
#include "sound/okim6295.h"

class drgnmst_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, drgnmst_state(machine)); }

	drgnmst_state(running_machine &machine)
		: oki_1(machine.device<okim6295_device>("oki1")),
		  oki_2(machine.device<okim6295_device>("oki2")) { }

	/* memory pointers */
	uint16_t *    vidregs;
	uint16_t *    fg_videoram;
	uint16_t *    bg_videoram;
	uint16_t *    md_videoram;
	uint16_t *    rowscrollram;
	uint16_t *    vidregs2;
	uint16_t *    spriteram;
//  uint16_t *    paletteram;     // currently this uses generic palette handling
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *bg_tilemap,*fg_tilemap, *md_tilemap;

	/* misc */
	uint16_t      snd_command;
	uint16_t      snd_flag;
	uint8_t       oki_control;
	uint8_t       oki_command;
	uint8_t       pic16c5x_port0;
	uint8_t       oki0_bank;
	uint8_t       oki1_bank;

	/* devices */
	okim6295_device *oki_1;
	okim6295_device *oki_2;
};


/*----------- defined in video/drgnmst.c -----------*/

WRITE16_HANDLER( drgnmst_fg_videoram_w );
WRITE16_HANDLER( drgnmst_bg_videoram_w );
WRITE16_HANDLER( drgnmst_md_videoram_w );

VIDEO_START(drgnmst);
VIDEO_UPDATE(drgnmst);
