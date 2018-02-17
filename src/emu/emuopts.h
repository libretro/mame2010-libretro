/***************************************************************************

    emuopts.h

    Options file and command line management.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMUOPTS_H__
#define __EMUOPTS_H__

#include "options.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* option priorities */
#define OPTION_PRIORITY_CMDLINE			OPTION_PRIORITY_HIGH
#define OPTION_PRIORITY_INI				OPTION_PRIORITY_NORMAL
#define OPTION_PRIORITY_MAME_INI		(OPTION_PRIORITY_NORMAL + 1)
#define OPTION_PRIORITY_DEBUG_INI		(OPTION_PRIORITY_MAME_INI + 1)
#define OPTION_PRIORITY_ORIENTATION_INI	(OPTION_PRIORITY_DEBUG_INI + 1)
#define OPTION_PRIORITY_VECTOR_INI		(OPTION_PRIORITY_ORIENTATION_INI + 1)
#define OPTION_PRIORITY_SOURCE_INI		(OPTION_PRIORITY_VECTOR_INI + 1)
#define OPTION_PRIORITY_GPARENT_INI		(OPTION_PRIORITY_SOURCE_INI + 1)
#define OPTION_PRIORITY_PARENT_INI		(OPTION_PRIORITY_GPARENT_INI + 1)
#define OPTION_PRIORITY_DRIVER_INI		(OPTION_PRIORITY_PARENT_INI + 1)

/* core options */
#define OPTION_GAMENAME				OPTION_UNADORNED(0)

/* core configuration options */
#define OPTION_READCONFIG			"readconfig"
#define OPTION_WRITECONFIG			"writeconfig"

/* core state/playback options */
#define OPTION_STATE				"state"
#define OPTION_AUTOSAVE				"autosave"
#define OPTION_PLAYBACK				"playback"
#define OPTION_RECORD				"record"
#define OPTION_MNGWRITE				"mngwrite"
#define OPTION_AVIWRITE				"aviwrite"
#define OPTION_WAVWRITE				"wavwrite"
#define OPTION_SNAPNAME				"snapname"
#define OPTION_SNAPSIZE				"snapsize"
#define OPTION_SNAPVIEW				"snapview"
#define OPTION_BURNIN				"burnin"

/* core performance options */
#define OPTION_AUTOFRAMESKIP		"autoframeskip"
#define OPTION_FRAMESKIP			"frameskip"
#define OPTION_SECONDS_TO_RUN		"seconds_to_run"
#define OPTION_THROTTLE				"throttle"
#define OPTION_SLEEP				"sleep"
#define OPTION_SPEED				"speed"
#define OPTION_REFRESHSPEED			"refreshspeed"

/* core rotation options */
#define OPTION_ROTATE				"rotate"
#define OPTION_ROR					"ror"
#define OPTION_ROL					"rol"
#define OPTION_AUTOROR				"autoror"
#define OPTION_AUTOROL				"autorol"
#define OPTION_FLIPX				"flipx"
#define OPTION_FLIPY				"flipy"

/* core artwork options */
#define OPTION_ARTWORK_CROP			"artwork_crop"
#define OPTION_USE_BACKDROPS		"use_backdrops"
#define OPTION_USE_OVERLAYS			"use_overlays"
#define OPTION_USE_BEZELS			"use_bezels"

/* core screen options */
#define OPTION_BRIGHTNESS			"brightness"
#define OPTION_CONTRAST				"contrast"
#define OPTION_GAMMA				"gamma"
#define OPTION_PAUSE_BRIGHTNESS		"pause_brightness"

/* core vector options */
#define OPTION_ANTIALIAS			"antialias"
#define OPTION_BEAM					"beam"
#define OPTION_FLICKER				"flicker"

/* core sound options */
#define OPTION_SOUND				"sound"
#define OPTION_SAMPLERATE			"samplerate"
#define OPTION_SAMPLES				"samples"
#define OPTION_VOLUME				"volume"

/* core input options */
#define OPTION_COIN_LOCKOUT			"coin_lockout"
#define OPTION_CTRLR				"ctrlr"
#define OPTION_MOUSE				"mouse"
#define OPTION_JOYSTICK				"joystick"
#define OPTION_LIGHTGUN				"lightgun"
#define OPTION_MULTIKEYBOARD		"multikeyboard"
#define OPTION_MULTIMOUSE			"multimouse"
#define OPTION_PADDLE_DEVICE		"paddle_device"
#define OPTION_ADSTICK_DEVICE		"adstick_device"
#define OPTION_PEDAL_DEVICE			"pedal_device"
#define OPTION_DIAL_DEVICE			"dial_device"
#define OPTION_TRACKBALL_DEVICE		"trackball_device"
#define OPTION_LIGHTGUN_DEVICE		"lightgun_device"
#define OPTION_POSITIONAL_DEVICE	"positional_device"
#define OPTION_MOUSE_DEVICE			"mouse_device"
#define OPTION_JOYSTICK_MAP			"joystick_map"
#define OPTION_JOYSTICK_DEADZONE	"joystick_deadzone"
#define OPTION_JOYSTICK_SATURATION	"joystick_saturation"
#define OPTION_STEADYKEY			"steadykey"
#define OPTION_OFFSCREEN_RELOAD		"offscreen_reload"
#define OPTION_NATURAL_KEYBOARD		"natural"

/* core debugging options */
#define OPTION_VERBOSE				"verbose"
#define OPTION_LOG					"log"
#define OPTION_DEBUG				"debug"
#define OPTION_DEBUG_INTERNAL		"debug_internal"
#define OPTION_DEBUGSCRIPT			"debugscript"
#define OPTION_UPDATEINPAUSE		"update_in_pause"

/* core misc options */
#define OPTION_BIOS					"bios"
#define OPTION_CHEAT				"cheat"
#define OPTION_SKIP_GAMEINFO		"skip_gameinfo"
#define OPTION_SKIP_WARNINGS		"skip_warnings"
#define OPTION_SKIP_NAGSCREEN		"skip_nagscreen"

/* image device options */
#define OPTION_ADDED_DEVICE_OPTIONS	"added_device_options"


/***************************************************************************
    GLOBALS
***************************************************************************/

extern const options_entry mame_core_options[];



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* referenced types from other classes */
struct game_driver;


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

core_options *mame_options_init(const options_entry *entries);

/* add the device options for a specified device */
void image_add_device_options(core_options *opts, const game_driver *driver);
/* accesses a device option, by device and index */
const char *image_get_device_option(device_image_interface *image);

#endif	/* __EMUOPTS_H__ */
