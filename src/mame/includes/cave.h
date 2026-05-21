/***************************************************************************

    Cave hardware

***************************************************************************/

struct sprite_cave
{
	int priority, flags;

	const uint8_t *pen_data;	/* points to top left corner of tile data */
	int line_offset;

	pen_t base_pen;
	int tile_width, tile_height;
	int total_width, total_height;	/* in screen coordinates */
	int x, y, xcount0, ycount0;
	int zoomx_re, zoomy_re;
};

#define MAX_PRIORITY        4
#define MAX_SPRITE_NUM      0x400

class cave_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cave_state(machine)); }

	cave_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *     videoregs;
	uint16_t *     vram_0;
	uint16_t *     vram_1;
	uint16_t *     vram_2;
	uint16_t *     vram_3;
	uint16_t *     vctrl_0;
	uint16_t *     vctrl_1;
	uint16_t *     vctrl_2;
	uint16_t *     vctrl_3;
	uint16_t *     spriteram;
	uint16_t *     spriteram_2;
	uint16_t *     paletteram;
	size_t       spriteram_size;
	size_t       paletteram_size;

	/* video-related */
	struct sprite_cave *sprite;
	struct sprite_cave *sprite_table[MAX_PRIORITY][MAX_SPRITE_NUM + 1];

	struct
	{
		int    clip_left, clip_right, clip_top, clip_bottom;
		uint8_t  *baseaddr;
		int    line_offset;
		uint8_t  *baseaddr_zbuf;
		int    line_offset_zbuf;
	} blit;


	void (*get_sprite_info)(running_machine *machine);
	void (*sprite_draw)(running_machine *machine, int priority);

	tilemap_t    *tilemap_0, *tilemap_1, *tilemap_2, *tilemap_3;
	int          tiledim_0, old_tiledim_0;
	int          tiledim_1, old_tiledim_1;
	int          tiledim_2, old_tiledim_2;
	int          tiledim_3, old_tiledim_3;

	bitmap_t     *sprite_zbuf;
	uint16_t       sprite_zbuf_baseval;

	int          num_sprites;

	int          spriteram_bank;
	int          spriteram_bank_delay;

	uint16_t       *palette_map;

	int          layers_offs_x, layers_offs_y;
	int          row_effect_offs_n;
	int          row_effect_offs_f;
	int          background_color;

	int          spritetype[2];
	int          kludge;


	/* misc */
	int          time_vblank_irq;
	uint8_t        irq_level;
	uint8_t        vblank_irq;
	uint8_t        sound_irq;
	uint8_t        unknown_irq;
	uint8_t        agallet_vblank_irq;

	/* sound related */
	int          soundbuf_len;
	uint8_t        soundbuf_data[32];
	//uint8_t        sound_flag1, sound_flag2;

	/* eeprom-related */
	int          region_byte;

	/* game specific */
	// sailormn
	int          sailormn_tilebank;
	uint8_t        *mirror_ram;
	// korokoro
	uint16_t       leds[2];
	int          hopper;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

/*----------- defined in video/cave.c -----------*/

WRITE16_HANDLER( cave_vram_0_w );
WRITE16_HANDLER( cave_vram_1_w );
WRITE16_HANDLER( cave_vram_2_w );
WRITE16_HANDLER( cave_vram_3_w );

WRITE16_HANDLER( cave_vram_0_8x8_w );
WRITE16_HANDLER( cave_vram_1_8x8_w );
WRITE16_HANDLER( cave_vram_2_8x8_w );
WRITE16_HANDLER( cave_vram_3_8x8_w );

PALETTE_INIT( cave );
PALETTE_INIT( ddonpach );
PALETTE_INIT( dfeveron );
PALETTE_INIT( mazinger );
PALETTE_INIT( sailormn );
PALETTE_INIT( pwrinst2 );
PALETTE_INIT( korokoro );

VIDEO_START( cave_1_layer );
VIDEO_START( cave_2_layers );
VIDEO_START( cave_3_layers );
VIDEO_START( cave_4_layers );

VIDEO_START( sailormn_3_layers );

VIDEO_UPDATE( cave );

void cave_get_sprite_info(running_machine *machine);
void sailormn_tilebank_w(running_machine *machine, int bank);
