/***************************************************************************

    Century CVS System

    (and Quasar)

****************************************************************************/


#define CVS_S2636_Y_OFFSET     (3)
#define CVS_S2636_X_OFFSET     (-26)
#define CVS_MAX_STARS          250

struct cvs_star
{
	int x, y, code;
};

class cvs_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cvs_state(machine)); }

	cvs_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    video_ram;
	uint8_t *    bullet_ram;
	uint8_t *    fo_state;
	uint8_t *    cvs_4_bit_dac_data;
	uint8_t *    tms5110_ctl_data;
	uint8_t *    dac3_state;
	uint8_t *    color_ram;
	uint8_t *    palette_ram;
	uint8_t *    character_ram;
	uint8_t *    effectram;	// quasar

	/* video-related */
	struct cvs_star stars[CVS_MAX_STARS];
	bitmap_t   *collision_background;
	bitmap_t   *background_bitmap;
	bitmap_t   *scrolled_collision_background;
	int        collision_register;
	int        total_stars;
	int        stars_on;
	uint8_t      scroll_reg;
	uint8_t      effectcontrol;	// quasar
	int        stars_scroll;

	/* misc */
	emu_timer  *cvs_393hz_timer;
	uint8_t      cvs_393hz_clock;

	uint8_t      character_banking_mode;
	uint16_t     character_ram_page_start;
	uint16_t     speech_rom_bit_address;

	uint8_t      page, io_page;	// quasar

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *speech;
	running_device *dac3;
	running_device *tms;
	running_device *s2636_0;
	running_device *s2636_1;
	running_device *s2636_2;
};

/*----------- defined in drivers/cvs.c -----------*/

MACHINE_START( cvs );
MACHINE_RESET( cvs );

READ8_HANDLER( cvs_video_or_color_ram_r );
WRITE8_HANDLER( cvs_video_or_color_ram_w );

READ8_HANDLER( cvs_bullet_ram_or_palette_r );
WRITE8_HANDLER( cvs_bullet_ram_or_palette_w );

READ8_HANDLER( cvs_s2636_0_or_character_ram_r );
WRITE8_HANDLER( cvs_s2636_0_or_character_ram_w );
READ8_HANDLER( cvs_s2636_1_or_character_ram_r );
WRITE8_HANDLER( cvs_s2636_1_or_character_ram_w );
READ8_HANDLER( cvs_s2636_2_or_character_ram_r );
WRITE8_HANDLER( cvs_s2636_2_or_character_ram_w );

/*----------- defined in video/cvs.c -----------*/

WRITE8_HANDLER( cvs_scroll_w );
WRITE8_HANDLER( cvs_video_fx_w );

READ8_HANDLER( cvs_collision_r );
READ8_HANDLER( cvs_collision_clear );

void cvs_scroll_stars(running_machine *machine);

PALETTE_INIT( cvs );
VIDEO_UPDATE( cvs );
VIDEO_START( cvs );

/*----------- defined in video/quasar.c -----------*/

PALETTE_INIT( quasar );
VIDEO_UPDATE( quasar );
VIDEO_START( quasar );
