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
void prep_retro_rotation(int rot);
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

struct kt_table
{
   const char  *   mame_key_name;
   int retro_key_name;
   input_item_id   mame_key;
};

// fake a keyboard mapped to retro joypad 
enum
{
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
	KEY_BUTTON_7,     
	KEY_JOYSTICK_U,
	KEY_JOYSTICK_D,
	KEY_JOYSTICK_L,
	KEY_JOYSTICK_R,
	KEY_TOTAL
};

const kt_table ktable[] = {
   {"A",        RETROK_a,           ITEM_ID_A},
   {"B",        RETROK_b,           ITEM_ID_B},
   {"C",        RETROK_c,           ITEM_ID_C},
   {"D",        RETROK_d,           ITEM_ID_D},
   {"E",        RETROK_e,           ITEM_ID_E},
   {"F",        RETROK_f,           ITEM_ID_F},
   {"G",        RETROK_g,           ITEM_ID_G},
   {"H",        RETROK_h,           ITEM_ID_H},
   {"I",        RETROK_i,           ITEM_ID_I},
   {"J",        RETROK_j,           ITEM_ID_J},
   {"K",        RETROK_k,           ITEM_ID_K},
   {"L",        RETROK_l,           ITEM_ID_L},
   {"M",        RETROK_m,           ITEM_ID_M},
   {"N",        RETROK_n,           ITEM_ID_N},
   {"O",        RETROK_o,           ITEM_ID_O},
   {"P",        RETROK_p,           ITEM_ID_P},
   {"Q",        RETROK_q,           ITEM_ID_Q},
   {"R",        RETROK_r,           ITEM_ID_R},
   {"S",        RETROK_s,           ITEM_ID_S},
   {"T",        RETROK_t,           ITEM_ID_T},
   {"U",        RETROK_u,           ITEM_ID_U},
   {"V",        RETROK_v,           ITEM_ID_V},
   {"W",        RETROK_w,           ITEM_ID_W},
   {"X",        RETROK_x,           ITEM_ID_X},
   {"Y",        RETROK_y,           ITEM_ID_Y},
   {"Z",        RETROK_z,           ITEM_ID_Z},
   {"0",        RETROK_0,           ITEM_ID_0},
   {"1",        RETROK_1,           ITEM_ID_1},
   {"2",        RETROK_2,           ITEM_ID_2},
   {"3",        RETROK_3,           ITEM_ID_3},
   {"4",        RETROK_4,           ITEM_ID_4},
   {"5",        RETROK_5,           ITEM_ID_5},
   {"6",        RETROK_6,           ITEM_ID_6},
   {"7",        RETROK_7,           ITEM_ID_7},
   {"8",        RETROK_8,           ITEM_ID_8},
   {"9",        RETROK_9,           ITEM_ID_9},
   {"F1",       RETROK_F1,          ITEM_ID_F1},
   {"F2",       RETROK_F2,          ITEM_ID_F2},
   {"F3",       RETROK_F3,          ITEM_ID_F3},
   {"F4",       RETROK_F4,          ITEM_ID_F4},
   {"F5",       RETROK_F5,          ITEM_ID_F5},
   {"F6",       RETROK_F6,          ITEM_ID_F6},
   {"F7",       RETROK_F7,          ITEM_ID_F7},
   {"F8",       RETROK_F8,          ITEM_ID_F8},
   {"F9",       RETROK_F9,          ITEM_ID_F9},
   {"F10",      RETROK_F10,         ITEM_ID_F10},
   {"F11",      RETROK_F11,         ITEM_ID_F11},
   {"F12",      RETROK_F12,         ITEM_ID_F12},
   {"F13",      RETROK_F13,         ITEM_ID_F13},
   {"F14",      RETROK_F14,         ITEM_ID_F14},
   {"F15",      RETROK_F15,         ITEM_ID_F15},
   {"Esc",      RETROK_ESCAPE,      ITEM_ID_ESC},
   {"TILDE",    RETROK_BACKQUOTE,   ITEM_ID_TILDE},
   {"MINUS",    RETROK_MINUS,       ITEM_ID_MINUS},
   {"EQUALS",   RETROK_EQUALS,      ITEM_ID_EQUALS},
   {"BKCSPACE", RETROK_BACKSPACE,   ITEM_ID_BACKSPACE},
   {"TAB",      RETROK_TAB,         ITEM_ID_TAB},
   {"(",        RETROK_LEFTPAREN,   ITEM_ID_OPENBRACE},
   {")",        RETROK_RIGHTPAREN,  ITEM_ID_CLOSEBRACE},
   {"ENTER",    RETROK_RETURN,      ITEM_ID_ENTER},
   {"·",        RETROK_COLON,       ITEM_ID_COLON},
   {"\'",       RETROK_QUOTE,       ITEM_ID_QUOTE},
   {"BCKSLASH", RETROK_BACKSLASH,   ITEM_ID_BACKSLASH},
   ///**/BCKSLASH2*/RETROK_, ITEM_ID_BACKSLASH2},
   {",",        RETROK_COMMA,       ITEM_ID_COMMA},
   ///**/STOP*/RETROK_, ITEM_ID_STOP},
   {"/",        RETROK_SLASH,       ITEM_ID_SLASH},
   {"SPACE",    RETROK_SPACE,       ITEM_ID_SPACE},
   {"INS",      RETROK_INSERT,      ITEM_ID_INSERT},
   {"DEL",      RETROK_DELETE,      ITEM_ID_DEL},
   {"HOME",     RETROK_HOME,        ITEM_ID_HOME},
   {"END",      RETROK_END,         ITEM_ID_END},
   {"PGUP",     RETROK_PAGEUP,      ITEM_ID_PGUP},
   {"PGDW",     RETROK_PAGEDOWN,    ITEM_ID_PGDN},
   {"LEFT",     RETROK_LEFT,        ITEM_ID_LEFT},
   {"RIGHT",    RETROK_RIGHT,       ITEM_ID_RIGHT},
   {"UP",       RETROK_UP,          ITEM_ID_UP},
   {"DOWN",     RETROK_DOWN,        ITEM_ID_DOWN},
   {"KO",       RETROK_KP0,         ITEM_ID_0_PAD},
   {"K1",       RETROK_KP1,         ITEM_ID_1_PAD},
   {"K2",       RETROK_KP2,         ITEM_ID_2_PAD},
   {"K3",       RETROK_KP3,         ITEM_ID_3_PAD},
   {"K4",       RETROK_KP4,         ITEM_ID_4_PAD},
   {"K5",       RETROK_KP5,         ITEM_ID_5_PAD},
   {"K6",       RETROK_KP6,         ITEM_ID_6_PAD},
   {"K7",       RETROK_KP7,         ITEM_ID_7_PAD},
   {"K8",       RETROK_KP8,         ITEM_ID_8_PAD},
   {"K9",       RETROK_KP9,         ITEM_ID_9_PAD},
   {"K/",       RETROK_KP_DIVIDE,   ITEM_ID_SLASH_PAD},
   {"K*",       RETROK_KP_MULTIPLY, ITEM_ID_ASTERISK},
   {"K-",       RETROK_KP_MINUS,    ITEM_ID_MINUS_PAD},
   {"K+",       RETROK_KP_PLUS,     ITEM_ID_PLUS_PAD},
   {"KDEL",     RETROK_KP_PERIOD,   ITEM_ID_DEL_PAD},
   {"KRTRN",    RETROK_KP_ENTER,    ITEM_ID_ENTER_PAD},
   {"PRINT",    RETROK_PRINT,       ITEM_ID_PRTSCR},
   {"PAUSE",    RETROK_PAUSE,       ITEM_ID_PAUSE},
   {"LSHFT",    RETROK_LSHIFT,      ITEM_ID_LSHIFT},
   {"RSHFT",    RETROK_RSHIFT,      ITEM_ID_RSHIFT},
   {"LCTRL",    RETROK_LCTRL,       ITEM_ID_LCONTROL},
   {"RCTRL",    RETROK_RCTRL,       ITEM_ID_RCONTROL},
   {"LALT",     RETROK_LALT,        ITEM_ID_LALT},
   {"RALT",     RETROK_RALT,        ITEM_ID_RALT},
   {"SCRLOCK",  RETROK_SCROLLOCK,   ITEM_ID_SCRLOCK},
   {"NUMLOCK",  RETROK_NUMLOCK,     ITEM_ID_NUMLOCK},
   {"CPSLOCK",  RETROK_CAPSLOCK,    ITEM_ID_CAPSLOCK},
   {"LMETA",    RETROK_LMETA,       ITEM_ID_LWIN},
   {"RMETA",    RETROK_RMETA,       ITEM_ID_RWIN},
   {"MENU",     RETROK_MENU,        ITEM_ID_MENU},
   {"BREAK",    RETROK_BREAK,       ITEM_ID_CANCEL},
   {"-1",       -1,                 ITEM_ID_INVALID},
};



#endif	/* __RETROMAIN_H__ */