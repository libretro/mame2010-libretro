/*************************************************************************

    Lock-On hardware

*************************************************************************/

/* Calculated from CRT controller writes */
#define PIXEL_CLOCK            (XTAL_21MHz / 3)
#define FRAMEBUFFER_CLOCK      XTAL_10MHz
#define HBSTART                320
#define HBEND                  0
#define HTOTAL                 448
#define VBSTART                240
#define VBEND                  0
#define VTOTAL                 280


class lockon_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, lockon_state(machine)); }

	lockon_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t	*char_ram;
	uint16_t	*hud_ram;
	uint16_t	*scene_ram;
	uint16_t	*ground_ram;
	uint16_t	*object_ram;

	size_t	hudram_size;
	size_t	objectram_size;
	size_t	groundram_size;

	/* video-related */
	tilemap_t   *tilemap;
	uint8_t	      ground_ctrl;
	uint16_t      scroll_h;
	uint16_t      scroll_v;
	bitmap_t    *front_buffer;
	bitmap_t    *back_buffer;
	emu_timer   *bufend_timer;
	emu_timer   *cursor_timer;

	/* Rotation Control */
	uint16_t      xsal;
	uint16_t      x0ll;
	uint16_t      dx0ll;
	uint16_t      dxll;
	uint16_t      ysal;
	uint16_t      y0ll;
	uint16_t      dy0ll;
	uint16_t      dyll;

	/* Object palette RAM control */
	uint32_t      iden;
	uint8_t	*     obj_pal_ram;
	uint32_t      obj_pal_latch;
	uint32_t      obj_pal_addr;

	/* misc */
	uint8_t       ctrl_reg;
	uint32_t      main_inten;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *ground;
	running_device *object;
	running_device *f2203_1l;
	running_device *f2203_2l;
	running_device *f2203_3l;
	running_device *f2203_1r;
	running_device *f2203_2r;
	running_device *f2203_3r;
};


/*----------- defined in video/lockon.c -----------*/

PALETTE_INIT( lockon );
VIDEO_START( lockon );
VIDEO_UPDATE( lockon );
VIDEO_EOF( lockon );
READ16_HANDLER( lockon_crtc_r );
WRITE16_HANDLER( lockon_crtc_w );
WRITE16_HANDLER( lockon_rotate_w );
WRITE16_HANDLER( lockon_fb_clut_w );
WRITE16_HANDLER( lockon_scene_h_scr_w );
WRITE16_HANDLER( lockon_scene_v_scr_w );
WRITE16_HANDLER( lockon_ground_ctrl_w );
WRITE16_HANDLER( lockon_char_w );

WRITE16_HANDLER( lockon_tza112_w );
READ16_HANDLER( lockon_obj_4000_r );
WRITE16_HANDLER( lockon_obj_4000_w );
