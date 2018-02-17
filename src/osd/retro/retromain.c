
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "osdepend.h"

#include "emu.h"
#include "clifront.h"
#include "render.h"
#include "ui.h"
#include "uiinput.h"

#include "libretro.h" 
#include "retromain.h"

#include "log.h"

#if !defined(HAVE_OPENGL) && !defined(HAVE_OPENGLES) && !defined(HAVE_RGB32)
   #define M16B
#endif

char g_rom_dir[1024];

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
#include "rendersw.c"

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

static void extract_basename(char *buf, const char *path, size_t size)
{
   const char *base = strrchr(path, '/');
   if (!base)
      base = strrchr(path, '\\');
   if (!base)
      base = path;

   if (*base == '\\' || *base == '/')
      base++;

   strncpy(buf, base, size - 1);
   buf[size - 1] = '\0';

   char *ext = strrchr(buf, '.');
   if (ext)
      *ext = '\0';
}

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
//  CONSTANTS
//============================================================

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

#include "retromapper.c"
#include "retroinput.c"

#if defined(_WIN32)
char PATH_DELIMITER = '\\';
#else
char PATH_DELIMITER = '/';
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

void retro_init (void)
{      
   const char *system_dir  = NULL;
   const char *save_dir    = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &system_dir) && system_dir)
   {
       // use a subfolder in the system directory with the core name (ie mame2010)
        snprintf(libretro_system_directory, sizeof(libretro_system_directory), "%s%c%s", system_dir, PATH_DELIMITER, core_name);
   }

   if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &save_dir) && save_dir)
   {
       // use a subfolder in the save directory with the core name (ie mame2010)
        snprintf(libretro_save_directory, sizeof(libretro_save_directory), "%s%c%s", save_dir, PATH_DELIMITER, core_name);
   }
   else
   {
        *libretro_save_directory = *libretro_system_directory;
   }
   
    mkdir(libretro_system_directory);
    mkdir(libretro_save_directory);
 
    // content loaded from mame2010 subfolder within the libretro system folder
    snprintf(cheatpath, sizeof(cheatpath), "%c%s", PATH_DELIMITER, libretro_system_directory);
    mkdir(libretro_save_directory);
    snprintf(samplepath, sizeof(samplepath), "%s%c%s", libretro_system_directory, PATH_DELIMITER, "samples");
    mkdir(samplepath);
    snprintf(artpath, sizeof(artpath), "%s%c%s", libretro_system_directory, PATH_DELIMITER, "artwork");
    mkdir(artpath);
    snprintf(fontpath, sizeof(fontpath), "%s%c%s", libretro_system_directory, PATH_DELIMITER, "fonts");
    mkdir(fontpath);
    snprintf(crosshairpath, sizeof(crosshairpath), "%s%c%s", libretro_system_directory, PATH_DELIMITER, "crosshairs");
    mkdir(crosshairpath);

    // user-generated content loaded from mame2010 subfolder within the libretro save folder
    snprintf(ctrlrpath, sizeof(ctrlrpath), "%s%c%s", libretro_save_directory, PATH_DELIMITER, "ctrlr");
    mkdir(ctrlrpath);
    snprintf(inipath, sizeof(inipath), "%s%c%s", libretro_save_directory, PATH_DELIMITER, "ini");
    mkdir(inipath);
    snprintf(cfg_directory, sizeof(cfg_directory), "%s%c%s", libretro_save_directory, PATH_DELIMITER, "cfg");
    mkdir(cfg_directory);
    snprintf(nvram_directory, sizeof(nvram_directory), "%s%c%s", libretro_save_directory, PATH_DELIMITER, "nvram");
    mkdir(nvram_directory);
    snprintf(memcard_directory, sizeof(memcard_directory), "%s%c%s", libretro_save_directory, PATH_DELIMITER, "memcard");
    mkdir(memcard_directory);
    snprintf(input_directory, sizeof(input_directory), "%s%c%s", libretro_save_directory, PATH_DELIMITER, "input");
    mkdir(input_directory);
    snprintf(image_directory, sizeof(image_directory), "%s%c%s", libretro_save_directory, PATH_DELIMITER, "image");
    mkdir(image_directory);
    snprintf(comment_directory, sizeof(comment_directory), "%s%c%s", libretro_save_directory, PATH_DELIMITER, "comment");
    mkdir(comment_directory);

}

bool retro_load_game(const struct retro_game_info *info)
{
   char basename[128];
   int result;
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
      fprintf(stderr, "RGB pixel format is not supported.\n");
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

   basename[0] = '\0';
   extract_basename(basename, info->path, sizeof(basename));
   extract_directory(g_rom_dir, info->path, sizeof(g_rom_dir));
   
   snprintf(libretro_content_directory, sizeof(libretro_content_directory), "%s", g_rom_dir);

   struct retro_log_callback log_cb;
   
   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log_cb))
   {     
      log_cb.log(RETRO_LOG_INFO, "libretro system directory: %s\n", libretro_system_directory);
      log_cb.log(RETRO_LOG_INFO, "libretro content directory: %s\n", libretro_content_directory);
      log_cb.log(RETRO_LOG_INFO, "libretro save directory: %s\n", libretro_save_directory);
   }
   
   strcpy(RPATH,info->path);

   result=mmain(1,RPATH);
   if(result!=1){
        	printf("Error: mame return an error\n");
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
//  set_mastervolume
//============================================================
void osd_set_mastervolume(int attenuation)
{
   // if we had actual sound output, we would adjust the global
   // volume in response to this function
}


//============================================================
//  customize_input_type_list
//============================================================
void osd_customize_input_type_list(input_type_desc *typelist)
{
	// This function is called on startup, before reading the
	// configuration from disk. Scan the list, and change the
	// default control mappings you want. It is quite possible
	// you won't need to change a thing.
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
		if (path[i] == PATH_DELIMITER) {
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
//FIXME for 0.149 , prevouisly in driver.h
#if 1
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
#else
	gameFound = 1;
#endif
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

	xargv[paramCount++] = (char*)g_rom_dir;

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

	write_log("[INFO][MAME2010] Executing frontend... params:%i\n", paramCount);

	for (int i = 0; xargv[i] != NULL; i++){
		write_log("%s ",xargv[i]);
		write_log("\n");
	}

	result = cli_execute(paramCount,(char**) xargv, NULL);

	xargv[paramCount - 2] = NULL;

	return result;
}

//============================================================
//  main
//============================================================

#ifdef __cplusplus
extern "C"
#endif
int mmain(int argc, const char *argv)
{
	static char gameName[1024];
	int result = 0;

	strcpy(gameName,argv);
	result = executeGame(gameName);
	if(result!=0)return -1;
	return 1;
}
