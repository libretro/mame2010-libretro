/*************************************************************************

    Universal 8106-A2 + 8106-B PCB set

    and Zero Hour / Red Clash

*************************************************************************/

class ladybug_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ladybug_state(machine)); }

	ladybug_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    colorram;
	uint8_t *    spriteram;
	uint8_t *    grid_data;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap, *grid_tilemap;	// ladybug
	tilemap_t    *fg_tilemap;	// redclash
	uint8_t      grid_color;
	int        star_speed;
	int        gfxbank;	// redclash only
	uint8_t      stars_enable;
	uint8_t      stars_speed;
	uint32_t     stars_state;
	uint16_t     stars_offset;
	uint8_t      stars_count;

	/* misc */
	uint8_t      sound_low;
	uint8_t      sound_high;
	uint8_t      weird_value[8];
	uint8_t      sraider_0x30, sraider_0x38;

	/* devices */
	running_device *maincpu;
};


/*----------- defined in video/ladybug.c -----------*/

WRITE8_HANDLER( ladybug_videoram_w );
WRITE8_HANDLER( ladybug_colorram_w );
WRITE8_HANDLER( ladybug_flipscreen_w );
WRITE8_HANDLER( sraider_io_w );

PALETTE_INIT( ladybug );
VIDEO_START( ladybug );
VIDEO_UPDATE( ladybug );

PALETTE_INIT( sraider );
VIDEO_START( sraider );
VIDEO_UPDATE( sraider );
VIDEO_EOF( sraider );

/*----------- defined in video/redclash.c -----------*/

WRITE8_HANDLER( redclash_videoram_w );
WRITE8_HANDLER( redclash_gfxbank_w );
WRITE8_HANDLER( redclash_flipscreen_w );

WRITE8_HANDLER( redclash_star0_w );
WRITE8_HANDLER( redclash_star1_w );
WRITE8_HANDLER( redclash_star2_w );
WRITE8_HANDLER( redclash_star_reset_w );

PALETTE_INIT( redclash );
VIDEO_START( redclash );
VIDEO_UPDATE( redclash );
VIDEO_EOF( redclash );

/* sraider uses the zerohour star generator board */
void redclash_set_stars_enable(running_machine *machine, uint8_t on);
void redclash_update_stars_state(running_machine *machine);
void redclash_set_stars_speed(running_machine *machine, uint8_t speed);
void redclash_draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, uint8_t palette_offset, uint8_t sraider, uint8_t firstx, uint8_t lastx);
