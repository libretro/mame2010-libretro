#define RASTER_LINES 262
#define FIRST_VISIBLE_LINE 0
#define LAST_VISIBLE_LINE 223

class hyprduel_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, hyprduel_state(machine)); }

	hyprduel_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *  videoregs;
	uint16_t *  screenctrl;
	uint16_t *  tiletable_old;
	uint16_t *  tiletable;
	uint16_t *  vram_0;
	uint16_t *  vram_1;
	uint16_t *  vram_2;
	uint16_t *  window;
	uint16_t *  scroll;
	uint16_t *  rombank;
	uint16_t *  blitter_regs;
	uint16_t *  irq_enable;
	uint16_t *  sharedram1;
	uint16_t *  sharedram3;
	uint16_t *  spriteram;
	uint16_t *  paletteram;
	size_t    tiletable_size;
	size_t    spriteram_size;

	/* video-related */
	tilemap_t   *bg_tilemap[3];
	uint8_t     *empty_tiles;
	uint8_t     *dirtyindex;
	int       sprite_xoffs, sprite_yoffs, sprite_yoffs_sub;

	/* misc */
	emu_timer *magerror_irq_timer;
	int       blitter_bit;
	int       requested_int;
	int       subcpu_resetline;
	int       cpu_trigger;
	int       int_num;

	/* devices */
	running_device *maincpu;
	running_device *subcpu;
};



/*----------- defined in video/hyprduel.c -----------*/


WRITE16_HANDLER( hyprduel_paletteram_w );
WRITE16_HANDLER( hyprduel_window_w );
WRITE16_HANDLER( hyprduel_vram_0_w );
WRITE16_HANDLER( hyprduel_vram_1_w );
WRITE16_HANDLER( hyprduel_vram_2_w );
WRITE16_HANDLER( hyprduel_scrollreg_w );
WRITE16_HANDLER( hyprduel_scrollreg_init_w );

VIDEO_START( hyprduel_14220 );
VIDEO_START( magerror_14220 );
VIDEO_UPDATE( hyprduel );
