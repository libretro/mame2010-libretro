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
	LX,
	LY,
	RX,
	RY,
	KEY_TOTAL
};

enum
{
   RETROPAD_B,
   RETROPAD_Y,
   RETROPAD_SELECT,
   RETROPAD_START,
   RETROPAD_PAD_UP,
   RETROPAD_PAD_DOWN,
   RETROPAD_PAD_LEFT,
   RETROPAD_PAD_RIGHT,
   RETROPAD_A,
   RETROPAD_X,
   RETROPAD_L,
   RETROPAD_R,
   RETROPAD_L2,
   RETROPAD_R2,
   RETROPAD_L3,
   RETROPAD_R3,
   RETROPAD_TOTAL
};

static const char *Buttons_Name[16]=
{
   "B",		//0
   "Y",		//1
   "SELECT",	//2
   "START",	//3
   "Pad UP",	//4
   "Pad DOWN",	//5
   "Pad LEFT",	//6
   "Pad RIGHT",	//7
   "A",		//8
   "X",		//9
   "L",		//10
   "R",		//11
   "L2",		//12
   "R2",		//13
   "L3",		//14
   "R3",		//15
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

/* Tekken 1/2 */
#define TEKKEN_LAYOUT	(core_stricmp(machine->gamedrv->name, "tekken") == 0) || (core_stricmp(machine->gamedrv->parent, "tekken") == 0) || \
         		(core_stricmp(machine->gamedrv->name, "tekken2") == 0) || (core_stricmp(machine->gamedrv->parent, "tekken2") == 0)

/* Soul Edge/Soul Calibur */
#define SOULEDGE_LAYOUT	(core_stricmp(machine->gamedrv->name, "souledge") == 0) || (core_stricmp(machine->gamedrv->parent, "souledge") == 0) || \
         		(core_stricmp(machine->gamedrv->name, "soulclbr") == 0) || (core_stricmp(machine->gamedrv->parent, "soulclbr") == 0)

/* Dead or Alive++ */
#define DOA_LAYOUT	(core_stricmp(machine->gamedrv->name, "doapp") == 0)

/* Virtua Fighter */
#define VF_LAYOUT	(core_stricmp(machine->gamedrv->name, "vf") == 0) || (core_stricmp(machine->gamedrv->parent, "vf") == 0)

/* Ehrgeiz */
#define EHRGEIZ_LAYOUT	(core_stricmp(machine->gamedrv->name, "ehrgeiz") == 0) || (core_stricmp(machine->gamedrv->parent, "ehrgeiz") == 0)

/* Toshinden 2 */
#define TS2_LAYOUT	(core_stricmp(machine->gamedrv->name, "ts2") == 0) || (core_stricmp(machine->gamedrv->parent, "ts2") == 0)

/* Capcom 6-button fighting games */
#define SF_LAYOUT	(core_stricmp(machine->gamedrv->name, "dstlk") == 0) || (core_stricmp(machine->gamedrv->parent, "dstlk") == 0) || (core_stricmp(machine->gamedrv->name, "hsf2") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "hsf2") == 0) || (core_stricmp(machine->gamedrv->name, "msh") == 0) || (core_stricmp(machine->gamedrv->parent, "msh") == 0) || \
         		(core_stricmp(machine->gamedrv->name, "mshvsf") == 0) || (core_stricmp(machine->gamedrv->parent, "mshvsf") == 0) || (core_stricmp(machine->gamedrv->name, "mvsc") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "mvsc") == 0) || (core_stricmp(machine->gamedrv->name, "nwarr") == 0) || (core_stricmp(machine->gamedrv->parent, "nwarr") == 0) || \
         		(core_stricmp(machine->gamedrv->name, "rvschool") == 0) || (core_stricmp(machine->gamedrv->parent, "rvschool") == 0) || (core_stricmp(machine->gamedrv->name, "sf2") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "sf2") == 0) || (core_stricmp(machine->gamedrv->name, "sf2ce") == 0) || (core_stricmp(machine->gamedrv->parent, "sf2ce") == 0) || \
         		(core_stricmp(machine->gamedrv->name, "sf2hf") == 0) || (core_stricmp(machine->gamedrv->parent, "sf2hf") == 0) || (core_stricmp(machine->gamedrv->name, "sfa") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "sfa") == 0) || (core_stricmp(machine->gamedrv->name, "sfa2") == 0) || (core_stricmp(machine->gamedrv->parent, "sfa2") == 0) || \
         		(core_stricmp(machine->gamedrv->name, "sfa3") == 0) || (core_stricmp(machine->gamedrv->parent, "sfa3") == 0) || (core_stricmp(machine->gamedrv->name, "sfex") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "sfex") == 0) || (core_stricmp(machine->gamedrv->name, "sfex2") == 0) || (core_stricmp(machine->gamedrv->parent, "sfex2") == 0) || \
         		(core_stricmp(machine->gamedrv->name, "sfex2p") == 0) || (core_stricmp(machine->gamedrv->parent, "sfex2p") == 0) || (core_stricmp(machine->gamedrv->name, "sfexp") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "sfexp") == 0) || (core_stricmp(machine->gamedrv->name, "sfiii") == 0) || (core_stricmp(machine->gamedrv->parent, "sfiii") == 0) || \
         		(core_stricmp(machine->gamedrv->name, "sfiii2") == 0) || (core_stricmp(machine->gamedrv->parent, "sfiii2") == 0) || (core_stricmp(machine->gamedrv->name, "sfiii3") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "sfiii3") == 0) || (core_stricmp(machine->gamedrv->name, "sftm") == 0) || (core_stricmp(machine->gamedrv->parent, "sftm") == 0) || \
         		(core_stricmp(machine->gamedrv->name, "ssf2") == 0) || (core_stricmp(machine->gamedrv->parent, "ssf2") == 0) || (core_stricmp(machine->gamedrv->name, "ssf2t") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "ssf2t") == 0) || (core_stricmp(machine->gamedrv->name, "starglad") == 0) || (core_stricmp(machine->gamedrv->parent, "starglad") == 0) || \
         		(core_stricmp(machine->gamedrv->name, "vsav") == 0) || (core_stricmp(machine->gamedrv->parent, "vsav") == 0) || (core_stricmp(machine->gamedrv->name, "vsav2") == 0) || \
			(core_stricmp(machine->gamedrv->parent, "vsav2") == 0) || (core_stricmp(machine->gamedrv->name, "xmcota") == 0) || (core_stricmp(machine->gamedrv->parent, "xmcota") == 0) || \
			(core_stricmp(machine->gamedrv->name, "xmvsf") == 0) || (core_stricmp(machine->gamedrv->parent, "xmvsf") == 0) || (core_stricmp(machine->gamedrv->name, "jojo") == 0) || \
			(core_stricmp(machine->gamedrv->parent, "jojo") == 0) || (core_stricmp(machine->gamedrv->name, "jojoba") == 0) || (core_stricmp(machine->gamedrv->parent, "jojoba") == 0) || \
			(core_stricmp(machine->gamedrv->name, "ringdest") == 0) || (core_stricmp(machine->gamedrv->parent, "ringdest") == 0) || (core_stricmp(machine->gamedrv->name, "sfz2al") == 0) || \
			(core_stricmp(machine->gamedrv->parent, "sfz2al") == 0) || (core_stricmp(machine->gamedrv->name, "vhunt2") == 0) || (core_stricmp(machine->gamedrv->parent, "vhunt2") == 0)

/* Neo Geo */
#define NEOGEO_LAYOUT	(core_stricmp(machine->gamedrv->parent, "aof") == 0) || (core_stricmp(machine->gamedrv->parent, "aof2") == 0) || (core_stricmp(machine->gamedrv->parent, "aof3") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "breakers") == 0) || (core_stricmp(machine->gamedrv->parent, "breakrev") == 0) || (core_stricmp(machine->gamedrv->parent, "doubledr") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "fatfursp") == 0) || (core_stricmp(machine->gamedrv->parent, "fatfury1") == 0) || (core_stricmp(machine->gamedrv->parent, "fatfury2") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "fatfury3") == 0) || (core_stricmp(machine->gamedrv->parent, "fightfev") == 0) || (core_stricmp(machine->gamedrv->parent, "galaxyfg") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "garou") == 0) || (core_stricmp(machine->gamedrv->parent, "gowcaizr") == 0) || (core_stricmp(machine->gamedrv->parent, "kabukikl") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "kizuna") == 0) || (core_stricmp(machine->gamedrv->parent, "kof94") == 0) || (core_stricmp(machine->gamedrv->parent, "kof95") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "kof96") == 0) || (core_stricmp(machine->gamedrv->parent, "kof97") == 0) || (core_stricmp(machine->gamedrv->parent, "kof98") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "kof99") == 0) || (core_stricmp(machine->gamedrv->parent, "kof2000") == 0) || (core_stricmp(machine->gamedrv->parent, "kof2001") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "kof2002") == 0) || (core_stricmp(machine->gamedrv->parent, "kof2003") == 0) || (core_stricmp(machine->gamedrv->parent, "lastblad") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "lastbld2") == 0) || (core_stricmp(machine->gamedrv->parent, "lresort") == 0) || (core_stricmp(machine->gamedrv->parent, "matrim") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "mslug") == 0) || (core_stricmp(machine->gamedrv->parent, "mslug2") == 0) || (core_stricmp(machine->gamedrv->parent, "mslug3") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "mslug4") == 0) || (core_stricmp(machine->gamedrv->parent, "mslug5") == 0) || (core_stricmp(machine->gamedrv->parent, "rbff1") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "mslugx") == 0) || (core_stricmp(machine->gamedrv->parent, "neogeo") == 0) ||(core_stricmp(machine->gamedrv->parent, "ninjamas") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "rbff2") == 0) || (core_stricmp(machine->gamedrv->parent, "rbffspec") == 0) || (core_stricmp(machine->gamedrv->parent, "rotd") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "samsh5sp") == 0) || (core_stricmp(machine->gamedrv->parent, "samsho") == 0) || (core_stricmp(machine->gamedrv->parent, "samsho2") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "samsho3") == 0) || (core_stricmp(machine->gamedrv->parent, "samsho4") == 0) || (core_stricmp(machine->gamedrv->parent, "samsho5") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "savagere") == 0) || (core_stricmp(machine->gamedrv->parent, "sengoku3") == 0) || (core_stricmp(machine->gamedrv->parent, "svc") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "viewpoin") == 0) || (core_stricmp(machine->gamedrv->parent, "wakuwak7") == 0) || (core_stricmp(machine->gamedrv->parent, "wh1") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "wh2") == 0) || (core_stricmp(machine->gamedrv->parent, "wh2j") == 0) || (core_stricmp(machine->gamedrv->parent, "whp") == 0) || \
       			(core_stricmp(machine->gamedrv->parent, "karnovr") == 0) || (core_stricmp(machine->gamedrv->parent, "aodk") == 0) || (core_stricmp(machine->gamedrv->parent, "kf2k3pcb") == 0) || \
			(core_stricmp(machine->gamedrv->parent, "svcpcb") == 0) || (core_stricmp(machine->gamedrv->parent, "ms5pcb") == 0)

/* Killer Instinct 1 */
#define KINST_LAYOUT	(core_stricmp(machine->gamedrv->name, "kinst") == 0) || (core_stricmp(machine->gamedrv->parent, "kinst") == 0)

/* Killer Instinct 2 */
#define KINST2_LAYOUT	(core_stricmp(machine->gamedrv->name, "kinst2") == 0) || (core_stricmp(machine->gamedrv->parent, "kinst2") == 0)

/* Tekken 3/Tekken Tag Tournament */
#define TEKKEN3_LAYOUT	(core_stricmp(machine->gamedrv->name, "tektagt") == 0) || (core_stricmp(machine->gamedrv->parent, "tektagt") == 0) || \
         		(core_stricmp(machine->gamedrv->name, "tekken3") == 0) || (core_stricmp(machine->gamedrv->parent, "tekken3") == 0)

/* Mortal Kombat 1/2/3/Ultimate/WWF: Wrestlemania */
#define MK_LAYOUT	(core_stricmp(machine->gamedrv->name, "mk") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "mk") == 0) || (core_stricmp(machine->gamedrv->name, "mk2") == 0) || \
         		(core_stricmp(machine->gamedrv->parent, "mk2") == 0) || (core_stricmp(machine->gamedrv->name, "mk3") == 0) || \
         		(core_stricmp(machine->gamedrv->name, "umk3") == 0) || (core_stricmp(machine->gamedrv->parent, "umk3") == 0) || \
         		(core_stricmp(machine->gamedrv->name, "wwfmania") == 0) || (core_stricmp(machine->gamedrv->parent, "wwfmania") == 0)

/* Capcom Eco Fighter , use L & R button to turn the weapon */
#define ECOFGT_LAYOUT	(core_stricmp(machine->gamedrv->name, "ecofghtr") == 0) || (core_stricmp(machine->gamedrv->parent, "ecofghtr") == 0)

#endif	/* __RETROMAIN_H__ */
