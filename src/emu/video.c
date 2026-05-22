/***************************************************************************

    video.c

    Core MAME video routines.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "profiler.h"
#include "png.h"
#include "debugger.h"
#include "debugint/debugint.h"
#include "rendutil.h"
#include "ui.h"
#include "aviio.h"
#include "crsshair.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _video_global video_global;
struct _video_global
{
	/* screenless systems */
	emu_timer *				screenless_frame_timer;	/* timer to signal VBLANK start */

	/* partial updates */
	uint32_t					partial_updates_this_frame;/* partial update counter this frame */

	/* overall speed computation */
	uint32_t					overall_real_seconds;	/* accumulated real seconds at normal speed */
	osd_ticks_t				overall_real_ticks;		/* accumulated real ticks at normal speed */

	/* configuration */
	uint8_t					auto_frameskip;			/* flag: TRUE if we're automatically frameskipping */
	uint32_t					speed;					/* overall speed (*100) */

	/* frameskipping */
	uint8_t					empty_skip_count;		/* number of empty frames we have skipped */
	uint8_t					frameskip_level;		/* current frameskip level */
	uint8_t					frameskip_counter;		/* counter that counts through the frameskip steps */
	uint8_t					skipping_this_frame;	/* flag: TRUE if we are skipping the current frame */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* global state */
static video_global global;

/* frameskipping tables */
static const uint8_t skiptable[FRAMESKIP_LEVELS][FRAMESKIP_LEVELS] =
{
	{ 0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,1 },
	{ 0,0,0,0,0,1,0,0,0,0,0,1 },
	{ 0,0,0,1,0,0,0,1,0,0,0,1 },
	{ 0,0,1,0,0,1,0,0,1,0,0,1 },
	{ 0,1,0,0,1,0,1,0,0,1,0,1 },
	{ 0,1,0,1,0,1,0,1,0,1,0,1 },
	{ 0,1,0,1,1,0,1,0,1,1,0,1 },
	{ 0,1,1,0,1,1,0,1,1,0,1,1 },
	{ 0,1,1,1,0,1,1,1,0,1,1,1 },
	{ 0,1,1,1,1,1,0,1,1,1,1,1 },
	{ 0,1,1,1,1,1,1,1,1,1,1,1 }
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* core implementation */
static void video_exit(running_machine &machine);
static void init_buffered_spriteram(running_machine *machine);

/* global rendering */
static int finish_screen_updates(running_machine *machine);
static TIMER_CALLBACK( screenless_update_callback );

/* throttling/frameskipping/performance */
static void update_frameskip(running_machine *machine);
static void update_refresh_speed(running_machine *machine);

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    effective_autoframeskip - return the effective
    autoframeskip value, accounting for fast
    forward
-------------------------------------------------*/

INLINE int effective_autoframeskip(running_machine *machine)
{
	/* if we're paused, autoframeskip is disabled */
	if (machine->paused())
		return FALSE;

	/* otherwise, it's up to the user */
	return global.auto_frameskip;
}


/*-------------------------------------------------
    effective_frameskip - return the effective
    frameskip value, accounting for fast
    forward
-------------------------------------------------*/

INLINE int effective_frameskip(void)
{
	/* it's up to the user */
	return global.frameskip_level;
}


/*-------------------------------------------------
    original_speed_setting - return the original
    speed setting
-------------------------------------------------*/

INLINE int original_speed_setting(void)
{
	return options_get_float(mame_options(), OPTION_SPEED) * 100.0 + 0.5;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    video_init - start up the video system
-------------------------------------------------*/

void video_init(running_machine *machine)
{
	/* validate */
	assert(machine != NULL);
	assert(machine->config != NULL);

	/* request a callback upon exiting */
	machine->add_notifier(MACHINE_NOTIFY_EXIT, video_exit);

	/* reset our global state */
	memset(&global, 0, sizeof(global));

	/* extract initial execution state from global configuration settings */
	global.speed = original_speed_setting();
	update_refresh_speed(machine);
	global.auto_frameskip = options_get_bool(machine->options(), OPTION_AUTOFRAMESKIP);
	global.frameskip_level = options_get_int(machine->options(), OPTION_FRAMESKIP);

	/* create spriteram buffers if necessary */
	if (machine->config->m_video_attributes & VIDEO_BUFFERS_SPRITERAM)
		init_buffered_spriteram(machine);

	/* call the PALETTE_INIT function */
	if (machine->config->m_init_palette != NULL)
		(*machine->config->m_init_palette)(machine, memory_region(machine, "proms"));

	/* if no screens, create a periodic timer to drive updates */
	if (machine->primary_screen == NULL)
	{
		global.screenless_frame_timer = timer_alloc(machine, screenless_update_callback, NULL);
		timer_adjust_periodic(global.screenless_frame_timer, screen_device::k_default_frame_period, 0, screen_device::k_default_frame_period);
	}
}


/*-------------------------------------------------
    video_exit - close down the video system
-------------------------------------------------*/

static void video_exit(running_machine &machine)
{
	int i;

	/* free all the graphics elements */
	for (i = 0; i < MAX_GFX_ELEMENTS; i++)
		gfx_element_free(machine.gfx[i]);
}


/*-------------------------------------------------
    init_buffered_spriteram - initialize the
    double-buffered spriteram
-------------------------------------------------*/

static void init_buffered_spriteram(running_machine *machine)
{
	assert_always(machine->generic.spriteram_size != 0, "Video buffers spriteram but spriteram size is 0");

	/* allocate memory for the back buffer */
	machine->generic.buffered_spriteram.u8 = auto_alloc_array(machine, uint8_t, machine->generic.spriteram_size);

	/* register for saving it */
	state_save_register_global_pointer(machine, machine->generic.buffered_spriteram.u8, machine->generic.spriteram_size);

	/* do the same for the second back buffer, if present */
	if (machine->generic.spriteram2_size)
	{
		/* allocate memory */
		machine->generic.buffered_spriteram2.u8 = auto_alloc_array(machine, uint8_t, machine->generic.spriteram2_size);

		/* register for saving it */
		state_save_register_global_pointer(machine, machine->generic.buffered_spriteram2.u8, machine->generic.spriteram2_size);
	}
}



/***************************************************************************
    SCREEN MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    screenless_update_callback - update generator
    when there are no screens to drive it
-------------------------------------------------*/

static TIMER_CALLBACK( screenless_update_callback )
{
	/* force an update */
	video_frame_update(machine, false);
}


/*-------------------------------------------------
    video_frame_update - handle frameskipping and
    UI, plus updating the screen during normal
    operations
-------------------------------------------------*/

void video_frame_update(running_machine *machine, int debug)
{
	int skipped_it = global.skipping_this_frame;
	int phase = machine->phase();

	/* validate */
	assert(machine != NULL);
	assert(machine->config != NULL);

	/* only render sound and video if we're in the running phase */
	if (phase == MACHINE_PHASE_RUNNING && (!machine->paused() || options_get_bool(machine->options(), OPTION_UPDATEINPAUSE)))
	{
		int anything_changed = finish_screen_updates(machine);

		/* if none of the screens changed and we haven't skipped too many frames in a row,
           mark this frame as skipped to prevent throttling; this helps for games that
           don't update their screen at the monitor refresh rate */
		if (!anything_changed && !global.auto_frameskip && global.frameskip_level == 0 && global.empty_skip_count++ < 3)
			skipped_it = TRUE;
		else
			global.empty_skip_count = 0;
	}

	/* draw the user interface */
	ui_update_and_render(machine, render_container_get_ui());

	/* update the internal render debugger */
	debugint_update_during_game(machine);

	/* libretro: the frontend owns frame pacing. Nothing to
	   do here -- we just hand the frame straight to the OSD. */

	/* ask the OSD to update */
	osd_update(machine, !debug && skipped_it);

	/* perform tasks for this frame */
	if (!debug)
		machine->call_notifiers(MACHINE_NOTIFY_FRAME);

	/* update frameskipping */
	if (!debug)
		update_frameskip(machine);

	/* call the end-of-frame callback */
	if (phase == MACHINE_PHASE_RUNNING)
	{
		/* reset partial updates if we're paused or if the debugger is active */
		if (machine->primary_screen != NULL && (machine->paused() || debug || debugger_within_instruction_hook(machine)))
			machine->primary_screen->scanline0_callback();

		/* otherwise, call the video EOF callback */
		else if (machine->config->m_video_eof != NULL)
			(*machine->config->m_video_eof)(machine);
	}
}


/*-------------------------------------------------
    finish_screen_updates - finish updating all
    the screens
-------------------------------------------------*/

static int finish_screen_updates(running_machine *machine)
{
	bool anything_changed = false;

	/* finish updating the screens */
	for (screen_device *screen = screen_first(*machine); screen != NULL; screen = screen_next(screen))
		screen->update_partial(screen->visible_area().max_y);

	/* now add the quads for all the screens */
	for (screen_device *screen = screen_first(*machine); screen != NULL; screen = screen_next(screen))
		if (screen->update_quads())
			anything_changed = true;

	/* update our movie recording and burn-in state */
	if (!machine->paused())
	{
		/* iterate over screens and update the burnin for the ones that care */
		for (screen_device *screen = screen_first(*machine); screen != NULL; screen = screen_next(screen))
			screen->update_burnin();
	}

	/* draw any crosshairs */
	for (screen_device *screen = screen_first(*machine); screen != NULL; screen = screen_next(screen))
		crosshair_render(*screen);

	return anything_changed;
}



/***************************************************************************
    THROTTLING/FRAMESKIPPING/PERFORMANCE
***************************************************************************/

/*-------------------------------------------------
    video_skip_this_frame - accessor to determine
    if this frame is being skipped
-------------------------------------------------*/

int video_skip_this_frame(void)
{
	return global.skipping_this_frame;
}


/*-------------------------------------------------
    video_get_speed_factor - return the speed
    factor as an integer * 100
-------------------------------------------------*/

int video_get_speed_factor(void)
{
	return global.speed;
}


/*-------------------------------------------------
    video_set_speed_factor - sets the speed
    factor as an integer * 100
-------------------------------------------------*/

void video_set_speed_factor(int speed)
{
	global.speed = speed;
}


/*-------------------------------------------------
    video_get_speed_text - print the text to
    be displayed in the upper-right corner
-------------------------------------------------*/

const char *video_get_speed_text(running_machine *machine)
{
	bool paused = machine->paused();
	static char buffer[1024];
	char *dest = buffer;

	/* validate */
	assert(machine != NULL);

	/* if we're paused, just display Paused */
	if (paused)
		dest += sprintf(dest, "paused");

	/* if we're auto frameskipping, display that plus the level */
	else if (effective_autoframeskip(machine))
		dest += sprintf(dest, "auto%2d/%d", effective_frameskip(), MAX_FRAMESKIP);

	/* otherwise, just display the frameskip plus the level */
	else
		dest += sprintf(dest, "skip %d/%d", effective_frameskip(), MAX_FRAMESKIP);

	/* display the number of partial updates as well */
	if (global.partial_updates_this_frame > 1)
		dest += sprintf(dest, "\n%d partial updates", global.partial_updates_this_frame);

	/* return a pointer to the static buffer */
	return buffer;
}


/*-------------------------------------------------
    video_get_speed_percent - return the current
    effective speed percentage. libretro: the
    frontend owns frame pacing; the historical
    wall-clock-derived speed_percent calculation has
    been removed because (a) it leaked osd_ticks()
    state into the inptport.c INP record path,
    breaking deterministic input playback, and (b)
    the only remaining consumer beyond the INP
    record was the OSD status line, which the
    libretro frontend does not surface. A constant
    1.0 keeps the inptport.c writer producing a
    fixed, deterministic 0x00100000 word per frame.
-------------------------------------------------*/

double video_get_speed_percent(running_machine *machine)
{
	(void)machine;
	return 1.0;
}


/*-------------------------------------------------
    video_get_frameskip - return the current
    actual frameskip (-1 means autoframeskip)
-------------------------------------------------*/

int video_get_frameskip(void)
{
	/* if autoframeskip is on, return -1 */
	if (global.auto_frameskip)
		return -1;

	/* otherwise, return the direct level */
	else
		return global.frameskip_level;
}


/*-------------------------------------------------
    video_set_frameskip - set the current
    actual frameskip (-1 means autoframeskip)
-------------------------------------------------*/

void video_set_frameskip(int frameskip)
{
	/* -1 means autoframeskip */
	if (frameskip == -1)
	{
		global.auto_frameskip = TRUE;
		global.frameskip_level = 0;
	}

	/* any other level is a direct control */
	else if (frameskip >= 0 && frameskip <= MAX_FRAMESKIP)
	{
		global.auto_frameskip = FALSE;
		global.frameskip_level = frameskip;
	}
}


/*-------------------------------------------------
    update_frameskip - advance the frameskip
    counter and decide whether the next frame
    should be skipped
-------------------------------------------------*/

static void update_frameskip(running_machine *machine)
{
	/* libretro: the historical 'if throttling and autoframeskip, adjust
	   frameskip_level based on speed_percent' block that used to sit
	   here is dead -- effective_throttle() returned FALSE
	   unconditionally and effective_autoframeskip() is only true when
	   not fast-forwarding, so the gate could never open. The auto
	   adjustment logic and the frameskip_adjust state it carried are
	   gone with the rest of the throttle path; only the deterministic
	   counter walk and skip-table lookup remain. */
	(void)machine;

	/* increment the frameskip counter and determine if we will skip the next frame */
	global.frameskip_counter = (global.frameskip_counter + 1) % FRAMESKIP_LEVELS;
	global.skipping_this_frame = skiptable[effective_frameskip()][global.frameskip_counter];
}


/*-------------------------------------------------
    update_refresh_speed - update the global.speed
    based on the maximum refresh rate supported
-------------------------------------------------*/

static void update_refresh_speed(running_machine *machine)
{
	/* only do this if the refreshspeed option is used */
	if (options_get_bool(machine->options(), OPTION_REFRESHSPEED))
	{
		float minrefresh = render_get_max_update_rate();
		if (minrefresh != 0)
		{
			attoseconds_t min_frame_period = ATTOSECONDS_PER_SECOND;
			uint32_t original_speed = original_speed_setting();
			uint32_t target_speed;

			/* find the screen with the shortest frame period (max refresh rate) */
			/* note that we first check the token since this can get called before all screens are created */
			for (screen_device *screen = screen_first(*machine); screen != NULL; screen = screen_next(screen))
			{
				attoseconds_t period = screen->frame_period().attoseconds;
				if (period != 0)
					min_frame_period = MIN(min_frame_period, period);
			}

			/* compute a target speed as an integral percentage */
			/* note that we lop 0.25Hz off of the minrefresh when doing the computation to allow for
               the fact that most refresh rates are not accurate to 10 digits... */
			target_speed = floor((minrefresh - 0.25f) * 100.0 / ATTOSECONDS_TO_HZ(min_frame_period));
			target_speed = MIN(target_speed, original_speed);

			/* if we changed, log that verbosely */
			if (target_speed != global.speed)
			{
				mame_printf_verbose("Adjusting target speed to %d%% (hw=%.2fHz, game=%.2fHz, adjusted=%.2fHz)\n", target_speed, minrefresh, ATTOSECONDS_TO_HZ(min_frame_period), ATTOSECONDS_TO_HZ(min_frame_period * 100 / target_speed));
				global.speed = target_speed;
			}
		}
	}
}


/***************************************************************************
    BURN-IN GENERATION
***************************************************************************/

/***************************************************************************
    CONFIGURATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    video_get_view_for_target - select a view
    for a given target
-------------------------------------------------*/

int video_get_view_for_target(running_machine *machine, render_target *target, const char *viewname, int targetindex, int numtargets)
{
	int viewindex = -1;

	/* auto view just selects the nth view */
	if (strcmp(viewname, "auto") != 0)
	{
		/* scan for a matching view name */
		for (viewindex = 0; ; viewindex++)
		{
			const char *name = render_target_get_view_name(target, viewindex);

			/* stop scanning when we hit NULL */
			if (name == NULL)
			{
				viewindex = -1;
				break;
			}
			if (mame_strnicmp(name, viewname, strlen(viewname)) == 0)
				break;
		}
	}

	/* if we don't have a match, default to the nth view */
	if (viewindex == -1)
	{
		int scrcount = screen_count(*machine->config);

		/* if we have enough targets to be one per screen, assign in order */
		if (numtargets >= scrcount)
		{
			/* find the first view with this screen and this screen only */
			for (viewindex = 0; ; viewindex++)
			{
				uint32_t viewscreens = render_target_get_view_screens(target, viewindex);
				if (viewscreens == (1 << targetindex))
					break;
				if (viewscreens == 0)
				{
					viewindex = -1;
					break;
				}
			}
		}

		/* otherwise, find the first view that has all the screens */
		if (viewindex == -1)
		{
			for (viewindex = 0; ; viewindex++)
			{
				uint32_t viewscreens = render_target_get_view_screens(target, viewindex);
				if (viewscreens == (1 << scrcount) - 1)
					break;
				if (viewscreens == 0)
					break;
			}
		}
	}

	/* make sure it's a valid view */
	if (render_target_get_view_name(target, viewindex) == NULL)
		viewindex = 0;

	return viewindex;
}



/***************************************************************************
    DEBUGGING HELPERS
***************************************************************************/

/*-------------------------------------------------
    video_assert_out_of_range_pixels - assert if
    any pixels in the given bitmap contain an
    invalid palette index
-------------------------------------------------*/

void video_assert_out_of_range_pixels(running_machine *machine, bitmap_t *bitmap)
{
#ifdef MAME_DEBUG
	int maxindex = palette_get_max_index(machine->palette);
	int x, y;

	/* this only applies to indexed16 bitmaps */
	if (bitmap->format != BITMAP_FORMAT_INDEXED16)
		return;

	/* iterate over rows */
	for (y = 0; y < bitmap->height; y++)
	{
		uint16_t *rowbase = BITMAP_ADDR16(bitmap, y, 0);
		for (x = 0; x < bitmap->width; x++)
			assert(rowbase[x] < maxindex);
	}
#endif
}



//**************************************************************************
//  VIDEO SCREEN DEVICE CONFIGURATION
//**************************************************************************

const attotime screen_device::k_default_frame_period = STATIC_ATTOTIME_IN_HZ(k_default_frame_rate);


//-------------------------------------------------
//  screen_device_config - constructor
//-------------------------------------------------

screen_device_config::screen_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, uint32_t clock)
	: device_config(mconfig, static_alloc_device_config, "Video Screen", tag, owner, clock),
	  m_type(SCREEN_TYPE_RASTER),
	  m_width(0),
	  m_height(0),
	  m_oldstyle_vblank_supplied(false),
	  m_refresh(0),
	  m_vblank(0),
	  m_format(BITMAP_FORMAT_INVALID),
	  m_xoffset(0.0f),
	  m_yoffset(0.0f),
	  m_xscale(1.0f),
	  m_yscale(1.0f)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *screen_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, uint32_t clock)
{
	return global_alloc(screen_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *screen_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, screen_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void screen_device_config::device_config_complete()
{
	// move inline data into its final home
	m_type = static_cast<screen_type_enum>(m_inline_data[INLINE_TYPE]);
	m_width = static_cast<int16_t>(m_inline_data[INLINE_WIDTH]);
	m_height = static_cast<int16_t>(m_inline_data[INLINE_HEIGHT]);
	m_visarea.min_x = static_cast<int16_t>(m_inline_data[INLINE_VIS_MINX]);
	m_visarea.min_y = static_cast<int16_t>(m_inline_data[INLINE_VIS_MINY]);
	m_visarea.max_x = static_cast<int16_t>(m_inline_data[INLINE_VIS_MAXX]);
	m_visarea.max_y = static_cast<int16_t>(m_inline_data[INLINE_VIS_MAXY]);
	m_oldstyle_vblank_supplied = (m_inline_data[INLINE_OLDVBLANK] != 0);
	m_refresh = m_inline_data[INLINE_REFRESH];
	m_vblank = m_inline_data[INLINE_VBLANK];
	m_format = static_cast<bitmap_format>(m_inline_data[INLINE_FORMAT]);
	m_xoffset = static_cast<double>(static_cast<int32_t>(m_inline_data[INLINE_XOFFSET])) / (double)(1 << 24);
	m_yoffset = static_cast<double>(static_cast<int32_t>(m_inline_data[INLINE_YOFFSET])) / (double)(1 << 24);
	m_xscale = (m_inline_data[INLINE_XSCALE] == 0) ? 1.0 : (static_cast<double>(static_cast<int32_t>(m_inline_data[INLINE_XSCALE])) / (double)(1 << 24));
	m_yscale = (m_inline_data[INLINE_YSCALE] == 0) ? 1.0 : (static_cast<double>(static_cast<int32_t>(m_inline_data[INLINE_YSCALE])) / (double)(1 << 24));
}


//-------------------------------------------------
//  device_validity_check - verify device
//  configuration
//-------------------------------------------------

bool screen_device_config::device_validity_check(const game_driver &driver) const
{
	bool error = false;

	// sanity check dimensions
	if (m_width <= 0 || m_height <= 0)
	{
		mame_printf_error("%s: %s screen '%s' has invalid display dimensions\n", driver.source_file, driver.name, tag());
		error = true;
	}

	// sanity check display area
	if (m_type != SCREEN_TYPE_VECTOR)
	{
		if ((m_visarea.max_x < m_visarea.min_x) ||
			(m_visarea.max_y < m_visarea.min_y) ||
			(m_visarea.max_x >= m_width) ||
			(m_visarea.max_y >= m_height))
		{
			mame_printf_error("%s: %s screen '%s' has an invalid display area\n", driver.source_file, driver.name, tag());
			error = true;
		}

		// sanity check screen formats
		if (m_format != BITMAP_FORMAT_INDEXED16 &&
			m_format != BITMAP_FORMAT_RGB15 &&
			m_format != BITMAP_FORMAT_RGB32)
		{
			mame_printf_error("%s: %s screen '%s' has unsupported format\n", driver.source_file, driver.name, tag());
			error = true;
		}
	}

	// check for zero frame rate
	if (m_refresh == 0)
	{
		mame_printf_error("%s: %s screen '%s' has a zero refresh rate\n", driver.source_file, driver.name, tag());
		error = true;
	}
	return error;
}



//**************************************************************************
//  LIVE VIDEO SCREEN DEVICE
//**************************************************************************

//-------------------------------------------------
//  screen_device - constructor
//-------------------------------------------------

screen_device::screen_device(running_machine &_machine, const screen_device_config &config)
	: device_t(_machine, config),
	  m_config(config),
	  m_width(m_config.m_width),
	  m_height(m_config.m_height),
	  m_visarea(m_config.m_visarea),
	  m_burnin(NULL),
	  m_curbitmap(0),
	  m_curtexture(0),
	  m_texture_format(0),
	  m_changed(true),
	  m_last_partial_scan(0),
	  m_frame_period(m_config.m_refresh),
	  m_scantime(1),
	  m_pixeltime(1),
	  m_vblank_period(0),
	  m_vblank_start_time(attotime_zero),
	  m_vblank_end_time(attotime_zero),
	  m_vblank_begin_timer(NULL),
	  m_vblank_end_timer(NULL),
	  m_scanline0_timer(NULL),
	  m_scanline_timer(NULL),
	  m_frame_number(0),
	  m_callback_list(NULL)
{
	memset(m_texture, 0, sizeof(m_texture));
	memset(m_bitmap, 0, sizeof(m_bitmap));
}


//-------------------------------------------------
//  ~screen_device - destructor
//-------------------------------------------------

screen_device::~screen_device()
{
	if (m_texture[0] != NULL)
		render_texture_free(m_texture[0]);
	if (m_texture[1] != NULL)
		render_texture_free(m_texture[1]);
	if (m_burnin != NULL)
		finalize_burnin();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void screen_device::device_start()
{
	// get and validate that the container for this screen exists
	render_container *container = render_container_get_screen(this);
	assert(container != NULL);

	// configure the default cliparea
	render_container_user_settings settings;
	render_container_get_user_settings(container, &settings);
	settings.xoffset = m_config.m_xoffset;
	settings.yoffset = m_config.m_yoffset;
	settings.xscale = m_config.m_xscale;
	settings.yscale = m_config.m_yscale;
	render_container_set_user_settings(container, &settings);

	// allocate the VBLANK timers
	m_vblank_begin_timer = timer_alloc(machine, static_vblank_begin_callback, (void *)this);
	m_vblank_end_timer = timer_alloc(machine, static_vblank_end_callback, (void *)this);

	// allocate a timer to reset partial updates
	m_scanline0_timer = timer_alloc(machine, static_scanline0_callback, (void *)this);

	// allocate a timer to generate per-scanline updates
	if ((machine->config->m_video_attributes & VIDEO_UPDATE_SCANLINE) != 0)
		m_scanline_timer = timer_alloc(machine, static_scanline_update_callback, (void *)this);

	// configure the screen with the default parameters
	configure(m_config.m_width, m_config.m_height, m_config.m_visarea, m_config.m_refresh);

	// reset VBLANK timing */
	m_vblank_start_time = attotime_zero;
	m_vblank_end_time = attotime_make(0, m_vblank_period);

	// start the timer to generate per-scanline updates
	if ((machine->config->m_video_attributes & VIDEO_UPDATE_SCANLINE) != 0)
		timer_adjust_oneshot(m_scanline_timer, time_until_pos(0), 0);

	// create burn-in bitmap
	if (options_get_int(machine->options(), OPTION_BURNIN) > 0)
	{
		int width, height;
		if (sscanf(options_get_string(machine->options(), OPTION_SNAPSIZE), "%dx%d", &width, &height) != 2 || width == 0 || height == 0)
			width = height = 300;
		m_burnin = auto_alloc(machine, bitmap_t(width, height, BITMAP_FORMAT_INDEXED64));
		if (m_burnin == NULL)
			fatalerror("Error allocating burn-in bitmap for screen at (%dx%d)\n", width, height);
		bitmap_fill(m_burnin, NULL, 0);
	}

	state_save_register_device_item(this, 0, m_width);
	state_save_register_device_item(this, 0, m_height);
	state_save_register_device_item(this, 0, m_visarea.min_x);
	state_save_register_device_item(this, 0, m_visarea.min_y);
	state_save_register_device_item(this, 0, m_visarea.max_x);
	state_save_register_device_item(this, 0, m_visarea.max_y);
	state_save_register_device_item(this, 0, m_last_partial_scan);
	state_save_register_device_item(this, 0, m_frame_period);
	state_save_register_device_item(this, 0, m_scantime);
	state_save_register_device_item(this, 0, m_pixeltime);
	state_save_register_device_item(this, 0, m_vblank_period);
	state_save_register_device_item(this, 0, m_vblank_start_time.seconds);
	state_save_register_device_item(this, 0, m_vblank_start_time.attoseconds);
	state_save_register_device_item(this, 0, m_vblank_end_time.seconds);
	state_save_register_device_item(this, 0, m_vblank_end_time.attoseconds);
	state_save_register_device_item(this, 0, m_frame_number);
}


//-------------------------------------------------
//  device_post_load - device-specific update
//  after a save state is loaded
//-------------------------------------------------

void screen_device::device_post_load()
{
	realloc_screen_bitmaps();
}


//-------------------------------------------------
//  configure - configure screen parameters
//-------------------------------------------------

void screen_device::configure(int width, int height, const rectangle &visarea, attoseconds_t frame_period)
{
	// validate arguments
	assert(width > 0);
	assert(height > 0);
	assert(visarea.min_x >= 0);
	assert(visarea.min_y >= 0);
	assert(m_config.m_type == SCREEN_TYPE_VECTOR || visarea.min_x < width);
	assert(m_config.m_type == SCREEN_TYPE_VECTOR || visarea.min_y < height);
	assert(frame_period > 0);

	// fill in the new parameters
	m_width = width;
	m_height = height;
	m_visarea = visarea;

	// reallocate bitmap if necessary
	realloc_screen_bitmaps();

	// compute timing parameters
	m_frame_period = frame_period;
	m_scantime = frame_period / height;
	m_pixeltime = frame_period / (height * width);

	// if there has been no VBLANK time specified in the MACHINE_DRIVER, compute it now
    // from the visible area, otherwise just used the supplied value
	if (m_config.m_vblank == 0 && !m_config.m_oldstyle_vblank_supplied)
		m_vblank_period = m_scantime * (height - (visarea.max_y + 1 - visarea.min_y));
	else
		m_vblank_period = m_config.m_vblank;

	// if we are on scanline 0 already, reset the update timer immediately
	// otherwise, defer until the next scanline 0
	if (vpos() == 0)
		timer_adjust_oneshot(m_scanline0_timer, attotime_zero, 0);
	else
		timer_adjust_oneshot(m_scanline0_timer, time_until_pos(0), 0);

	// start the VBLANK timer
	timer_adjust_oneshot(m_vblank_begin_timer, time_until_vblank_start(), 0);

	// adjust speed if necessary
	update_refresh_speed(machine);
}


//-------------------------------------------------
//  realloc_screen_bitmaps - reallocate bitmaps
//  and textures as necessary
//-------------------------------------------------

void screen_device::realloc_screen_bitmaps()
{
	if (m_config.m_type == SCREEN_TYPE_VECTOR)
		return;

	int curwidth = 0, curheight = 0;

	// extract the current width/height from the bitmap
	if (m_bitmap[0] != NULL)
	{
		curwidth = m_bitmap[0]->width;
		curheight = m_bitmap[0]->height;
	}

	// if we're too small to contain this width/height, reallocate our bitmaps and textures
	if (m_width > curwidth || m_height > curheight)
	{
		// free what we have currently
		if (m_texture[0] != NULL)
			render_texture_free(m_texture[0]);
		if (m_texture[1] != NULL)
			render_texture_free(m_texture[1]);
		if (m_bitmap[0] != NULL)
			auto_free(machine, m_bitmap[0]);
		if (m_bitmap[1] != NULL)
			auto_free(machine, m_bitmap[1]);

		// compute new width/height
		curwidth = MAX(m_width, curwidth);
		curheight = MAX(m_height, curheight);

		// choose the texture format - convert the screen format to a texture format
		palette_t *palette = NULL;
		switch (m_config.m_format)
		{
			case BITMAP_FORMAT_INDEXED16:	m_texture_format = TEXFORMAT_PALETTE16;	palette = machine->palette;	break;
			case BITMAP_FORMAT_RGB15:		m_texture_format = TEXFORMAT_RGB15;		palette = NULL;				break;
			case BITMAP_FORMAT_RGB32:		m_texture_format = TEXFORMAT_RGB32;		palette = NULL;				break;
			default:						fatalerror("Invalid bitmap format!");												break;
		}

		// allocate bitmaps
		m_bitmap[0] = auto_alloc(machine, bitmap_t(curwidth, curheight, m_config.m_format));
		bitmap_set_palette(m_bitmap[0], machine->palette);
		m_bitmap[1] = auto_alloc(machine, bitmap_t(curwidth, curheight, m_config.m_format));
		bitmap_set_palette(m_bitmap[1], machine->palette);

		// allocate textures
		m_texture[0] = render_texture_alloc(NULL, NULL);
		render_texture_set_bitmap(m_texture[0], m_bitmap[0], &m_visarea, m_texture_format, palette);
		m_texture[1] = render_texture_alloc(NULL, NULL);
		render_texture_set_bitmap(m_texture[1], m_bitmap[1], &m_visarea, m_texture_format, palette);
	}
}


//-------------------------------------------------
//  set_visible_area - just set the visible area
//-------------------------------------------------

void screen_device::set_visible_area(int min_x, int max_x, int min_y, int max_y)
{
	// validate arguments
	assert(min_x >= 0);
	assert(min_y >= 0);
	assert(min_x < max_x);
	assert(min_y < max_y);

	rectangle visarea;
	visarea.min_x = min_x;
	visarea.max_x = max_x;
	visarea.min_y = min_y;
	visarea.max_y = max_y;

	configure(m_width, m_height, visarea, m_frame_period);
}


//-------------------------------------------------
//  update_partial - perform a partial update from
//  the last scanline up to and including the
//  specified scanline
//-----------------------------------------------*/

bool screen_device::update_partial(int scanline)
{
	// validate arguments
	assert(scanline >= 0);

	// these two checks only apply if we're allowed to skip frames
	if (!(machine->config->m_video_attributes & VIDEO_ALWAYS_UPDATE))
	{
		// if skipping this frame, bail
      /* skipped due to frameskipping */
		if (global.skipping_this_frame)
			return FALSE;

		// skip if this screen is not visible anywhere
		/* skipped because screen not live */
		if (!render_is_live_screen(this))
			return FALSE;
	}

	// skip if less than the lowest so far
   /* skipped because less than previous */
	if (scanline < m_last_partial_scan)
		return FALSE;

	// set the start/end scanlines
	rectangle clip = m_visarea;
	if (m_last_partial_scan > clip.min_y)
		clip.min_y = m_last_partial_scan;
	if (scanline < clip.max_y)
		clip.max_y = scanline;

	// render if necessary
	bool result = false;
	if (clip.min_y <= clip.max_y)
	{
		uint32_t flags = UPDATE_HAS_NOT_CHANGED;

		if (machine->config->m_video_update != NULL)
			flags = (*machine->config->m_video_update)(this, m_bitmap[m_curbitmap], &clip);
		global.partial_updates_this_frame++;

		// if we modified the bitmap, we have to commit
		m_changed |= ~flags & UPDATE_HAS_NOT_CHANGED;
		result = true;
	}

	// remember where we left off
	m_last_partial_scan = scanline + 1;
	return result;
}


//-------------------------------------------------
//  update_now - perform an update from the last
//  beam position up to the current beam position
//-------------------------------------------------

void screen_device::update_now()
{
	int current_vpos = vpos();
	int current_hpos = hpos();

	// since we can currently update only at the scanline
    // level, we are trying to do the right thing by
    // updating including the current scanline, only if the
    // beam is past the halfway point horizontally.
    // If the beam is in the first half of the scanline,
    // we only update up to the previous scanline.
    // This minimizes the number of pixels that might be drawn
    // incorrectly until we support a pixel level granularity
	if (current_hpos < (m_width / 2) && current_vpos > 0)
		current_vpos = current_vpos - 1;

	update_partial(current_vpos);
}


//-------------------------------------------------
//  vpos - returns the current vertical position
//  of the beam
//-------------------------------------------------

int screen_device::vpos() const
{
	attoseconds_t delta = attotime_to_attoseconds(attotime_sub(timer_get_time(machine), m_vblank_start_time));
	int vpos;

	// round to the nearest pixel
	delta += m_pixeltime / 2;

	// compute the v position relative to the start of VBLANK
	vpos = delta / m_scantime;

	// adjust for the fact that VBLANK starts at the bottom of the visible area
	return (m_visarea.max_y + 1 + vpos) % m_height;
}


//-------------------------------------------------
//  hpos - returns the current horizontal position
//  of the beam
//-------------------------------------------------

int screen_device::hpos() const
{
	attoseconds_t delta = attotime_to_attoseconds(attotime_sub(timer_get_time(machine), m_vblank_start_time));

	// round to the nearest pixel
	delta += m_pixeltime / 2;

	// compute the v position relative to the start of VBLANK
	int vpos = delta / m_scantime;

	// subtract that from the total time
	delta -= vpos * m_scantime;

	// return the pixel offset from the start of this scanline
	return delta / m_pixeltime;
}


//-------------------------------------------------
//  time_until_pos - returns the amount of time
//  remaining until the beam is at the given
//  hpos,vpos
//-------------------------------------------------

attotime screen_device::time_until_pos(int vpos, int hpos) const
{
	// validate arguments
	assert(vpos >= 0);
	assert(hpos >= 0);

	// since we measure time relative to VBLANK, compute the scanline offset from VBLANK
	vpos += m_height - (m_visarea.max_y + 1);
	vpos %= m_height;

	// compute the delta for the given X,Y position
	attoseconds_t targetdelta = (attoseconds_t)vpos * m_scantime + (attoseconds_t)hpos * m_pixeltime;

	// if we're past that time (within 1/2 of a pixel), head to the next frame
	attoseconds_t curdelta = attotime_to_attoseconds(attotime_sub(timer_get_time(machine), m_vblank_start_time));
	if (targetdelta <= curdelta + m_pixeltime / 2)
		targetdelta += m_frame_period;
	while (targetdelta <= curdelta)
		targetdelta += m_frame_period;

	// return the difference
	return attotime_make(0, targetdelta - curdelta);
}


//-------------------------------------------------
//  time_until_vblank_end - returns the amount of
//  time remaining until the end of the current
//  VBLANK (if in progress) or the end of the next
//  VBLANK
//-------------------------------------------------

attotime screen_device::time_until_vblank_end() const
{
	// if we are in the VBLANK region, compute the time until the end of the current VBLANK period
	attotime target_time = m_vblank_end_time;
	if (!vblank())
		target_time = attotime_add_attoseconds(target_time, m_frame_period);
	return attotime_sub(target_time, timer_get_time(machine));
}


//-------------------------------------------------
//  register_vblank_callback - registers a VBLANK
//  callback
//-------------------------------------------------

void screen_device::register_vblank_callback(vblank_state_changed_func vblank_callback, void *param)
{
	// validate arguments
	assert(vblank_callback != NULL);

	// check if we already have this callback registered
	callback_item **itemptr;
	for (itemptr = &m_callback_list; *itemptr != NULL; itemptr = &(*itemptr)->m_next)
		if ((*itemptr)->m_callback == vblank_callback)
			break;

	// if not found, register
	if (*itemptr == NULL)
	{
		*itemptr = auto_alloc(machine, callback_item);
		(*itemptr)->m_next = NULL;
		(*itemptr)->m_callback = vblank_callback;
		(*itemptr)->m_param = param;
	}
}


//-------------------------------------------------
//  vblank_begin_callback - call any external
//  callbacks to signal the VBLANK period has begun
//-------------------------------------------------

void screen_device::vblank_begin_callback()
{
	// reset the starting VBLANK time
	m_vblank_start_time = timer_get_time(machine);
	m_vblank_end_time = attotime_add_attoseconds(m_vblank_start_time, m_vblank_period);

	// call the screen specific callbacks
	for (callback_item *item = m_callback_list; item != NULL; item = item->m_next)
		(*item->m_callback)(*this, item->m_param, true);

	// if this is the primary screen and we need to update now
	if (this == machine->primary_screen && !(machine->config->m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
		video_frame_update(machine, FALSE);

	// reset the VBLANK start timer for the next frame
	timer_adjust_oneshot(m_vblank_begin_timer, time_until_vblank_start(), 0);

	// if no VBLANK period, call the VBLANK end callback immedietely, otherwise reset the timer
	if (m_vblank_period == 0)
		vblank_end_callback();
	else
		timer_adjust_oneshot(m_vblank_end_timer, time_until_vblank_end(), 0);
}


//-------------------------------------------------
//  vblank_end_callback - call any external
//  callbacks to signal the VBLANK period has ended
//-------------------------------------------------

void screen_device::vblank_end_callback()
{
	// call the screen specific callbacks
	for (callback_item *item = m_callback_list; item != NULL; item = item->m_next)
		(*item->m_callback)(*this, item->m_param, false);

	// if this is the primary screen and we need to update now
	if (this == machine->primary_screen && (machine->config->m_video_attributes & VIDEO_UPDATE_AFTER_VBLANK))
		video_frame_update(machine, FALSE);

	// increment the frame number counter
	m_frame_number++;
}


//-------------------------------------------------
//  scanline0_callback - reset partial updates
//  for a screen
//-------------------------------------------------

void screen_device::scanline0_callback()
{
	// reset partial updates
	m_last_partial_scan = 0;
	global.partial_updates_this_frame = 0;

	timer_adjust_oneshot(m_scanline0_timer, time_until_pos(0), 0);
}


//-------------------------------------------------
//  scanline_update_callback - perform partial
//  updates on each scanline
//-------------------------------------------------

void screen_device::scanline_update_callback(int scanline)
{
	// force a partial update to the current scanline
	update_partial(scanline);

	// compute the next visible scanline
	scanline++;
	if (scanline > m_visarea.max_y)
		scanline = m_visarea.min_y;
	timer_adjust_oneshot(m_scanline_timer, time_until_pos(scanline), scanline);
}


//-------------------------------------------------
//  update_quads - set up the quads for this
//  screen
//-------------------------------------------------

bool screen_device::update_quads()
{
	// only update if live
	if (render_is_live_screen(this))
	{
		// only update if empty and not a vector game; otherwise assume the driver did it directly
		if (m_config.m_type != SCREEN_TYPE_VECTOR && (machine->config->m_video_attributes & VIDEO_SELF_RENDER) == 0)
		{
			// if we're not skipping the frame and if the screen actually changed, then update the texture
			if (!global.skipping_this_frame && m_changed)
			{
				rectangle fixedvis = m_visarea;
				fixedvis.max_x++;
				fixedvis.max_y++;

				palette_t *palette = (m_texture_format == TEXFORMAT_PALETTE16) ? machine->palette : NULL;
				render_texture_set_bitmap(m_texture[m_curbitmap], m_bitmap[m_curbitmap], &fixedvis, m_texture_format, palette);

				m_curtexture = m_curbitmap;
				m_curbitmap = 1 - m_curbitmap;
			}

			// create an empty container with a single quad
			render_container_empty(render_container_get_screen(this));
			render_screen_add_quad(this, 0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(0xff,0xff,0xff,0xff), m_texture[m_curtexture], PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));
		}
	}

	// reset the screen changed flags
	bool result = m_changed;
	m_changed = false;
	return result;
}


//-------------------------------------------------
//  update_burnin - update the burnin bitmap
//-------------------------------------------------

void screen_device::update_burnin()
{
#undef rand
	if (m_burnin == NULL)
		return;

	bitmap_t *srcbitmap = m_bitmap[m_curtexture];
	if (srcbitmap == NULL)
		return;

	int srcwidth = srcbitmap->width;
	int srcheight = srcbitmap->height;
	int dstwidth = m_burnin->width;
	int dstheight = m_burnin->height;
	int xstep = (srcwidth << 16) / dstwidth;
	int ystep = (srcheight << 16) / dstheight;
	int xstart = ((uint32_t)rand() % 32767) * xstep / 32767;
	int ystart = ((uint32_t)rand() % 32767) * ystep / 32767;
	int srcx, srcy;
	int x, y;

	// iterate over rows in the destination
	for (y = 0, srcy = ystart; y < dstheight; y++, srcy += ystep)
	{
		uint64_t *dst = BITMAP_ADDR64(m_burnin, y, 0);

		// handle the 16-bit palettized case
		if (srcbitmap->format == BITMAP_FORMAT_INDEXED16)
		{
			const uint16_t *src = BITMAP_ADDR16(srcbitmap, srcy >> 16, 0);
			const rgb_t *palette = palette_entry_list_adjusted(machine->palette);
			for (x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
			{
				rgb_t pixel = palette[src[srcx >> 16]];
				dst[x] += RGB_GREEN(pixel) + RGB_RED(pixel) + RGB_BLUE(pixel);
			}
		}

		// handle the 15-bit RGB case
		else if (srcbitmap->format == BITMAP_FORMAT_RGB15)
		{
			const uint16_t *src = BITMAP_ADDR16(srcbitmap, srcy >> 16, 0);
			for (x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
			{
				rgb15_t pixel = src[srcx >> 16];
				dst[x] += ((pixel >> 10) & 0x1f) + ((pixel >> 5) & 0x1f) + ((pixel >> 0) & 0x1f);
			}
		}

		// handle the 32-bit RGB case
		else if (srcbitmap->format == BITMAP_FORMAT_RGB32)
		{
			const uint32_t *src = BITMAP_ADDR32(srcbitmap, srcy >> 16, 0);
			for (x = 0, srcx = xstart; x < dstwidth; x++, srcx += xstep)
			{
				rgb_t pixel = src[srcx >> 16];
				dst[x] += RGB_GREEN(pixel) + RGB_RED(pixel) + RGB_BLUE(pixel);
			}
		}
	}
}


//-------------------------------------------------
//  video_finalize_burnin - finalize the burnin
//  bitmap
//-------------------------------------------------

void screen_device::finalize_burnin()
{

}

const device_type SCREEN = screen_device_config::static_alloc_device_config;
