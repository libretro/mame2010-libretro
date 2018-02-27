/***************************************************************************
retromain.c 
mame2010 - libretro port of mame 0.139
****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "osdepend.h"

#include "emu.h"
#include "clifront.h"
#include "render.h"
#include "ui.h"
#include "uiinput.h"

#include "libretro.h" 
#include <file/file_path.h>
#include "retromain.h"

#include "log.h"
#include "rendersw.c"

#ifdef COMPILE_DATS
	#include "precompile_hiscore_dat.h"
    #include "precompile_mameini_boilerplate.h"
#else
	#include "hiscore_dat.h"
    #include "mameini_boilerplate.h"
#endif


#ifdef M16B
	uint16_t videoBuffer[1024*1024];
	#define PITCH 1
#else
	unsigned int videoBuffer[1024*1024];
	#define PITCH 1 * 2
#endif

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "retroogl.c"
#endif


const char* core_name = "mame2010";
char libretro_content_directory[1024];
char libretro_save_directory[1024];
char libretro_system_directory[1024];
char cheatpath[1024];
char samplepath[1024];
char artpath[1024];
char fontpath[1024];
char crosshairpath[1024];
char ctrlrpath[1024];
char inipath[1024];
char cfg_directory[1024];
char nvram_directory[1024];
char memcard_directory[1024];
char input_directory[1024];
char image_directory[1024];
char diff_directory[1024];
char comment_directory[1024];

int mame_reset = -1;
static int ui_ipt_pushchar=-1;

static bool mouse_enable = false;
static bool videoapproach1_enable = false;
bool hide_nagscreen = false;
bool hide_gameinfo = false;
bool hide_warnings = false;
//
static void update_geometry();
static unsigned int turbo_enable, turbo_state, turbo_delay = 5;
static bool set_par = false;
static double refresh_rate = 60.0;
static int set_frame_skip;
static unsigned sample_rate = 48000;
static unsigned adjust_opt[6] = {0/*Enable/Disable*/, 0/*Limit*/, 0/*GetRefreshRate*/, 0/*Brightness*/, 0/*Contrast*/, 0/*Gamma*/};
static float arroffset[3] = {0/*For brightness*/, 0/*For contrast*/, 0/*For gamma*/};

static int rtwi=320,rthe=240,topw=1024; // DEFAULT TEXW/TEXH/PITCH
int SHIFTON = -1;

extern "C" int mmain(int argc, const char *argv);
extern bool draw_this_frame;

retro_video_refresh_t video_cb = NULL;
retro_environment_t environ_cb = NULL;

retro_log_printf_t log_cb;

static retro_input_state_t input_state_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;

int RLOOP=1;

extern void retro_finish();
extern void retro_main_loop();

void retro_poll_mame_input();

size_t retro_serialize_size(void){ return 0; }
bool retro_serialize(void *data, size_t size){ return false; }
bool retro_unserialize(const void * data, size_t size){ return false; }

unsigned retro_get_region (void) {return RETRO_REGION_NTSC;}
void *retro_get_memory_data(unsigned type) {return 0;}
size_t retro_get_memory_size(unsigned type) {return 0;}
bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info){return false;}
void retro_cheat_reset(void){}
void retro_cheat_set(unsigned unused, bool unused1, const char* unused2){}
void retro_set_controller_port_device(unsigned in_port, unsigned device){}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
static retro_input_poll_t input_poll_cb;

void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }

void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { }

static void extract_directory(char *buf, const char *path, size_t size)
{
   strncpy(buf, path, size - 1);
   buf[size - 1] = '\0';

   char *base = strrchr(buf, '/');
   if (!base)
      base = strrchr(buf, '\\');

   if (base)
      *base = '\0';
   else
      buf[0] = '\0';
}


//============================================================
//  GLOBALS
//============================================================

// rendering target
static render_target *our_target = NULL;

// input device
static input_device *P1_device; // P1 JOYPAD
static input_device *P2_device; // P2 JOYPAD
static input_device *retrokbd_device; // KEYBD
static input_device *mouse_device;    // MOUSE

// state
static UINT8 P1_state[KEY_TOTAL];
static UINT8 P2_state[KEY_TOTAL];
static UINT16 retrokbd_state[RETROK_LAST];
static UINT16 retrokbd_state2[RETROK_LAST];

struct kt_table
{
   const char  *   mame_key_name;
   int retro_key_name;
   input_item_id   mame_key;
};
static int mouseLX,mouseLY;
static int mouseBUT[4];
//static int mouse_enabled;

int optButtonLayoutP1 = 0; //for player 1
int optButtonLayoutP2 = 0; //for player 2

//enables / disables tate mode
static int tate = 0;
static int screenRot = 0;
int vertical,orient;

static char MgamePath[1024];
static char MgameName[512];

static int FirstTimeUpdate = 1;

bool retro_load_ok  = false;
int pauseg = 0;

//============================================================
//  RETRO
//============================================================

void retro_set_environment(retro_environment_t cb)
{
   static const struct retro_variable vars[] = {
      { "mame_current_mouse_enable", "Mouse supported; disabled|enabled" },
      { "mame_current_videoapproach1_enable", "Video approach 1 Enabled; disabled|enabled" },
      { "mame_current_skip_nagscreen", "Hide nag screen; enabled|disabled" },
      { "mame_current_skip_gameinfo", "Hide game info screen; disabled|enabled" },
      { "mame_current_skip_warnings", "Hide warning screen; disabled|enabled" },
      { "mame_current_aspect_ratio", "Core provided aspect ratio; DAR|PAR" },
      { "mame_current_turbo_button", "Enable autofire; disabled|button 1|button 2|R2 to button 1 mapping|R2 to button 2 mapping" },
      { "mame_current_turbo_delay", "Set autofire pulse speed; medium|slow|fast" },
      { "mame_current_frame_skip", "Set frameskip; 0|1|2|3|4|automatic" },
      { "mame_current_sample_rate", "Set sample rate (Restart); 48000Hz|44100Hz|32000Hz|22050Hz" },
      { "mame_current_adj_brightness",
	"Set brightness; default|+1%|+2%|+3%|+4%|+5%|+6%|+7%|+8%|+9%|+10%|+11%|+12%|+13%|+14%|+15%|+16%|+17%|+18%|+19%|+20%|-20%|-19%|-18%|-17%|-16%|-15%|-14%|-13%|-12%|-11%|-10%|-9%|-8%|-7%|-6%|-5%|-4%|-3%|-2%|-1%" },
      { "mame_current_adj_contrast",
	"Set contrast; default|+1%|+2%|+3%|+4%|+5%|+6%|+7%|+8%|+9%|+10%|+11%|+12%|+13%|+14%|+15%|+16%|+17%|+18%|+19%|+20%|-20%|-19%|-18%|-17%|-16%|-15%|-14%|-13%|-12%|-11%|-10%|-9%|-8%|-7%|-6%|-5%|-4%|-3%|-2%|-1%" },
      { "mame_current_adj_gamma",
	"Set gamma; default|+1%|+2%|+3%|+4%|+5%|+6%|+7%|+8%|+9%|+10%|+11%|+12%|+13%|+14%|+15%|+16%|+17%|+18%|+19%|+20%|-20%|-19%|-18%|-17%|-16%|-15%|-14%|-13%|-12%|-11%|-10%|-9%|-8%|-7%|-6%|-5%|-4%|-3%|-2%|-1%" },

      { NULL, NULL },
   };

   environ_cb = cb;

   cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);
}

static void check_variables(void)
{
   struct retro_variable var = {0};
   bool tmp_ar = set_par;

   var.key = "mame_current_mouse_enable";
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      fprintf(stderr, "mouse_enable value: %s\n", var.value);
      if (!strcmp(var.value, "disabled"))
         mouse_enable = false;
      if (!strcmp(var.value, "enabled"))
         mouse_enable = true;
   }

   var.key = "mame_current_skip_nagscreen";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      fprintf(stderr, "skip_nagscreen value: %s\n", var.value);
      if (!strcmp(var.value, "disabled"))
         hide_nagscreen = false;
      if (!strcmp(var.value, "enabled"))
         hide_nagscreen = true;
   }

   var.key = "mame_current_skip_gameinfo";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      fprintf(stderr, "skip_gameinfo value: %s\n", var.value);
      if (!strcmp(var.value, "disabled"))
         hide_gameinfo = false;
      if (!strcmp(var.value, "enabled"))
         hide_gameinfo = true;
   }

   var.key = "mame_current_skip_warnings";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      fprintf(stderr, "skip_warnings value: %s\n", var.value);
      if (!strcmp(var.value, "disabled"))
         hide_warnings = false;
      if (!strcmp(var.value, "enabled"))
         hide_warnings = true;
   }

   var.key = "mame_current_videoapproach1_enable";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      fprintf(stderr, "videoapproach1_enable value: %s\n", var.value);
      if (!strcmp(var.value, "disabled"))
         videoapproach1_enable = false;
      if (!strcmp(var.value, "enabled"))
         videoapproach1_enable = true;
   }

   var.key = "mame_current_frame_skip";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
	int temp_fs = set_frame_skip;
	if (!strcmp(var.value, "automatic"))
		set_frame_skip = -1;
	else
		set_frame_skip = atoi(var.value);

	if (temp_fs != set_frame_skip)
		video_set_frameskip(set_frame_skip);
   }

   var.key = "mame_current_sample_rate";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
	sample_rate = atoi(var.value);

   var.key = "mame_current_turbo_button";
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      	if (!strcmp(var.value, "button 1"))
		turbo_enable = 1;
      	else if (!strcmp(var.value, "button 2"))
		turbo_enable = 2;
      	else if (!strcmp(var.value, "R2 to button 1 mapping"))
		turbo_enable = 3;
      	else if (!strcmp(var.value, "R2 to button 2 mapping"))
		turbo_enable = 4;
      	else
		turbo_enable = 0;
   }

   var.key = "mame_current_turbo_delay";
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      	if (!strcmp(var.value, "medium"))
		turbo_delay = 5;
      	else if (!strcmp(var.value, "slow"))
		turbo_delay = 7;
	else
		turbo_delay = 3;
   }

   var.key = "mame_current_aspect_ratio";
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      	if (!strcmp(var.value, "PAR"))
		set_par = true;
	else
		set_par = false;
   }

   var.key = "mame_current_adj_brightness";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
	float temp_value = arroffset[0];
	if (!strcmp(var.value, "default"))
		arroffset[0] = 0.0;
	else
		arroffset[0] = (float)atoi(var.value) / 100.0f;

	if (temp_value != arroffset[0])
		adjust_opt[0] = adjust_opt[3] = 1;
   }

   var.key = "mame_current_adj_contrast";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
	float temp_value = arroffset[1];
	if (!strcmp(var.value, "default"))
		arroffset[1] = 0.0;
	else
		arroffset[1] = (float)atoi(var.value) / 100.0f;

	if (temp_value != arroffset[1])
		adjust_opt[0] = adjust_opt[4] = 1;
   }

   var.key = "mame_current_adj_gamma";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
	float temp_value = arroffset[2];
	if (!strcmp(var.value, "default"))
		arroffset[2] = 0.0;
	else
		arroffset[2] = (float)atoi(var.value) / 100.0f;

	if (temp_value != arroffset[2])
		adjust_opt[0] = adjust_opt[5] = 1;
   }

   if (tmp_ar != set_par)
	update_geometry();
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

static void update_geometry()
{
   struct retro_system_av_info av_info;
   retro_get_system_av_info( &av_info);
   environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &av_info);
}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name = "MAME 2010";
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif
   info->library_version = "0.139" GIT_VERSION;
   info->valid_extensions = "zip|chd|7z";
   info->need_fullpath = true;
   info->block_extract = true;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   info->geometry.base_width   = rtwi;
   info->geometry.base_height  = rthe;

   info->geometry.max_width    = 1024;
   info->geometry.max_height   = 768;

   float display_ratio 	= set_par ? (vertical ? (float)rthe / (float)rtwi : (float)rtwi / (float)rthe) : (vertical ? 3.0f / 4.0f : 4.0f / 3.0f);
   info->geometry.aspect_ratio = display_ratio;

   info->timing.fps            = refresh_rate;
   info->timing.sample_rate    = (double)sample_rate;

#if 0	/* Test */
	int common_factor = 1;
	if (set_par)
	{
		int temp_width = rtwi;
		int temp_height = rthe;
		while (temp_width != temp_height)
		{
			if (temp_width > temp_height)
				temp_width -= temp_height;
			else
				temp_height -= temp_width;
		}
		common_factor = temp_height;
	}
	write_log("Current aspect ratio = %d : %d , screen refresh rate = %f , sound sample rate = %.1f \n", set_par ? vertical ? rthe / common_factor : rtwi / common_factor :
			vertical ? 3 : 4, set_par ? vertical ? rtwi / common_factor : rthe / common_factor : vertical ? 4 : 3, info->timing.fps, info->timing.sample_rate);
#endif
}

void retro_deinit(void)
{
   if(retro_load_ok)retro_finish();
   LOGI("Retro DeInit\n");
}

void retro_reset (void)
{
   mame_reset = 1;
}

void retro_run (void)
{
   bool updated = false;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
      check_variables();

	retro_poll_mame_input();
	retro_main_loop();

	RLOOP = 1;

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	do_gl2d();
#else
	if (draw_this_frame)
      		video_cb(videoBuffer,rtwi, rthe, topw << PITCH);
   	else
      		video_cb(NULL,rtwi, rthe, topw << PITCH);
#endif
   turbo_state > turbo_delay ? turbo_state = 0 : turbo_state++;
}

void prep_retro_rotation(int rot)
{
   LOGI("Rotation:%d\n",rot);
   environ_cb(RETRO_ENVIRONMENT_SET_ROTATION, &rot);
}

void retro_unload_game(void)
{
	if(pauseg == 0)
		pauseg = -1;

	LOGI("Retro unload_game\n");
}

void init_input_descriptors(void)
{
   #define describe_buttons(INDEX) \
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "Joystick Left" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Joystick Right" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "Joystick Up" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "Joystick Down" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "Button 1" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "Button 2" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "Button 3" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Button 4" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "Button 5" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "Button 6" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2,     "UI Menu" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2,     "Turbo Button" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3,     "Service" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3,     "Framerate" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Insert Coin" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

   struct retro_input_descriptor desc[] = {
      describe_buttons(0)
      describe_buttons(1)
      describe_buttons(2)
      describe_buttons(3)
      describe_buttons(4)
      describe_buttons(5)
      { 0, 0, 0, 0, NULL }
   };

   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
}

/*
static void keyboard_cb(bool down, unsigned keycode, uint32_t character, uint16_t mod)
{
#ifdef KEYDBG
   printf( "Down: %s, Code: %d, Char: %u, Mod: %u. \n",
         down ? "yes" : "no", keycode, character, mod);
#endif
   if (keycode>=320);
   else
   {
      if(down && keycode==RETROK_LSHIFT)
      {
         SHIFTON=-SHIFTON;					
         if(SHIFTON == 1)
            retrokbd_state[keycode]=1;
         else
            retrokbd_state[keycode]=0;	
      }
      else if(keycode!=RETROK_LSHIFT)
      {
         if (down)
            retrokbd_state[keycode]=1;	
         else if (!down)
            retrokbd_state[keycode]=0;
      }
   }
}
*/

#define PLAYER1_PRESS(button) input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_##button)
#define PLAYER2_PRESS(button) input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_##button)

kt_table ktable[] = {
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

static INT32 pad1_get_state(void *device_internal, void *item_internal)
{
   UINT8 *itemdata = (UINT8 *)item_internal;
   return *itemdata;
}

static INT32 pad2_get_state(void *device_internal, void *item_internal)
{
   UINT8 *itemdata = (UINT8 *)item_internal;
   return *itemdata;
}

static INT32 retrokbd_get_state(void *device_internal, void *item_internal)
{
   UINT8 *itemdata = (UINT8 *)item_internal;
   return *itemdata;
}

static INT32 generic_axis_get_state(void *device_internal, void *item_internal)
{
   INT32 *axisdata = (INT32 *)item_internal;
   return *axisdata;
}


static INT32 generic_button_get_state(void *device_internal, void *item_internal)
{
   INT32 *itemdata = (INT32 *)item_internal;
   return *itemdata >> 7;
}

#define input_device_item_add_mouse(a,b,c,d,e) input_device_item_add(a,b,c,d,e)

#define input_device_item_add_p1(a,b,c,d,e) input_device_item_add(a,b,c,d,e)

#define input_device_item_add_p2(a,b,c,d,e) input_device_item_add(a,b,c,d,e)

#define input_device_item_add_kbd(a,b,c,d,e) input_device_item_add(a,b,c,d,e)

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

static void initInput(running_machine* machine)
{
   int i,button;
   char defname[20];

   if (mouse_enable)
   {

      mouse_device = input_device_add(machine, DEVICE_CLASS_MOUSE, "Mice1", NULL);
      // add the axes
      input_device_item_add_mouse(mouse_device, "X", &mouseLX, ITEM_ID_XAXIS, generic_axis_get_state);
      input_device_item_add_mouse(mouse_device, "Y", &mouseLY, ITEM_ID_YAXIS, generic_axis_get_state);

      for (button = 0; button < 4; button++)
      {
         input_item_id itemid = (input_item_id) (ITEM_ID_BUTTON1 + button);
         sprintf(defname, "B%d", button + 1);

         input_device_item_add_mouse(mouse_device, defname, &mouseBUT[button], itemid, generic_button_get_state);
      }
   }

   P1_device = input_device_add(machine, DEVICE_CLASS_KEYBOARD, "Pad1", NULL);
   P2_device = input_device_add(machine, DEVICE_CLASS_KEYBOARD, "Pad2", NULL);

   if (P1_device == NULL)
      fatalerror("P1 Error creating keyboard device\n");

   if (P2_device == NULL)
      fatalerror("P2 Error creating keyboard device\n");

   // our faux keyboard only has a couple of keys (corresponding to the
   // common defaults)
   fprintf(stderr, "SOURCE FILE: %s\n", machine->gamedrv->source_file);
   fprintf(stderr, "PARENT: %s\n", machine->gamedrv->parent);
   fprintf(stderr, "NAME: %s\n", machine->gamedrv->name);
   fprintf(stderr, "DESCRIPTION: %s\n", machine->gamedrv->description);
   fprintf(stderr, "YEAR: %s\n", machine->gamedrv->year);
   fprintf(stderr, "MANUFACTURER: %s\n", machine->gamedrv->manufacturer);

   P1_state[KEY_F11]        = 0;/*RETRO_DEVICE_ID_JOYPAD_R3*/
   P1_state[KEY_TAB]        = 0;//RETRO_DEVICE_ID_JOYPAD_L2
   P1_state[KEY_F2]         = 0;//RETRO_DEVICE_ID_JOYPAD_L3
   P1_state[KEY_START]      = 0;//RETRO_DEVICE_ID_JOYPAD_START
   P1_state[KEY_COIN]       = 0;//RETRO_DEVICE_ID_JOYPAD_SELECT

   P1_state[KEY_BUTTON_1]   = 0;//RETRO_DEVICE_ID_JOYPAD_A
   P1_state[KEY_BUTTON_2]   = 0;//RETRO_DEVICE_ID_JOYPAD_B
   P1_state[KEY_BUTTON_3]   = 0;//RETRO_DEVICE_ID_JOYPAD_X
   P1_state[KEY_BUTTON_4]   = 0;//RETRO_DEVICE_ID_JOYPAD_Y
   P1_state[KEY_BUTTON_5]   = 0;//RETRO_DEVICE_ID_JOYPAD_L
   P1_state[KEY_BUTTON_6]   = 0;//RETRO_DEVICE_ID_JOYPAD_R
   P1_state[KEY_JOYSTICK_U] = 0;//RETRO_DEVICE_ID_JOYPAD_UP
   P1_state[KEY_JOYSTICK_D] = 0;//RETRO_DEVICE_ID_JOYPAD_DOWN
   P1_state[KEY_JOYSTICK_L] = 0;//RETRO_DEVICE_ID_JOYPAD_LEFT
   P1_state[KEY_JOYSTICK_R] = 0;//RETRO_DEVICE_ID_JOYPAD_RIGHT

   P2_state[KEY_F11]        = 0;/*RETRO_DEVICE_ID_JOYPAD_R3*/
   P2_state[KEY_TAB]        = 0;//RETRO_DEVICE_ID_JOYPAD_L2
   P2_state[KEY_F2]         = 0;//RETRO_DEVICE_ID_JOYPAD_L3
   P2_state[KEY_START]      = 0;//RETRO_DEVICE_ID_JOYPAD_START
   P2_state[KEY_COIN]       = 0;//RETRO_DEVICE_ID_JOYPAD_SELECT

   P2_state[KEY_BUTTON_1]   = 0;//RETRO_DEVICE_ID_JOYPAD_A
   P2_state[KEY_BUTTON_2]   = 0;//RETRO_DEVICE_ID_JOYPAD_B
   P2_state[KEY_BUTTON_3]   = 0;//RETRO_DEVICE_ID_JOYPAD_X
   P2_state[KEY_BUTTON_4]   = 0;//RETRO_DEVICE_ID_JOYPAD_Y
   P2_state[KEY_BUTTON_5]   = 0;//RETRO_DEVICE_ID_JOYPAD_L
   P2_state[KEY_BUTTON_6]   = 0;//RETRO_DEVICE_ID_JOYPAD_R
   P2_state[KEY_JOYSTICK_U] = 0;//RETRO_DEVICE_ID_JOYPAD_UP
   P2_state[KEY_JOYSTICK_D] = 0;//RETRO_DEVICE_ID_JOYPAD_DOWN
   P2_state[KEY_JOYSTICK_L] = 0;//RETRO_DEVICE_ID_JOYPAD_LEFT
   P2_state[KEY_JOYSTICK_R] = 0;//RETRO_DEVICE_ID_JOYPAD_RIGHT
#ifdef WIIU
   //FIXME: HACK WIIU GUI CONFIRM (ENTER) MAP TO R3
   input_device_item_add_p2(P2_device, "F11", &P2_state[KEY_F11], ITEM_ID_ENTER/* ITEM_ID_F11*/, pad2_get_state);
#else
   input_device_item_add_p2(P2_device, "F11", &P2_state[KEY_F11], ITEM_ID_F11, pad2_get_state);
#endif
   input_device_item_add_p2(P2_device, "Tab", &P2_state[KEY_TAB], ITEM_ID_TAB, pad2_get_state);
   input_device_item_add_p2(P2_device, "F3", &P2_state[KEY_F3], ITEM_ID_F3, pad2_get_state);
   input_device_item_add_p2(P2_device, "F2", &P2_state[KEY_F2], ITEM_ID_F2, pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 Start", &P2_state[KEY_START], ITEM_ID_2, pad2_get_state);
   input_device_item_add_p2(P2_device, "COIN2", &P2_state[KEY_COIN], ITEM_ID_6, pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 JoyU", &P2_state[KEY_JOYSTICK_U], ITEM_ID_R, pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 JoyD", &P2_state[KEY_JOYSTICK_D], ITEM_ID_F, pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 JoyL", &P2_state[KEY_JOYSTICK_L], ITEM_ID_D, pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 JoyR", &P2_state[KEY_JOYSTICK_R], ITEM_ID_G, pad2_get_state);
#ifdef WIIU
   //FIXME: HACK WIIU GUI CONFIRM (ENTER) MAP TO R3
   input_device_item_add_p1(P1_device, "F11", &P1_state[KEY_F11], ITEM_ID_ENTER/* ITEM_ID_F11*/, pad1_get_state);
#else
   input_device_item_add_p1(P1_device, "F11", &P1_state[KEY_F11], ITEM_ID_F11, pad1_get_state);
#endif
   input_device_item_add_p1(P1_device, "Tab", &P1_state[KEY_TAB], ITEM_ID_TAB, pad1_get_state);
   input_device_item_add_p1(P1_device, "F3", &P1_state[KEY_F3], ITEM_ID_F3, pad1_get_state);
   input_device_item_add_p1(P1_device, "F2", &P1_state[KEY_F2], ITEM_ID_F2, pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 Start", &P1_state[KEY_START], ITEM_ID_1, pad1_get_state);
   input_device_item_add_p1(P1_device, "COIN1", &P1_state[KEY_COIN], ITEM_ID_5, pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 JoyU", &P1_state[KEY_JOYSTICK_U], ITEM_ID_UP, pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 JoyD", &P1_state[KEY_JOYSTICK_D], ITEM_ID_DOWN, pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 JoyL", &P1_state[KEY_JOYSTICK_L], ITEM_ID_LEFT, pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 JoyR", &P1_state[KEY_JOYSTICK_R], ITEM_ID_RIGHT, pad1_get_state);

   if (TEKKEN_LAYOUT)	/* Tekken 1/2 */
   {
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_LSHIFT, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_W, pad2_get_state);
   }
   else      /* Soul Edge / Soul Calibur */
   if (SOULEDGE_LAYOUT)
   {
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LSHIFT, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_W, pad2_get_state);
   }
   else      /* Dead or Alive++ */
   if (DOA_LAYOUT)
   {
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_SPACE, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_Q, pad2_get_state);
   }
   else      /* Virtua Fighter */
   if (VF_LAYOUT)
   {
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_SPACE, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_Q, pad2_get_state);
   }
   else      /* Ehrgeiz */
   if (EHRGEIZ_LAYOUT)
   {
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_SPACE, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_Q, pad2_get_state);
   }
   else      /* Toshinden 2 */
   if (TS2_LAYOUT)
   {
      input_device_item_add_p1(P1_device, "RetroPad P1 L", &P1_state[KEY_BUTTON_5], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 R", &P1_state[KEY_BUTTON_6], ITEM_ID_LSHIFT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_Z, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_X, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 L", &P2_state[KEY_BUTTON_5], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 R", &P2_state[KEY_BUTTON_6], ITEM_ID_W, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_I, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_K, pad2_get_state);
   }
   else      /* Capcom 6-button fighting games */
   if (SF_LAYOUT)
   {
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 L", &P1_state[KEY_BUTTON_5], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LSHIFT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_Z, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 R", &P1_state[KEY_BUTTON_6], ITEM_ID_X, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 L", &P2_state[KEY_BUTTON_5], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_W, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_I, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 R", &P2_state[KEY_BUTTON_6], ITEM_ID_K, pad2_get_state);
   }
   else      /* Neo Geo */
   if (NEOGEO_LAYOUT)
   {
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_LSHIFT, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_W, pad2_get_state);
   }
   else      /* Killer Instinct 1 */
   if (KINST_LAYOUT)
   {
      input_device_item_add_p1(P1_device, "RetroPad P1 L", &P1_state[KEY_BUTTON_5], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 R", &P1_state[KEY_BUTTON_6], ITEM_ID_LSHIFT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_Z, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_X, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 L", &P2_state[KEY_BUTTON_5], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 R", &P2_state[KEY_BUTTON_6], ITEM_ID_W, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_I, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_K, pad2_get_state);
   }
   else      /* Killer Instinct 2 */
   if (KINST2_LAYOUT)
   {
      input_device_item_add_p1(P1_device, "RetroPad P1 L", &P1_state[KEY_BUTTON_5], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_6], ITEM_ID_LSHIFT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_2], ITEM_ID_Z, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 R", &P1_state[KEY_BUTTON_1], ITEM_ID_X, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 L", &P2_state[KEY_BUTTON_5], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_6], ITEM_ID_W, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_2], ITEM_ID_I, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 R", &P2_state[KEY_BUTTON_1], ITEM_ID_K, pad2_get_state);
   }
   else      /* Tekken 3 / Tekken Tag Tournament */
   if (TEKKEN3_LAYOUT)
   {
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 R", &P1_state[KEY_BUTTON_6], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LSHIFT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_Z, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 R", &P2_state[KEY_BUTTON_6], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_W, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_I, pad2_get_state);
   }
   else      /* Mortal Kombat 1/2/3 / Ultimate/WWF: Wrestlemania */
   if (MK_LAYOUT)
   {
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 L", &P1_state[KEY_BUTTON_5], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LSHIFT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_Z, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 R", &P1_state[KEY_BUTTON_6], ITEM_ID_X, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 L", &P2_state[KEY_BUTTON_5], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_W, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_I, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 R", &P2_state[KEY_BUTTON_6], ITEM_ID_K, pad2_get_state);
   }
   else	     /* Capcom Eco Fighters */
   if (ECOFGT_LAYOUT)
   {
      input_device_item_add_p1(P1_device, "P1 B1", &P1_state[KEY_BUTTON_5], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "P1 B2", &P1_state[KEY_BUTTON_2], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "P1 B3", &P1_state[KEY_BUTTON_6], ITEM_ID_SPACE, pad1_get_state);

      input_device_item_add_p2(P2_device, "P2 B1", &P2_state[KEY_BUTTON_5], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "P2 B2", &P2_state[KEY_BUTTON_2], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "P2 B3", &P2_state[KEY_BUTTON_6], ITEM_ID_Q, pad2_get_state);
   }
   else      /* Default config */
   {
      input_device_item_add_p1(P1_device, "P1 B1", &P1_state[KEY_BUTTON_1], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "P1 B2", &P1_state[KEY_BUTTON_2], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "P1 B3", &P1_state[KEY_BUTTON_3], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "P1 B4", &P1_state[KEY_BUTTON_4], ITEM_ID_LSHIFT, pad1_get_state);
      input_device_item_add_p1(P1_device, "P1 B5", &P1_state[KEY_BUTTON_5], ITEM_ID_Z, pad1_get_state);
      input_device_item_add_p1(P1_device, "P1 B6", &P1_state[KEY_BUTTON_6], ITEM_ID_X, pad1_get_state);

      input_device_item_add_p2(P2_device, "P2 B1", &P2_state[KEY_BUTTON_1], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "P2 B2", &P2_state[KEY_BUTTON_2], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "P2 B3", &P2_state[KEY_BUTTON_3], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "P2 B4", &P2_state[KEY_BUTTON_4], ITEM_ID_W, pad2_get_state);
      input_device_item_add_p2(P2_device, "P2 B5", &P2_state[KEY_BUTTON_5], ITEM_ID_I, pad2_get_state);
      input_device_item_add_p2(P2_device, "P2 B6", &P2_state[KEY_BUTTON_6], ITEM_ID_K, pad2_get_state);
   }

   retrokbd_device = input_device_add(machine, DEVICE_CLASS_KEYBOARD, "Retrokdb", NULL);

   if (retrokbd_device == NULL)
      fatalerror("KBD Error creating keyboard device\n");

   for (i = 0; i < RETROK_LAST; i++)
   {
      retrokbd_state[i] = 0;
      retrokbd_state2[i] = 0;
   }

   i = 0;
   do
   {
      input_device_item_add_kbd(retrokbd_device,\
            ktable[i].mame_key_name, &retrokbd_state[ktable[i].retro_key_name], ktable[i].mame_key, retrokbd_get_state);
      i++;
   } while (ktable[i].retro_key_name != -1);

}
#undef TEKKEN_LAYOUT
#undef SOULEDGE_LAYOUT
#undef DOA_LAYOUT
#undef VF_LAYOUT
#undef EHRGEIZ_LAYOUT
#undef TS2_LAYOUT
#undef SF_LAYOUT
#undef NEOGEO_LAYOUT
#undef KINST_LAYOUT
#undef KINST2_LAYOUT
#undef TEKKEN3_LAYOUT
#undef MK_LAYOUT
#undef ECOFGT_LAYOUT

void retro_poll_mame_input()
{
   input_poll_cb();

   // process_keyboard_state

   /* TODO: handle mods:SHIFT/CTRL/ALT/META/NUMLOCK/CAPSLOCK/SCROLLOCK */
   unsigned i = 0;
   do
   {
      retrokbd_state[ktable[i].retro_key_name] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, ktable[i].retro_key_name) ? 0x80 : 0;

      if (retrokbd_state[ktable[i].retro_key_name] && retrokbd_state2[ktable[i].retro_key_name] == 0)
      {
         ui_ipt_pushchar = ktable[i].retro_key_name;
         retrokbd_state2[ktable[i].retro_key_name] = 1;
      }
      else if (!retrokbd_state[ktable[i].retro_key_name] && retrokbd_state2[ktable[i].retro_key_name] == 1)
         retrokbd_state2[ktable[i].retro_key_name] = 0;

      i++;
   } while (ktable[i].retro_key_name != -1);

   if (mouse_enable)
   {
      static int mbL = 0, mbR = 0;
      int mouse_l;
      int mouse_r;
      int16_t mouse_x;
      int16_t mouse_y;

      mouse_x = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
      mouse_y = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);
      mouse_l = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT);
      mouse_r = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT);
      mouseLX = mouse_x*INPUT_RELATIVE_PER_PIXEL;;
      mouseLY = mouse_y*INPUT_RELATIVE_PER_PIXEL;;

      if (mbL == 0 && mouse_l)
      {
         mbL = 1;
         mouseBUT[0] = 0x80;
      }
      else if (mbL == 1 && !mouse_l)
      {
         mouseBUT[0] = 0;
         mbL = 0;
      }

      if (mbR == 0 && mouse_r)
      {
         mbR = 1;
         mouseBUT[1] = 0x80;
      }
      else if(mbR == 1 && !mouse_r)
      {
         mouseBUT[1] = 0;
         mbR = 0;
      }
   }

   P1_state[KEY_F11]        = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3);	/* change : remap F11 to R3,the R2 button becomes available */
   P1_state[KEY_TAB]        = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2);
   P1_state[KEY_F2] 	    = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3);
   P1_state[KEY_START]      = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START);
   P1_state[KEY_COIN]       = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT);
   P1_state[KEY_BUTTON_1]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A);
   P1_state[KEY_BUTTON_2]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B);
   P1_state[KEY_BUTTON_3]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X);
   P1_state[KEY_BUTTON_4]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);
   P1_state[KEY_BUTTON_5]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
   P1_state[KEY_BUTTON_6]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);
   P1_state[KEY_JOYSTICK_U] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
   P1_state[KEY_JOYSTICK_D] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
   P1_state[KEY_JOYSTICK_L] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
   P1_state[KEY_JOYSTICK_R] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);

   P2_state[KEY_F11]        = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3);
   P2_state[KEY_TAB]        = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2);
   P2_state[KEY_F2] 	    = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3);
   P2_state[KEY_START]      = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START);
   P2_state[KEY_COIN]       = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT);
   P2_state[KEY_BUTTON_1]   = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A);
   P2_state[KEY_BUTTON_2]   = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B);
   P2_state[KEY_BUTTON_3]   = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X);
   P2_state[KEY_BUTTON_4]   = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);
   P2_state[KEY_BUTTON_5]   = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
   P2_state[KEY_BUTTON_6]   = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);
   P2_state[KEY_JOYSTICK_U] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
   P2_state[KEY_JOYSTICK_D] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
   P2_state[KEY_JOYSTICK_L] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
   P2_state[KEY_JOYSTICK_R] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);

   switch (turbo_enable)
   {
      case 0:
         break;
      case 1:
         if (PLAYER1_PRESS(A))
            P1_state[KEY_BUTTON_1] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER2_PRESS(A))
            P2_state[KEY_BUTTON_1] = turbo_state < turbo_delay ? 0 : 1;
         break;
      case 2:
         if (PLAYER1_PRESS(B))
            P1_state[KEY_BUTTON_2] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER2_PRESS(B))
            P2_state[KEY_BUTTON_2] = turbo_state < turbo_delay ? 0 : 1;
         break;
      case 3:
         if (PLAYER1_PRESS(R2))
            P1_state[KEY_BUTTON_1] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER2_PRESS(R2))
            P2_state[KEY_BUTTON_1] = turbo_state < turbo_delay ? 0 : 1;
         break;
      case 4:
         if (PLAYER1_PRESS(R2))
            P1_state[KEY_BUTTON_2] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER2_PRESS(R2))
            P2_state[KEY_BUTTON_2] = turbo_state < turbo_delay ? 0 : 1;
         break;
   }
}

static bool path_file_exists(const char *path)
{
   FILE *dummy;

   if (!path || !*path)
      return false;

   dummy = fopen(path, "rb");

   if (!dummy)
      return false;

   fclose(dummy);
   return true;
}

void retro_init (void)
{      
   const char *system_dir  = NULL;
   const char *save_dir    = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &system_dir) && system_dir)
   {
       // use a subfolder in the system directory with the core name (ie mame2010)
        snprintf(libretro_system_directory, sizeof(libretro_system_directory), "%s%s%s", system_dir, path_default_slash(), core_name);
   }

   if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &save_dir) && save_dir)
   {
       // use a subfolder in the save directory with the core name (ie mame2010)
        snprintf(libretro_save_directory, sizeof(libretro_save_directory), "%s%s%s", save_dir, path_default_slash(), core_name);
   }
   else
   {
        *libretro_save_directory = *libretro_system_directory;
   }
   
    path_mkdir(libretro_system_directory);
    path_mkdir(libretro_save_directory);
 
    // content loaded from mame2010 subfolder within the libretro system folder
    snprintf(cheatpath, sizeof(cheatpath), "%s%s", path_default_slash(), libretro_system_directory);
    path_mkdir(libretro_save_directory);
    snprintf(samplepath, sizeof(samplepath), "%s%s%s", libretro_system_directory, path_default_slash(), "samples");
    path_mkdir(samplepath);
    snprintf(artpath, sizeof(artpath), "%s%s%s", libretro_system_directory, path_default_slash(), "artwork");
    path_mkdir(artpath);
    snprintf(fontpath, sizeof(fontpath), "%s%s%s", libretro_system_directory, path_default_slash(), "fonts");
    path_mkdir(fontpath);
    snprintf(crosshairpath, sizeof(crosshairpath), "%s%s%s", libretro_system_directory, path_default_slash(), "crosshairs");
    path_mkdir(crosshairpath);

    // user-generated content loaded from mame2010 subfolder within the libretro save folder
    snprintf(ctrlrpath, sizeof(ctrlrpath), "%s%s%s", libretro_save_directory, path_default_slash(), "ctrlr");
    path_mkdir(ctrlrpath);
    snprintf(inipath, sizeof(inipath), "%s%s%s", libretro_save_directory, path_default_slash(), "ini");
    path_mkdir(inipath);
    snprintf(cfg_directory, sizeof(cfg_directory), "%s%s%s", libretro_save_directory, path_default_slash(), "cfg");
    path_mkdir(cfg_directory);
    snprintf(nvram_directory, sizeof(nvram_directory), "%s%s%s", libretro_save_directory, path_default_slash(), "nvram");
    path_mkdir(nvram_directory);
    snprintf(memcard_directory, sizeof(memcard_directory), "%s%s%s", libretro_save_directory, path_default_slash(), "memcard");
    path_mkdir(memcard_directory);
    snprintf(input_directory, sizeof(input_directory), "%s%s%s", libretro_save_directory, path_default_slash(), "input");
    path_mkdir(input_directory);
    snprintf(image_directory, sizeof(image_directory), "%s%s%s", libretro_save_directory, path_default_slash(), "image");
    path_mkdir(image_directory);
	snprintf(diff_directory, sizeof(diff_directory), "%s%s%s", libretro_save_directory, path_default_slash(), "diff");
    path_mkdir(diff_directory);
    snprintf(comment_directory, sizeof(comment_directory), "%s%s%s", libretro_save_directory, path_default_slash(), "comment");
    path_mkdir(comment_directory);

    char mameini_path[1024];
    
    snprintf(mameini_path, sizeof(mameini_path), "%s%s%s", inipath, path_default_slash(), "mame.ini");
    if(!path_file_exists(mameini_path))
    {
        FILE *mameini_file;
        if((mameini_file=fopen(mameini_path, "wb"))==NULL)
        {
            printf("[ERROR][MAME 2010] Something went wrong creating new mame.ini at: %s\n", mameini_path);
        }
        else
        {
            fwrite(mameini_boilerplate, sizeof(char), mameini_boilerplate_length, mameini_file);          
            fclose(mameini_file);         
        }
    }
}

bool retro_load_game(const struct retro_game_info *info)
{
   extract_directory(libretro_content_directory, info->path, sizeof(libretro_content_directory));
   strncpy(libretro_content_directory, info->path, sizeof(libretro_content_directory));
	
   struct retro_log_callback log_cb;
   
   printf("[INFO][MAME 2010] mame2010 system directory: %s\n", libretro_system_directory);
   printf("[INFO][MAME 2010] mame2010 content directory: %s\n", libretro_content_directory);
   printf("[INFO][MAME 2010] mame2010 save directory: %s\n", libretro_save_directory); 
   
#if 0
   struct retro_keyboard_callback cb = { keyboard_cb };
   environ_cb(RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK, &cb);
#endif

#ifdef M16B
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
#else
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
#endif

   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      fprintf(stderr, "[ERROR][MAME 2010] RGB pixel format is not supported.\n");
      exit(0);
   }
   check_variables();

#ifdef M16B
   memset(videoBuffer, 0, 1024*1024*2);
#else
   memset(videoBuffer, 0, 1024*1024*2*2);
#endif

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#ifdef HAVE_OPENGLES
   hw_render.context_type = RETRO_HW_CONTEXT_OPENGLES2;
#else
   hw_render.context_type = RETRO_HW_CONTEXT_OPENGL;
#endif
   hw_render.context_reset = context_reset;
   hw_render.context_destroy = context_destroy;

   if (!environ_cb(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render))
      return false;
#endif

   init_input_descriptors();
   
   if(mmain(1,info->path)!=1){ // path the romset path to the mmain function to start emulation
        printf("[ERROR][MAME2010] MAME returned an error!\n");
		return 0;
   } 

   retro_load_ok  = true;
   video_set_frameskip(set_frame_skip);

   for (int i = 0; i < 6; i++)
	adjust_opt[i] = 1;

   return 1;
}

void osd_exit(running_machine &machine)
{
   write_log("osd_exit called \n");

   if (our_target != NULL)
      render_target_free(our_target);
   our_target = NULL;

   global_free(P1_device);
   global_free(P2_device);
   global_free(retrokbd_device);
   global_free(mouse_device);
}

void osd_init(running_machine* machine)
{
   int gamRot=0;

   machine->add_notifier(MACHINE_NOTIFY_EXIT, osd_exit);

   our_target = render_target_alloc(machine,NULL, 0);

   initInput(machine);

   write_log("[INFO][MAME2010] Machine screen orientation: %s \n",
         (machine->gamedrv->flags & ORIENTATION_SWAP_XY) ? "VERTICAL" : "HORIZONTAL"
         );

   orient = (machine->gamedrv->flags & ORIENTATION_MASK);
   vertical = (machine->gamedrv->flags & ORIENTATION_SWAP_XY);

   gamRot = (ROT270 == orient) ? 1 : gamRot;
   gamRot = (ROT180 == orient) ? 2 : gamRot;
   gamRot = (ROT90 == orient) ? 3 : gamRot;

   prep_retro_rotation(gamRot);
   machine->sample_rate = sample_rate;	/* Override original value */

   write_log("[INFO][MAME2010] OSD init done\n");
}

bool draw_this_frame;

void osd_update(running_machine *machine,int skip_redraw)
{
   const render_primitive_list *primlist;
   UINT8 *surfptr;

   if (mame_reset == 1)
   {
      machine->schedule_soft_reset();
      mame_reset = -1;
   }

   if(pauseg==-1){
      machine->schedule_exit();
      return;
   }

   if (FirstTimeUpdate == 1)
      skip_redraw = 0; //force redraw to make sure the video texture is created

   if (!skip_redraw)
   {

      draw_this_frame = true;
      // get the minimum width/height for the current layout
      int minwidth, minheight;

      if(videoapproach1_enable==false){
         render_target_get_minimum_size(our_target,&minwidth, &minheight);
      }
      else{
         minwidth=1024;minheight=768;
      }

      if (adjust_opt[0])
      {
		adjust_opt[0] = 0;

		if (adjust_opt[2])
		{
			adjust_opt[2] = 0;
			refresh_rate = (machine->primary_screen == NULL) ? screen_device::k_default_frame_rate : ATTOSECONDS_TO_HZ(machine->primary_screen->frame_period().attoseconds);
			update_geometry();
		}

		if ((adjust_opt[3] || adjust_opt[4] || adjust_opt[5]) && adjust_opt[1])
		{
			screen_device *screen = screen_first(*machine);
			render_container *container = render_container_get_screen(screen);
			render_container_user_settings settings;
			render_container_get_user_settings(container, &settings);

			if (adjust_opt[3])
			{
				adjust_opt[3] = 0;
				settings.brightness = arroffset[0] + 1.0f;
				render_container_set_user_settings(container, &settings);
			}
			if (adjust_opt[4])
			{
				adjust_opt[4] = 0;
				settings.contrast = arroffset[1] + 1.0f;
				render_container_set_user_settings(container, &settings);
			}
			if (adjust_opt[5])
			{
				adjust_opt[5] = 0;
				settings.gamma = arroffset[2] + 1.0f;
				render_container_set_user_settings(container, &settings);
			}
		}
      }

      if (FirstTimeUpdate == 1) {

         FirstTimeUpdate++;
         write_log("game screen w=%i h=%i  rowPixels=%i\n", minwidth, minheight,minwidth );

         rtwi=minwidth;
         rthe=minheight;
         topw=minwidth;

         int gamRot=0;
         orient  = (machine->gamedrv->flags & ORIENTATION_MASK);
         vertical = (machine->gamedrv->flags & ORIENTATION_SWAP_XY);

         gamRot = (ROT270 == orient) ? 1 : gamRot;
         gamRot = (ROT180 == orient) ? 2 : gamRot;
         gamRot = (ROT90  == orient) ? 3 : gamRot;

         prep_retro_rotation(gamRot);
      }

      if (minwidth != rtwi || minheight != rthe ){
         write_log("Res change: old(%d,%d) new(%d,%d) %d\n",rtwi,rthe,minwidth,minheight,topw);
         rtwi=minwidth;
         rthe=minheight;
         topw=minwidth;

	 adjust_opt[0] = adjust_opt[2] = 1;
      }
/*    No need
      if(videoapproach1_enable){
         rtwi=topw=1024;
         rthe=768;
      } */

      // make that the size of our target
      render_target_set_bounds(our_target,rtwi,rthe, 0);
      // our_target->set_bounds(rtwi,rthe);
      // get the list of primitives for the target at the current size
      // render_primitive_list &primlist = our_target->get_primitives();
      primlist = render_target_get_primitives(our_target);
      // lock them, and then render them
      //      primlist.acquire_lock();
      osd_lock_acquire(primlist->lock);

      surfptr = (UINT8 *) videoBuffer;
#ifdef M16B
      rgb565_draw_primitives(primlist->head, surfptr,rtwi,rthe,rtwi);
#else
      rgb888_draw_primitives(primlist->head, surfptr, rtwi,rthe,rtwi);
#endif
#if 0
      surfptr = (UINT8 *) videoBuffer;

      //  draw a series of primitives using a software rasterizer
      for (const render_primitive *prim = primlist.first(); prim != NULL; prim = prim->next())
      {
         switch (prim->type)
         {
            case render_primitive::LINE:
               draw_line(*prim, (PIXEL_TYPE*)surfptr, minwidth, minheight, minwidth);
               break;

            case render_primitive::QUAD:
               if (!prim->texture.base)
                  draw_rect(*prim, (PIXEL_TYPE*)surfptr, minwidth, minheight, minwidth);
               else
                  setup_and_draw_textured_quad(*prim, (PIXEL_TYPE*)surfptr, minwidth, minheight, minwidth);
               break;

            default:
               throw emu_fatalerror("Unexpected render_primitive type");
         }
      }
#endif
      osd_lock_release(primlist->lock);


      //  primlist.release_lock();
   } 
   else
      draw_this_frame = false;

   RLOOP=0;

   if(ui_ipt_pushchar!=-1)
   {
      ui_input_push_char_event(machine, our_target, (unicode_char)ui_ipt_pushchar);
      ui_ipt_pushchar=-1;
   }
}

 //============================================================
// osd_wait_for_debugger
//============================================================

void osd_wait_for_debugger(running_device *device, int firststop)
{
   // we don't have a debugger, so we just return here
}

//============================================================
//  update_audio_stream
//============================================================
void osd_update_audio_stream(running_machine *machine,short *buffer, int samples_this_frame) 
{
	if(pauseg!=-1)audio_batch_cb(buffer, samples_this_frame);
}


//============================================================
//  main
//============================================================

static const char* xargv[] = {
	"mamemini",
	"-joystick",
	"-noautoframeskip",
	"-samplerate",
	"48000",
	"-sound",
	"-contrast",
	"1.0",
	"-brightness",
	"1.0",
	"-gamma",
	"1.0",
	"-rompath",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

static int parsePath(char* path, char* gamePath, char* gameName) {
	int i;
	int slashIndex = -1;
	int dotIndex = -1;
	int len = strlen(path);
	if (len < 1) {
		return 0;
	}

	for (i = len - 1; i >=0; i--) {
		if (path[i] == path_default_slash()[0]) {
			slashIndex = i;
			break;
		} else
		if (path[i] == '.') {
			dotIndex = i;
		}
	}
	if (slashIndex < 0 && dotIndex >0){
		strcpy(gamePath, ".\0");
		strncpy(gameName, path , dotIndex );
		gameName[dotIndex] = 0;
		write_log("[INFO][MAME2010] gamePath=%s gameName=%s\n", gamePath, gameName);
		return 1;
	}
	if (slashIndex < 0 || dotIndex < 0) {
		return 0;
	}

	strncpy(gamePath, path, slashIndex);
	gamePath[slashIndex] = 0;
	strncpy(gameName, path + (slashIndex + 1), dotIndex - (slashIndex + 1));
	gameName[dotIndex - (slashIndex + 1)] = 0;

	write_log("[INFO][MAME2010] gamePath=%s gameName=%s\n", gamePath, gameName);
	return 1;
}

static int getGameInfo(char* gameName, int* rotation, int* driverIndex) {
	int gameFound = 0;
	int drvindex;

	//check invalid game name
	if (gameName[0] == 0)
		return 0;

	for (drvindex = 0; drivers[drvindex]; drvindex++) {
		if ( (drivers[drvindex]->flags & GAME_NO_STANDALONE) == 0 &&
			mame_strwildcmp(gameName, drivers[drvindex]->name) == 0 ) {
				gameFound = 1;
				*driverIndex = drvindex;
				*rotation = drivers[drvindex]->flags & 0x7;
				write_log("[INFO][MAME2010] %-18s\"%s\" rot=%i \n", drivers[drvindex]->name, drivers[drvindex]->description, *rotation);
		}
	}
	return gameFound;
}

int executeGame(char* path) {
	// cli_frontend does the heavy lifting; if we have osd-specific options, we
	// create a derivative of cli_options and add our own

	int paramCount;
	int result = 0;
	int gameRot=0;

	int driverIndex;

	FirstTimeUpdate = 1;

	screenRot = 0;

	//split the path to directory and the name without the zip extension
	result = parsePath(path, MgamePath, MgameName);
	if (result == 0) {
		write_log("[ERROR][MAME2010] Parse path failed! path=%s\n", path);
		strcpy(MgameName,path);
	//	return -1;
	}

	//Find the game info. Exit if game driver was not found.
	if (getGameInfo(MgameName, &gameRot, &driverIndex) == 0) {
		write_log("[ERROR][MAME2010] Game not found: %s\n", MgameName);
		return -2;
	}

	//tate enabled
	if (tate) {
		//horizontal game
		if (gameRot == ROT0) {
			screenRot = 1;
		} else
		if (gameRot &  ORIENTATION_FLIP_X) {
			write_log("[INFO][MAME2010] *********** flip X \n");
			screenRot = 3;
		}

	} else
	{
		if (gameRot != ROT0) {
			screenRot = 1;
			if (gameRot &  ORIENTATION_FLIP_X) {
				write_log("[INFO][MAME2010] *********** flip X \n");
				screenRot = 2;
			}
		}
	}

	write_log("[INFO][MAME2010] Creating frontend... game=%s\n", MgameName);

	//find how many parameters we have
	for (paramCount = 0; xargv[paramCount] != NULL; paramCount++);

	xargv[paramCount++] = (char*)libretro_content_directory;

	if (tate) {
		if (screenRot == 3) {
			xargv[paramCount++] =(char*) "-rol";
		} else {
			xargv[paramCount++] = (char*)(screenRot ? "-mouse" : "-ror");
		}
	} else {
		if (screenRot == 2) {
			xargv[paramCount++] = (char*)"-rol";
		} else {
			xargv[paramCount++] = (char*)(screenRot ? "-ror" : "-mouse");
		}
	}

	if(hide_gameinfo) {
		xargv[paramCount++] =(char*) "-skip_gameinfo";
	}

	if(hide_nagscreen) {
		xargv[paramCount++] =(char*) "-skip_nagscreen";
	}

	if(hide_warnings) {
		xargv[paramCount++] =(char*) "-skip_warnings";
	}

	xargv[paramCount++] = MgameName;

	write_log("[INFO][MAME2010] Invoking MAME2010 CLI frontend. Parameter count: %i\n", paramCount);

    write_log("[INFO][MAME2010] Parameter list: ");
	for (int i = 0; xargv[i] != NULL; i++){
		write_log("%s ",xargv[i]);
	}
    write_log("\n");

	result = cli_execute(paramCount,(char**) xargv, NULL);

	xargv[paramCount - 2] = NULL;

	return result;
}

//============================================================
//  mmain
//============================================================

#ifdef __cplusplus
extern "C"
#endif
int mmain(int argc, const char *argv)
{
	static char gameName[1024];

	strncpy(gameName, argv, 1024);
	if(executeGame(gameName)!=0) return -1;
	return 1;
}
