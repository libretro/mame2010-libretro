/***************************************************************************

retromain.h 

****************************************************************************/

#pragma once

#ifndef __RETROMAIN_H__
#define __RETROMAIN_H__

#include "options.h"
#include "osdepend.h"
#include "libretro.h"

#if !defined(HAVE_OPENGL) && !defined(HAVE_OPENGLES) && !defined(HAVE_RGB32)
   #define M16B
#endif

#ifdef M16B
   #define FUNC_PREFIX(x) rgb565_##x
   #define PIXEL_TYPE UINT16
   #define SRCSHIFT_R 3
   #define SRCSHIFT_G 2
   #define SRCSHIFT_B 3
   #define DSTSHIFT_R 11
   #define DSTSHIFT_G 5
   #define DSTSHIFT_B 0
#else
   #define FUNC_PREFIX(x) rgb888_##x
   #define PIXEL_TYPE UINT32
   #define SRCSHIFT_R 0
   #define SRCSHIFT_G 0
   #define SRCSHIFT_B 0
   #define DSTSHIFT_R 16
   #define DSTSHIFT_G 8
   #define DSTSHIFT_B 0
#endif

void osd_init(running_machine* machine);
void osd_update(running_machine* machine,int skip_redraw);
void osd_update_audio_stream(running_machine* machine,short *buffer, int samples_this_frame);
void osd_exit(running_machine &machine);

void retro_poll_mame_input();
void retro_init (void);
void check_variables(void);
void prep_retro_rotation(int rot);
void initInput(running_machine* machine);
void init_input_descriptors(void);

extern void retro_finish();
extern void retro_main_loop();

extern int osd_num_processors;

// use if you want to print something with the verbose flag
void CLIB_DECL mame_printf_verbose(const char *text, ...) ATTR_PRINTF(1,2);

extern int RLOOP;

extern const char* core_name;

extern unsigned use_external_hiscore;

extern char libretro_content_directory[];
extern char libretro_save_directory[];
extern char libretro_system_directory[];
extern char cheatpath[];
extern char samplepath[];
extern char artpath[];
extern char fontpath[];
extern char crosshairpath[];
extern char ctrlrpath[];
extern char inipath[];
extern char cfg_directory[];
extern char nvram_directory[];
extern char memcard_directory[];
extern char input_directory[];
extern char diff_directory[];
extern char hiscore_directory[];
extern char comment_directory[];

extern retro_log_printf_t retro_log;


// fake a keyboard mapped to retro joypad 
enum
{
	KEY_F11,
	KEY_TAB,
	KEY_F3,
	KEY_F2,
	KEY_START,
	KEY_COIN,
	KEY_BUTTON_1,
	KEY_BUTTON_2,
	KEY_BUTTON_3,
	KEY_BUTTON_4,
	KEY_BUTTON_5,
	KEY_BUTTON_6, 
	KEY_JOYSTICK_U,
	KEY_JOYSTICK_D,
	KEY_JOYSTICK_L,
	KEY_JOYSTICK_R,
	KEY_TOTAL
};

#ifdef DEBUG_LOG
# define LOG(msg) fprintf(stderr, "%s\n", msg)
#else
# define LOG(msg)
#endif

#endif	/* __RETROMAIN_H__ */