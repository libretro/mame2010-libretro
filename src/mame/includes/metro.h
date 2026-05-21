/*************************************************************************

    Metro Games

*************************************************************************/

#include "sound/okim6295.h"
#include "sound/2151intf.h"
#include "video/konicdev.h"

class metro_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, metro_state(machine)); }

	metro_state(running_machine &machine)
		: maincpu(machine.device<cpu_device>("maincpu")),
		  audiocpu(machine.device<cpu_device>("audiocpu")),
		  oki(machine.device<okim6295_device>("oki")),
		  ymsnd(machine.device("ymsnd")),
		  k053936(machine.device<k053936_device>("k053936")) { }

	/* memory pointers */
	uint16_t *    vram_0;
	uint16_t *    vram_1;
	uint16_t *    vram_2;
	uint16_t *    spriteram;
	uint16_t *    tiletable;
	uint16_t *    tiletable_old;
	uint16_t *    blitter_regs;
	uint16_t *    scroll;
	uint16_t *    window;
	uint16_t *    irq_enable;
	uint16_t *    irq_levels;
	uint16_t *    irq_vectors;
	uint16_t *    rombank;
	uint16_t *    videoregs;
	uint16_t *    screenctrl;
	uint16_t *    input_sel;
	uint16_t *    k053936_ram;

	size_t      spriteram_size;
	size_t      tiletable_size;

	int         flip_screen;

	/* video-related */
	tilemap_t   *k053936_tilemap;
	int         bg_tilemap_enable[3];
	int         bg_tilemap_enable16[3];
	int         bg_tilemap_scrolldx[3];

	int         support_8bpp, support_16x16;
	int         has_zoom;
	int         sprite_xoffs, sprite_yoffs;

	/* blitter */
	int         blitter_bit;

	/* irq_related */
	int         irq_line;
	uint8_t       requested_int[8];
	emu_timer   *mouja_irq_timer;

	/* sound related */
	uint16_t      soundstatus;
	int         porta, portb, busy_sndcpu;

	/* misc */
	int         gakusai_oki_bank_lo, gakusai_oki_bank_hi;

	/* used by vmetal.c */
	uint16_t *vmetal_texttileram;
	uint16_t *vmetal_mid1tileram;
	uint16_t *vmetal_mid2tileram;
	uint16_t *vmetal_tlookup;
	uint16_t *vmetal_videoregs;

	tilemap_t *vmetal_texttilemap;
	tilemap_t *vmetal_mid1tilemap;
	tilemap_t *vmetal_mid2tilemap;

	/* devices */
	cpu_device *maincpu;
	cpu_device *audiocpu;
	okim6295_device *oki;
	device_t *ymsnd;
	k053936_device *k053936;
};


/*----------- defined in video/metro.c -----------*/

WRITE16_HANDLER( metro_window_w );
WRITE16_HANDLER( metro_vram_0_w );
WRITE16_HANDLER( metro_vram_1_w );
WRITE16_HANDLER( metro_vram_2_w );
WRITE16_HANDLER( metro_k053936_w );

VIDEO_START( metro_14100 );
VIDEO_START( metro_14220 );
VIDEO_START( metro_14300 );
VIDEO_START( blzntrnd );
VIDEO_START( gstrik2 );

VIDEO_UPDATE( metro );

void metro_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
