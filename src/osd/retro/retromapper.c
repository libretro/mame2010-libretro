void retro_poll_mame_input();

static int rtwi=320,rthe=240,topw=1024; // DEFAULT TEXW/TEXH/PITCH
int SHIFTON = -1;
char RPATH[512];

extern "C" int mmain(int argc, const char *argv);
extern bool draw_this_frame;

#ifdef M16B
	uint16_t videoBuffer[1024*1024];
	#define PITCH 1
#else
	unsigned int videoBuffer[1024*1024];
	#define PITCH 1 * 2
#endif

retro_video_refresh_t video_cb = NULL;
retro_environment_t environ_cb = NULL;

retro_log_printf_t log_cb;

static retro_input_state_t input_state_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "retroogl.c"
#endif

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
static retro_input_poll_t input_poll_cb;

void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }

void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { }

void retro_set_environment(retro_environment_t cb)
{
   static const struct retro_variable vars[] = {
      { "mame_current_mouse_enable", "Mouse supported; disabled|enabled" },
      { "mame_current_videoapproach1_enable", "Video approach 1 Enabled; disabled|enabled" },
      { "mame_current_skip_nagscreen", "Hide nag screen; disabled|enabled" },
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

extern void retro_finish();
extern void retro_main_loop();

int RLOOP=1;

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

void retro_unload_game(void)
{
	if(pauseg == 0)
		pauseg = -1;

	LOGI("Retro unload_game\n");
}

// Stubs
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
