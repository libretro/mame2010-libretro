/***************************************************************************

    Galaxian hardware family

***************************************************************************/

#include "sound/discrete.h"
/* we scale horizontally by 3 to render stars correctly */
#define GALAXIAN_XSCALE			3

/* master clocks */
#define GALAXIAN_MASTER_CLOCK	(18432000)
#define GALAXIAN_PIXEL_CLOCK	(GALAXIAN_XSCALE*GALAXIAN_MASTER_CLOCK/3)

/* H counts from 128->511, HBLANK starts at 130 and ends at 250 */
/* we normalize this here so that we count 0->383 with HBLANK */
/* from 264-383 */
#define GALAXIAN_HTOTAL			(384*GALAXIAN_XSCALE)
#define GALAXIAN_HBEND			(0*GALAXIAN_XSCALE)
//#define GALAXIAN_H0START      (6*GALAXIAN_XSCALE)
//#define GALAXIAN_HBSTART      (264*GALAXIAN_XSCALE)
#define GALAXIAN_H0START		(0*GALAXIAN_XSCALE)
#define GALAXIAN_HBSTART		(256*GALAXIAN_XSCALE)

#define GALAXIAN_VTOTAL			(264)
#define GALAXIAN_VBEND			(16)
#define GALAXIAN_VBSTART		(224+16)

#define GAL_AUDIO	"discrete"

/*----------- defined in video/galaxian.c -----------*/

extern uint8_t galaxian_frogger_adjust;
extern uint8_t galaxian_sfx_tilemap;
extern uint8_t galaxian_sprite_clip_start;
extern uint8_t galaxian_sprite_clip_end;

PALETTE_INIT( galaxian );
PALETTE_INIT( moonwar );

VIDEO_START( galaxian );
VIDEO_UPDATE( galaxian );

WRITE8_HANDLER( galaxian_videoram_w );
WRITE8_HANDLER( galaxian_objram_w );

WRITE8_HANDLER( galaxian_flip_screen_x_w );
WRITE8_HANDLER( galaxian_flip_screen_y_w );
WRITE8_HANDLER( galaxian_flip_screen_xy_w );

WRITE8_HANDLER( galaxian_stars_enable_w );
WRITE8_HANDLER( scramble_background_enable_w );
WRITE8_HANDLER( scramble_background_red_w );
WRITE8_HANDLER( scramble_background_green_w );
WRITE8_HANDLER( scramble_background_blue_w );

WRITE8_HANDLER( galaxian_gfxbank_w );

TIMER_CALLBACK( galaxian_stars_blink_timer );

/* video extension callbacks */
typedef void (*galaxian_extend_tile_info_func)(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
typedef void (*galaxian_extend_sprite_info_func)(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);
typedef void (*galaxian_draw_bullet_func)(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y);
typedef void (*galaxian_draw_background_func)(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);

extern galaxian_extend_tile_info_func galaxian_extend_tile_info_ptr;
extern galaxian_extend_sprite_info_func galaxian_extend_sprite_info_ptr;
extern galaxian_draw_bullet_func galaxian_draw_bullet_ptr;
extern galaxian_draw_background_func galaxian_draw_background_ptr;

/* special purpose background rendering */
void galaxian_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
void frogger_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
//void amidar_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
void turtles_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
void scramble_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
void jumpbug_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);

/* special purpose bullet rendering */
void galaxian_draw_bullet(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y);
void mshuttle_draw_bullet(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y);
void scramble_draw_bullet(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y);
void theend_draw_bullet(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y);

/* generic extensions */
void upper_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
void upper_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);

/* Frogger extensions */
void frogger_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
void frogger_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);

/* Ghostmuncher Galaxian extensions */
void gmgalax_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
void gmgalax_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);

/* Pisces extensions */
void pisces_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
void pisces_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);

/* Batman Part 2 extensions */
void batman2_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);

/* Moon Cresta extensions */
void mooncrst_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
void mooncrst_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);

/* Moon Quasar extensions */
void moonqsr_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
void moonqsr_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);

/* Moon Shuttle extensions */
void mshuttle_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
void mshuttle_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);

/* Calipso extensions */
void calipso_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);

/* Jumpbug extensions */
void jumpbug_extend_tile_info(uint16_t *code, uint8_t *color, uint8_t attrib, uint8_t x);
void jumpbug_extend_sprite_info(const uint8_t *base, uint8_t *sx, uint8_t *sy, uint8_t *flipx, uint8_t *flipy, uint16_t *code, uint8_t *color);

/*----------- defined in drivers/galaxian.c -----------*/

/* Ten Spot extensions */
void tenspot_set_game_bank(running_machine *machine, int bank, int from_game);

/*----------- defined in audio/galaxian.c -----------*/

MACHINE_DRIVER_EXTERN( mooncrst_audio );
MACHINE_DRIVER_EXTERN( galaxian_audio );

WRITE8_DEVICE_HANDLER( galaxian_sound_w );
WRITE8_DEVICE_HANDLER( galaxian_pitch_w );
WRITE8_DEVICE_HANDLER( galaxian_vol_w );
WRITE8_DEVICE_HANDLER( galaxian_noise_enable_w );
WRITE8_DEVICE_HANDLER( galaxian_background_enable_w );
WRITE8_DEVICE_HANDLER( galaxian_shoot_enable_w );
WRITE8_DEVICE_HANDLER( galaxian_lfo_freq_w );

