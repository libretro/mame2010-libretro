/***************************************************************************
retromain.c 
mame2010 - libretro port of mame 0.139
****************************************************************************/

#include <unistd.h>
#include <stdint.h>
#include "osdepend.h"

#include "emu.h"
#include "clifront.h"
#include "render.h"
#include "ui.h"
#include "uiinput.h"

#include "libretro.h" 
#include "retromain.h"
#include "file/file_path.h"

#include "rendersw.c"

#include "../../precompile/mameini_boilerplate.h"

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
char hiscore_directory[1024];
char comment_directory[1024];

//Shell for CPU overclock setting. Search mame_current_overclock for other pieces
//double oc;

int mame_reset = -1;
static int ui_ipt_pushchar=-1;

static int mouse_mode = 1;
static bool videoapproach1_enable = false;
bool hide_nagscreen = false;
bool hide_gameinfo = false;
bool hide_warnings = false;

static void update_geometry();
static unsigned int turbo_enable, turbo_state, turbo_delay = 5;
static bool set_par = false;
static double refresh_rate = 60.0;
static int set_frame_skip;
static unsigned sample_rate = 48000;
unsigned use_external_hiscore = 0;
static unsigned adjust_opt[6] = {0/*Enable/Disable*/, 0/*Limit*/, 0/*GetRefreshRate*/, 0/*Brightness*/, 0/*Contrast*/, 0/*Gamma*/};
static float arroffset[3] = {0/*For brightness*/, 0/*For contrast*/, 0/*For gamma*/};

static int rtwi=320,rthe=240,topw=1024; // DEFAULT TEXW/TEXH/PITCH
int SHIFTON = -1;

extern "C" int mmain(int argc, const char *argv);
extern bool draw_this_frame;

retro_video_refresh_t video_cb = NULL;
retro_environment_t environ_cb = NULL;

retro_log_printf_t retro_log = NULL;

static retro_input_state_t input_state_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;
static retro_input_poll_t input_poll_cb = NULL;

int RLOOP=1;

// rendering target
static render_target *our_target = NULL;

// input device
static input_device *P1_device;  // P1 JOYPAD
static input_device *P2_device;  // P2 JOYPAD
static input_device *P3_device;  // P3 JOYPAD
static input_device *P4_device;  // P4 JOYPAD
static input_device *P5_device;  // P5 JOYPAD
static input_device *P6_device;  // P6 JOYPAD
static input_device *P7_device;  // P7 JOYPAD
static input_device *P8_device;  // P8 JOYPAD
static input_device *mouse1_device; // P1 MOUSE
static input_device *mouse2_device; // P2 MOUSE
static input_device *mouse3_device; // P3 MOUSE
static input_device *mouse4_device; // P4 MOUSE
static input_device *mouse5_device; // P5 MOUSE
static input_device *mouse6_device; // P6 MOUSE
static input_device *mouse7_device; // P7 MOUSE
static input_device *mouse8_device; // P8 MOUSE
static input_device *gun1_device; // P1 LIGHTGUN
static input_device *gun2_device; // P2 LIGHTGUN
static input_device *gun3_device; // P3 LIGHTGUN
static input_device *gun4_device; // P4 LIGHTGUN
static input_device *gun5_device; // P5 LIGHTGUN
static input_device *gun6_device; // P6 LIGHTGUN
static input_device *gun7_device; // P7 LIGHTGUN
static input_device *gun8_device; // P8 LIGHTGUN
static input_device *retrokbd_device; // KEYBD

// state
static UINT32 P1_state[KEY_TOTAL];
static UINT32 P2_state[KEY_TOTAL];
static UINT32 P3_state[KEY_TOTAL];
static UINT32 P4_state[KEY_TOTAL];
static UINT32 P5_state[KEY_TOTAL];
static UINT32 P6_state[KEY_TOTAL];
static UINT32 P7_state[KEY_TOTAL];
static UINT32 P8_state[KEY_TOTAL];
static UINT16 retrokbd_state[RETROK_LAST];
static UINT16 retrokbd_state2[RETROK_LAST];

int optButtonLayoutP1 = 0; //for player 1
int optButtonLayoutP2 = 0; //for player 2

static int mouse1X,mouse1Y;
static int mouse2X,mouse2Y;
static int mouse3X,mouse3Y;
static int mouse4X,mouse4Y;
static int mouse5X,mouse5Y;
static int mouse6X,mouse6Y;
static int mouse7X,mouse7Y;
static int mouse8X,mouse8Y;

static int gun1X,gun1Y;
static int gun2X,gun2Y;
static int gun3X,gun3Y;
static int gun4X,gun4Y;
static int gun5X,gun5Y;
static int gun6X,gun6Y;
static int gun7X,gun7Y;
static int gun8X,gun8Y;

//enables / disables tate mode
static int tate = 0;
static int screenRot = 0;
int vertical,orient;

static char MgamePath[1024];
static char MgameName[512];

static int FirstTimeUpdate = 1;

bool retro_load_ok  = false;
int pauseg = 0;

/*********************************************
   LOCAL FUNCTION PROTOTYPES
*********************************************/

static void check_variables(void);
static void initInput(running_machine* machine);

/*********************************************/


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
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { }

void retro_init (void)
{   
    struct retro_log_callback log_cb;

    if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log_cb))
        retro_log = log_cb.log;
    	
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
    snprintf(hiscore_directory, sizeof(hiscore_directory), "%s%s%s", libretro_save_directory, path_default_slash(), "hi");
    path_mkdir(hiscore_directory);    
    snprintf(comment_directory, sizeof(comment_directory), "%s%s%s", libretro_save_directory, path_default_slash(), "comment");
    path_mkdir(comment_directory);

    char mameini_path[1024];
    
    snprintf(mameini_path, sizeof(mameini_path), "%s%s%s", inipath, path_default_slash(), "mame.ini");
    if(!path_is_valid(mameini_path))
    {
        retro_log(RETRO_LOG_INFO, "[MAME 2010] mame.ini not found at: %s\n", mameini_path);
        
        FILE *mameini_file;
        if((mameini_file=fopen(mameini_path, "wb"))==NULL)
        {
            retro_log(RETRO_LOG_ERROR, "[MAME 2010] something went wrong generating new mame.ini at: %s\n", mameini_path);
        }
        else
        {
            fwrite(mameini_boilerplate, sizeof(char), mameini_boilerplate_length, mameini_file);          
            fclose(mameini_file);
            retro_log(RETRO_LOG_INFO, "[MAME 2010] new mame.ini generated at: %s\n", mameini_path);            
        }
    }
    else
        retro_log(RETRO_LOG_INFO, "[MAME 2010] mame.ini found at: %s\n", mameini_path);

}

bool retro_load_game(const struct retro_game_info *info)
{
   strncpy(libretro_content_directory, info->path, sizeof(libretro_content_directory));
   path_basedir(libretro_content_directory);
   
   retro_log(RETRO_LOG_INFO, "[MAME 2010] libretro_content_directory: %s\n", libretro_content_directory);  
   retro_log(RETRO_LOG_INFO, "[MAME 2010] libretro_system_directory: %s\n", libretro_system_directory);
   retro_log(RETRO_LOG_INFO, "[MAME 2010] libretro_save directory: %s\n", libretro_save_directory); 
   
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
      retro_log(RETRO_LOG_ERROR, "[MAME 2010] RGB pixel format is not supported.\n");
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
        retro_log(RETRO_LOG_ERROR, "[MAME 2010] MAME returned an error!\n");
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
   retro_log(RETRO_LOG_INFO, "[MAME 2010] osd_exit called \n");

   if (our_target != NULL)
      render_target_free(our_target);
   our_target = NULL;

   global_free(P1_device);
   global_free(P2_device);
   global_free(P3_device);
   global_free(P4_device);
   global_free(P5_device);
   global_free(P6_device);
   global_free(P7_device);
   global_free(P8_device);
   global_free(retrokbd_device);
   global_free(mouse1_device);
   global_free(mouse2_device);
   global_free(mouse3_device);
   global_free(mouse4_device);
   global_free(mouse5_device);
   global_free(mouse6_device);
   global_free(mouse7_device);
   global_free(mouse8_device);
   global_free(gun1_device);
   global_free(gun2_device);
   global_free(gun3_device);
   global_free(gun4_device);
   global_free(gun5_device);
   global_free(gun6_device);
   global_free(gun7_device);
   global_free(gun8_device);
}

void osd_init(running_machine* machine)
{
   retro_log(RETRO_LOG_INFO, "[MAME 2010] osd_init starting\n");
  
   int gamRot=0;

   machine->add_notifier(MACHINE_NOTIFY_EXIT, osd_exit);

   our_target = render_target_alloc(machine,NULL, 0);

   initInput(machine);

   retro_log(RETRO_LOG_INFO, "[MAME 2010] Machine screen orientation: %s \n",
         (machine->gamedrv->flags & ORIENTATION_SWAP_XY) ? "VERTICAL" : "HORIZONTAL"
         );

   orient = (machine->gamedrv->flags & ORIENTATION_MASK);
   vertical = (machine->gamedrv->flags & ORIENTATION_SWAP_XY);

   gamRot = (ROT270 == orient) ? 1 : gamRot;
   gamRot = (ROT180 == orient) ? 2 : gamRot;
   gamRot = (ROT90 == orient) ? 3 : gamRot;

   prep_retro_rotation(gamRot);
   machine->sample_rate = sample_rate;	/* Override original value */

   retro_log(RETRO_LOG_INFO, "[MAME 2010] osd_init done\n");
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
         retro_log(RETRO_LOG_INFO, "[MAME 2010] game screen w=%i h=%i  rowPixels=%i\n", minwidth, minheight,minwidth );

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
         retro_log(RETRO_LOG_INFO, "[MAME 2010] Res change: old(%d,%d) new(%d,%d) %d\n",rtwi,rthe,minwidth,minheight,topw);
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

void osd_wait_for_debugger(running_device *device, int firststop)
{
   // we don't have a debugger, so we just return here
}

void osd_update_audio_stream(running_machine *machine,short *buffer, int samples_this_frame) 
{
	if(pauseg!=-1)audio_batch_cb(buffer, samples_this_frame);
}

void retro_set_environment(retro_environment_t cb)
{
   static const struct retro_variable vars[] = {
      { "mame_current_xy_type", "XY device type (Restart); mouse|lightgun|none" },
      //Shell for CPU overclock setting. Search mame_current_overclock for other pieces
      //{ "mame_current_overclock", "Main CPU Overclock; 100|25|30|35|40|45|50|55|60|65|70|75|80|95|90|95|105|110|115|120" },
      { "mame_current_videoapproach1_enable", "Video approach 1 Enabled; disabled|enabled" },
      { "mame_current_skip_nagscreen", "Hide nag screen; enabled|disabled" },
      { "mame_current_skip_gameinfo", "Hide game info screen; disabled|enabled" },
      { "mame_current_skip_warnings", "Hide warning screen; disabled|enabled" },
      { "mame_current_aspect_ratio", "Core provided aspect ratio; DAR|PAR" },
      { "mame_current_turbo_button", "Enable autofire; disabled|button 1|button 2|R2 to button 1 mapping|R2 to button 2 mapping" },
      { "mame_current_turbo_delay", "Set autofire pulse speed; medium|slow|fast" },
      { "mame_current_frame_skip", "Set frameskip; 0|1|2|3|4|5|6|7|8|9|10|automatic" },
      { "mame_current_sample_rate", "Set sample rate (Restart); 48000Hz|44100Hz|32000Hz|22050Hz" },
      { "mame_current_adj_brightness",
	"Set brightness; default|+1%|+2%|+3%|+4%|+5%|+6%|+7%|+8%|+9%|+10%|+11%|+12%|+13%|+14%|+15%|+16%|+17%|+18%|+19%|+20%|-20%|-19%|-18%|-17%|-16%|-15%|-14%|-13%|-12%|-11%|-10%|-9%|-8%|-7%|-6%|-5%|-4%|-3%|-2%|-1%" },
      { "mame_current_adj_contrast",
	"Set contrast; default|+1%|+2%|+3%|+4%|+5%|+6%|+7%|+8%|+9%|+10%|+11%|+12%|+13%|+14%|+15%|+16%|+17%|+18%|+19%|+20%|-20%|-19%|-18%|-17%|-16%|-15%|-14%|-13%|-12%|-11%|-10%|-9%|-8%|-7%|-6%|-5%|-4%|-3%|-2%|-1%" },
      { "mame_current_adj_gamma",
	"Set gamma; default|+1%|+2%|+3%|+4%|+5%|+6%|+7%|+8%|+9%|+10%|+11%|+12%|+13%|+14%|+15%|+16%|+17%|+18%|+19%|+20%|-20%|-19%|-18%|-17%|-16%|-15%|-14%|-13%|-12%|-11%|-10%|-9%|-8%|-7%|-6%|-5%|-4%|-3%|-2%|-1%" },
      { "mame-external_hiscore", "Use external hiscore.dat; disabled|enabled" },
      { NULL, NULL },
   };

   environ_cb = cb;

   cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);
}

static void check_variables(void)
{
   struct retro_variable var = {0};
   bool tmp_ar = set_par;
	
   var.key = "mame_current_xy_type";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      retro_log(RETRO_LOG_INFO, "[MAME 2010] mouse_mode value: %s\n", var.value);
      if (!strcmp(var.value, "mouse"))
         mouse_mode = 1;
      if (!strcmp(var.value, "lightgun"))
         mouse_mode = 2;
      if (!strcmp(var.value, "none"))
	 mouse_mode = 0;
   }
   //Shell for CPU overclock setting. Search mame_current_overclock for other pieces
   //var.key = "mame_current_overclock";
   //var.value = NULL;
   //if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   //{
   //   oc = (double) atoi(var.value) / 100;
   //	   
   //}

   var.key = "mame_current_skip_nagscreen";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      retro_log(RETRO_LOG_INFO, "[MAME 2010] skip_nagscreen value: %s\n", var.value);
      if (!strcmp(var.value, "disabled"))
         hide_nagscreen = false;
      if (!strcmp(var.value, "enabled"))
         hide_nagscreen = true;
   }

   var.key = "mame_current_skip_gameinfo";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      retro_log(RETRO_LOG_INFO, "[MAME 2010] skip_gameinfo value: %s\n", var.value);
      if (!strcmp(var.value, "disabled"))
         hide_gameinfo = false;
      if (!strcmp(var.value, "enabled"))
         hide_gameinfo = true;
   }

   var.key = "mame_current_skip_warnings";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      retro_log(RETRO_LOG_INFO, "[MAME 2010] skip_warnings value: %s\n", var.value);
      if (!strcmp(var.value, "disabled"))
         hide_warnings = false;
      if (!strcmp(var.value, "enabled"))
         hide_warnings = true;
   }

   var.key = "mame_current_videoapproach1_enable";
   var.value = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      retro_log(RETRO_LOG_INFO, "[MAME 2010] videoapproach1_enable value: %s\n", var.value);
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

   var.value = NULL;
   var.key = "mame-external_hiscore";
   
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if(strcmp(var.value, "enabled") == 0)
         use_external_hiscore = 1;
      else
         use_external_hiscore = 0;    
    }
   else
      use_external_hiscore = 0;  

  
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
	retro_log(RETRO_LOG_INFO, "Current aspect ratio = %d : %d , screen refresh rate = %f , sound sample rate = %.1f \n", set_par ? vertical ? rthe / common_factor : rtwi / common_factor :
			vertical ? 3 : 4, set_par ? vertical ? rtwi / common_factor : rthe / common_factor : vertical ? 4 : 3, info->timing.fps, info->timing.sample_rate);
#endif
}

void retro_deinit(void)
{
   if(retro_load_ok)retro_finish();
   retro_log(RETRO_LOG_INFO, "[MAME 2010] retro_deinit called\n");
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
   retro_log(RETRO_LOG_INFO, "[MAME 2010] Rotation:%d\n",rot);
   environ_cb(RETRO_ENVIRONMENT_SET_ROTATION, &rot);
}

void retro_unload_game(void)
{
	if(pauseg == 0)
		pauseg = -1;

	retro_log(RETRO_LOG_INFO, "[MAME 2010] Retro unload_game\n");
}

void init_input_descriptors(void)
{
   #define describe_buttons(INDEX) \
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,     "Joystick Left" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,    "Joystick Right" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,       "Joystick Up" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,     "Joystick Down" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,        "Button 1" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,        "Button 2" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,        "Button 3" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Insert Coin" },\
   { INDEX, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X, "Analog LX" },\
   { INDEX, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y, "Analog LY" },\
   { INDEX, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X, "Analog RX" },\
   { INDEX, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y, "Analog RY" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2,       "Turbo Button" },\
   { INDEX, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER,    "Gun Trigger" },\
   { INDEX, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD,    "Gun Offscreen Shot" },\
   { INDEX, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_A,    "Gun Aux A" },\
   { INDEX, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_B,    "Gun Aux B" },\
   { INDEX, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START,    "Gun Start" },\
   { INDEX, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SELECT,    "Gun Select" },

   #define describe_extended_buttons(INDEX) \
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,        "Button 4" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,        "Button 5" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,        "Button 6" },\
   { INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2,       "Button 7" },\

   #define describe_GUI_buttons(INDEX) \
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3,           "Service/Test"  },\
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3,           "Toggle MAME UI" },
    
   struct retro_input_descriptor desc[] = {

      /* start with gui function keys unique to the Player 1 joypad */
      describe_GUI_buttons(0)
      describe_buttons(0)
      describe_extended_buttons(0)         
      describe_buttons(1)
      describe_extended_buttons(1)
      describe_buttons(2)
      describe_buttons(3)
      { 0, 0, 0, 0, NULL }
   };

   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
}

#define PLAYER1_PRESS(button) input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_##button)
#define PLAYER2_PRESS(button) input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_##button)
#define PLAYER3_PRESS(button) input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_##button)
#define PLAYER4_PRESS(button) input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_##button)
#define PLAYER5_PRESS(button) input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_##button)
#define PLAYER6_PRESS(button) input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_##button)
#define PLAYER7_PRESS(button) input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_##button)
#define PLAYER8_PRESS(button) input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_##button)

#define input_device_item_add_mouse1(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_mouse2(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_mouse3(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_mouse4(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_mouse5(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_mouse6(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_mouse7(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_mouse8(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_kbd(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_gun1(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_gun2(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_gun3(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_gun4(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_gun5(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_gun6(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_gun7(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_gun8(a,b,c,d,e) input_device_item_add(a,b,c,d,e)

#define input_device_item_add_p1(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_p2(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_p3(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_p4(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_p5(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_p6(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_p7(a,b,c,d,e) input_device_item_add(a,b,c,d,e)
#define input_device_item_add_p8(a,b,c,d,e) input_device_item_add(a,b,c,d,e)

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

static INT32 pad3_get_state(void *device_internal, void *item_internal)
{
   UINT8 *itemdata = (UINT8 *)item_internal;
   return *itemdata;
}

static INT32 pad4_get_state(void *device_internal, void *item_internal)
{
   UINT8 *itemdata = (UINT8 *)item_internal;
   return *itemdata;
}

static INT32 pad5_get_state(void *device_internal, void *item_internal)
{
   UINT8 *itemdata = (UINT8 *)item_internal;
   return *itemdata;
}

static INT32 pad6_get_state(void *device_internal, void *item_internal)
{
   UINT8 *itemdata = (UINT8 *)item_internal;
   return *itemdata;
}

static INT32 pad7_get_state(void *device_internal, void *item_internal)
{
   UINT8 *itemdata = (UINT8 *)item_internal;
   return *itemdata;
}

static INT32 pad8_get_state(void *device_internal, void *item_internal)
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


static void initInput(running_machine* machine)
{
   int i,button;
   char defname[20];

   // create mouse devices
   mouse1_device = input_device_add(machine, DEVICE_CLASS_MOUSE, "Mouse1", NULL);
   mouse2_device = input_device_add(machine, DEVICE_CLASS_MOUSE, "Mouse2", NULL);
   mouse3_device = input_device_add(machine, DEVICE_CLASS_MOUSE, "Mouse3", NULL);
   mouse4_device = input_device_add(machine, DEVICE_CLASS_MOUSE, "Mouse4", NULL);
   mouse5_device = input_device_add(machine, DEVICE_CLASS_MOUSE, "Mouse5", NULL);
   mouse6_device = input_device_add(machine, DEVICE_CLASS_MOUSE, "Mouse6", NULL);
   mouse7_device = input_device_add(machine, DEVICE_CLASS_MOUSE, "Mouse7", NULL);
   mouse8_device = input_device_add(machine, DEVICE_CLASS_MOUSE, "Mouse8", NULL);

   // add the mouse axes
   input_device_item_add_mouse1(mouse1_device, "X", &mouse1X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_mouse1(mouse1_device, "Y", &mouse1Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_mouse2(mouse2_device, "X", &mouse2X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_mouse2(mouse2_device, "Y", &mouse2Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_mouse3(mouse3_device, "X", &mouse3X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_mouse3(mouse3_device, "Y", &mouse3Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_mouse4(mouse4_device, "X", &mouse4X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_mouse4(mouse4_device, "Y", &mouse4Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_mouse5(mouse5_device, "X", &mouse5X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_mouse5(mouse5_device, "Y", &mouse5Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_mouse6(mouse6_device, "X", &mouse6X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_mouse6(mouse6_device, "Y", &mouse6Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_mouse7(mouse7_device, "X", &mouse7X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_mouse7(mouse7_device, "Y", &mouse7Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_mouse8(mouse8_device, "X", &mouse8X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_mouse8(mouse8_device, "Y", &mouse8Y, ITEM_ID_YAXIS, generic_axis_get_state);

   //create lightgun devices
   gun1_device = input_device_add(machine, DEVICE_CLASS_LIGHTGUN, "Gun1", NULL);
   gun2_device = input_device_add(machine, DEVICE_CLASS_LIGHTGUN, "Gun2", NULL);
   gun3_device = input_device_add(machine, DEVICE_CLASS_LIGHTGUN, "Gun3", NULL);
   gun4_device = input_device_add(machine, DEVICE_CLASS_LIGHTGUN, "Gun4", NULL);
   gun5_device = input_device_add(machine, DEVICE_CLASS_LIGHTGUN, "Gun5", NULL);
   gun6_device = input_device_add(machine, DEVICE_CLASS_LIGHTGUN, "Gun6", NULL);
   gun7_device = input_device_add(machine, DEVICE_CLASS_LIGHTGUN, "Gun7", NULL);
   gun8_device = input_device_add(machine, DEVICE_CLASS_LIGHTGUN, "Gun8", NULL);

   // add the lightgun axes
   input_device_item_add_gun1(gun1_device, "X", &gun1X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_gun1(gun1_device, "Y", &gun1Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_gun2(gun2_device, "X", &gun2X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_gun2(gun2_device, "Y", &gun2Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_gun3(gun3_device, "X", &gun3X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_gun3(gun3_device, "Y", &gun3Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_gun4(gun4_device, "X", &gun4X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_gun4(gun4_device, "Y", &gun4Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_gun5(gun5_device, "X", &gun5X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_gun5(gun5_device, "Y", &gun5Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_gun6(gun6_device, "X", &gun6X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_gun6(gun6_device, "Y", &gun6Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_gun7(gun7_device, "X", &gun7X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_gun7(gun7_device, "Y", &gun7Y, ITEM_ID_YAXIS, generic_axis_get_state);
   input_device_item_add_gun8(gun8_device, "X", &gun8X, ITEM_ID_XAXIS, generic_axis_get_state);
   input_device_item_add_gun8(gun8_device, "Y", &gun8Y, ITEM_ID_YAXIS, generic_axis_get_state);	
   
   // our faux keyboard only has a couple of keys (corresponding to the common defaults
   P1_device = input_device_add(machine, DEVICE_CLASS_JOYSTICK, "Retropad1", NULL);
   P2_device = input_device_add(machine, DEVICE_CLASS_JOYSTICK, "Retropad2", NULL);
   P3_device = input_device_add(machine, DEVICE_CLASS_JOYSTICK, "Retropad3", NULL);
   P4_device = input_device_add(machine, DEVICE_CLASS_JOYSTICK, "Retropad4", NULL);
   P5_device = input_device_add(machine, DEVICE_CLASS_JOYSTICK, "Retropad5", NULL);
   P6_device = input_device_add(machine, DEVICE_CLASS_JOYSTICK, "Retropad6", NULL);
   P7_device = input_device_add(machine, DEVICE_CLASS_JOYSTICK, "Retropad7", NULL);
   P8_device = input_device_add(machine, DEVICE_CLASS_JOYSTICK, "Retropad8", NULL);

   if (P1_device == NULL)
      fatalerror("P1 Error creating retropad device\n");

   if (P2_device == NULL)
      fatalerror("P2 Error creating retropad device\n");

   if (P3_device == NULL)
      fatalerror("P3 Error creating retropad device\n");

   if (P4_device == NULL)
      fatalerror("P4 Error creating retropad device\n");
	
   if (P5_device == NULL)
      fatalerror("P5 Error creating retropad device\n");

   if (P6_device == NULL)
      fatalerror("P6 Error creating retropad device\n");

   if (P7_device == NULL)
      fatalerror("P7 Error creating retropad device\n");

   if (P8_device == NULL)
      fatalerror("P8 Error creating retropad device\n");
 
   retro_log(RETRO_LOG_INFO, "[MAME 2010] SOURCE FILE: %s\n", machine->gamedrv->source_file);
   retro_log(RETRO_LOG_INFO, "[MAME 2010] PARENT: %s\n", machine->gamedrv->parent);
   retro_log(RETRO_LOG_INFO, "[MAME 2010] NAME: %s\n", machine->gamedrv->name);
   retro_log(RETRO_LOG_INFO, "[MAME 2010] DESCRIPTION: %s\n", machine->gamedrv->description);
   retro_log(RETRO_LOG_INFO, "[MAME 2010] YEAR: %s\n", machine->gamedrv->year);
   retro_log(RETRO_LOG_INFO, "[MAME 2010] MANUFACTURER: %s\n", machine->gamedrv->manufacturer);
 
   P1_state[KEY_BUTTON_9]   = 0;//RETRO_DEVICE_ID_JOYPAD_R3
   P1_state[KEY_BUTTON_8]   = 0;//RETRO_DEVICE_ID_JOYPAD_L3
   P1_state[KEY_START]      = 0;//RETRO_DEVICE_ID_JOYPAD_START
   P1_state[KEY_COIN]       = 0;//RETRO_DEVICE_ID_JOYPAD_SELECT
   P1_state[KEY_BUTTON_1]   = 0;//RETRO_DEVICE_ID_JOYPAD_A
   P1_state[KEY_BUTTON_2]   = 0;//RETRO_DEVICE_ID_JOYPAD_B
   P1_state[KEY_BUTTON_3]   = 0;//RETRO_DEVICE_ID_JOYPAD_X
   P1_state[KEY_BUTTON_4]   = 0;//RETRO_DEVICE_ID_JOYPAD_Y
   P1_state[KEY_BUTTON_5]   = 0;//RETRO_DEVICE_ID_JOYPAD_L
   P1_state[KEY_BUTTON_6]   = 0;//RETRO_DEVICE_ID_JOYPAD_R
   P1_state[KEY_BUTTON_7]   = 0;//RETRO_DEVICE_ID_JOYPAD_L2
   P1_state[KEY_JOYSTICK_U] = 0;//RETRO_DEVICE_ID_JOYPAD_UP
   P1_state[KEY_JOYSTICK_D] = 0;//RETRO_DEVICE_ID_JOYPAD_DOWN
   P1_state[KEY_JOYSTICK_L] = 0;//RETRO_DEVICE_ID_JOYPAD_LEFT
   P1_state[KEY_JOYSTICK_R] = 0;//RETRO_DEVICE_ID_JOYPAD_RIGHT
   P1_state[LX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_X
   P1_state[LY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_Y
   P1_state[RX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_X
   P1_state[RY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_Y

   P2_state[KEY_START]      = 0;//RETRO_DEVICE_ID_JOYPAD_START
   P2_state[KEY_COIN]       = 0;//RETRO_DEVICE_ID_JOYPAD_SELECT
   P2_state[KEY_BUTTON_1]   = 0;//RETRO_DEVICE_ID_JOYPAD_A
   P2_state[KEY_BUTTON_2]   = 0;//RETRO_DEVICE_ID_JOYPAD_B
   P2_state[KEY_BUTTON_3]   = 0;//RETRO_DEVICE_ID_JOYPAD_X
   P2_state[KEY_BUTTON_4]   = 0;//RETRO_DEVICE_ID_JOYPAD_Y
   P2_state[KEY_BUTTON_5]   = 0;//RETRO_DEVICE_ID_JOYPAD_L
   P2_state[KEY_BUTTON_6]   = 0;//RETRO_DEVICE_ID_JOYPAD_R
   P2_state[KEY_BUTTON_7]   = 0;//RETRO_DEVICE_ID_JOYPAD_L2
   P2_state[KEY_JOYSTICK_U] = 0;//RETRO_DEVICE_ID_JOYPAD_UP
   P2_state[KEY_JOYSTICK_D] = 0;//RETRO_DEVICE_ID_JOYPAD_DOWN
   P2_state[KEY_JOYSTICK_L] = 0;//RETRO_DEVICE_ID_JOYPAD_LEFT
   P2_state[KEY_JOYSTICK_R] = 0;//RETRO_DEVICE_ID_JOYPAD_RIGHT
   P2_state[LX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_X
   P2_state[LY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_Y
   P2_state[RX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_X
   P2_state[RY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_Y
	
   P3_state[KEY_START]      = 0;//RETRO_DEVICE_ID_JOYPAD_START
   P3_state[KEY_COIN]       = 0;//RETRO_DEVICE_ID_JOYPAD_SELECT
   P3_state[KEY_BUTTON_1]   = 0;//RETRO_DEVICE_ID_JOYPAD_A
   P3_state[KEY_BUTTON_2]   = 0;//RETRO_DEVICE_ID_JOYPAD_B
   P3_state[KEY_BUTTON_3]   = 0;//RETRO_DEVICE_ID_JOYPAD_X
   P3_state[KEY_BUTTON_4]   = 0;//RETRO_DEVICE_ID_JOYPAD_Y
   P3_state[KEY_BUTTON_5]   = 0;//RETRO_DEVICE_ID_JOYPAD_L
   P3_state[KEY_BUTTON_6]   = 0;//RETRO_DEVICE_ID_JOYPAD_R
   P3_state[KEY_BUTTON_7]   = 0;//RETRO_DEVICE_ID_JOYPAD_L2
   P3_state[KEY_JOYSTICK_U] = 0;//RETRO_DEVICE_ID_JOYPAD_UP
   P3_state[KEY_JOYSTICK_D] = 0;//RETRO_DEVICE_ID_JOYPAD_DOWN
   P3_state[KEY_JOYSTICK_L] = 0;//RETRO_DEVICE_ID_JOYPAD_LEFT
   P3_state[KEY_JOYSTICK_R] = 0;//RETRO_DEVICE_ID_JOYPAD_RIGHT
   P3_state[LX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_X
   P3_state[LY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_Y
   P3_state[RX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_X
   P3_state[RY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_Y
	
   P4_state[KEY_START]      = 0;//RETRO_DEVICE_ID_JOYPAD_START
   P4_state[KEY_COIN]       = 0;//RETRO_DEVICE_ID_JOYPAD_SELECT
   P4_state[KEY_BUTTON_1]   = 0;//RETRO_DEVICE_ID_JOYPAD_A
   P4_state[KEY_BUTTON_2]   = 0;//RETRO_DEVICE_ID_JOYPAD_B
   P4_state[KEY_BUTTON_3]   = 0;//RETRO_DEVICE_ID_JOYPAD_X
   P4_state[KEY_BUTTON_4]   = 0;//RETRO_DEVICE_ID_JOYPAD_Y
   P4_state[KEY_BUTTON_5]   = 0;//RETRO_DEVICE_ID_JOYPAD_L
   P4_state[KEY_BUTTON_6]   = 0;//RETRO_DEVICE_ID_JOYPAD_R
   P4_state[KEY_BUTTON_7]   = 0;//RETRO_DEVICE_ID_JOYPAD_L2
   P4_state[KEY_JOYSTICK_U] = 0;//RETRO_DEVICE_ID_JOYPAD_UP
   P4_state[KEY_JOYSTICK_D] = 0;//RETRO_DEVICE_ID_JOYPAD_DOWN
   P4_state[KEY_JOYSTICK_L] = 0;//RETRO_DEVICE_ID_JOYPAD_LEFT
   P4_state[KEY_JOYSTICK_R] = 0;//RETRO_DEVICE_ID_JOYPAD_RIGHT 
   P4_state[LX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_X
   P4_state[LY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_Y
   P4_state[RX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_X
   P4_state[RY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_Y
	
   P5_state[KEY_START]      = 0;//RETRO_DEVICE_ID_JOYPAD_START
   P5_state[KEY_COIN]       = 0;//RETRO_DEVICE_ID_JOYPAD_SELECT
   P5_state[KEY_BUTTON_1]   = 0;//RETRO_DEVICE_ID_JOYPAD_A
   P5_state[KEY_BUTTON_2]   = 0;//RETRO_DEVICE_ID_JOYPAD_B
   P5_state[KEY_BUTTON_3]   = 0;//RETRO_DEVICE_ID_JOYPAD_X
   P5_state[KEY_BUTTON_4]   = 0;//RETRO_DEVICE_ID_JOYPAD_Y
   P5_state[KEY_BUTTON_5]   = 0;//RETRO_DEVICE_ID_JOYPAD_L
   P5_state[KEY_BUTTON_6]   = 0;//RETRO_DEVICE_ID_JOYPAD_R
   P5_state[KEY_BUTTON_7]   = 0;//RETRO_DEVICE_ID_JOYPAD_L2
   P5_state[KEY_JOYSTICK_U] = 0;//RETRO_DEVICE_ID_JOYPAD_UP
   P5_state[KEY_JOYSTICK_D] = 0;//RETRO_DEVICE_ID_JOYPAD_DOWN
   P5_state[KEY_JOYSTICK_L] = 0;//RETRO_DEVICE_ID_JOYPAD_LEFT
   P5_state[KEY_JOYSTICK_R] = 0;//RETRO_DEVICE_ID_JOYPAD_RIGHT 
   P5_state[LX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_X
   P5_state[LY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_Y
   P5_state[RX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_X
   P5_state[RY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_Y
	
   P6_state[KEY_START]      = 0;//RETRO_DEVICE_ID_JOYPAD_START
   P6_state[KEY_COIN]       = 0;//RETRO_DEVICE_ID_JOYPAD_SELECT
   P6_state[KEY_BUTTON_1]   = 0;//RETRO_DEVICE_ID_JOYPAD_A
   P6_state[KEY_BUTTON_2]   = 0;//RETRO_DEVICE_ID_JOYPAD_B
   P6_state[KEY_BUTTON_3]   = 0;//RETRO_DEVICE_ID_JOYPAD_X
   P6_state[KEY_BUTTON_4]   = 0;//RETRO_DEVICE_ID_JOYPAD_Y
   P6_state[KEY_BUTTON_5]   = 0;//RETRO_DEVICE_ID_JOYPAD_L
   P6_state[KEY_BUTTON_6]   = 0;//RETRO_DEVICE_ID_JOYPAD_R
   P6_state[KEY_BUTTON_7]   = 0;//RETRO_DEVICE_ID_JOYPAD_L2
   P6_state[KEY_JOYSTICK_U] = 0;//RETRO_DEVICE_ID_JOYPAD_UP
   P6_state[KEY_JOYSTICK_D] = 0;//RETRO_DEVICE_ID_JOYPAD_DOWN
   P6_state[KEY_JOYSTICK_L] = 0;//RETRO_DEVICE_ID_JOYPAD_LEFT
   P6_state[KEY_JOYSTICK_R] = 0;//RETRO_DEVICE_ID_JOYPAD_RIGHT 
   P6_state[LX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_X
   P6_state[LY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_Y
   P6_state[RX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_X
   P6_state[RY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_Y
	
   P7_state[KEY_START]      = 0;//RETRO_DEVICE_ID_JOYPAD_START
   P7_state[KEY_COIN]       = 0;//RETRO_DEVICE_ID_JOYPAD_SELECT
   P7_state[KEY_BUTTON_1]   = 0;//RETRO_DEVICE_ID_JOYPAD_A
   P7_state[KEY_BUTTON_2]   = 0;//RETRO_DEVICE_ID_JOYPAD_B
   P7_state[KEY_BUTTON_3]   = 0;//RETRO_DEVICE_ID_JOYPAD_X
   P7_state[KEY_BUTTON_4]   = 0;//RETRO_DEVICE_ID_JOYPAD_Y
   P7_state[KEY_BUTTON_5]   = 0;//RETRO_DEVICE_ID_JOYPAD_L
   P7_state[KEY_BUTTON_6]   = 0;//RETRO_DEVICE_ID_JOYPAD_R
   P7_state[KEY_BUTTON_7]   = 0;//RETRO_DEVICE_ID_JOYPAD_L2
   P7_state[KEY_JOYSTICK_U] = 0;//RETRO_DEVICE_ID_JOYPAD_UP
   P7_state[KEY_JOYSTICK_D] = 0;//RETRO_DEVICE_ID_JOYPAD_DOWN
   P7_state[KEY_JOYSTICK_L] = 0;//RETRO_DEVICE_ID_JOYPAD_LEFT
   P7_state[KEY_JOYSTICK_R] = 0;//RETRO_DEVICE_ID_JOYPAD_RIGHT 
   P7_state[LX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_X
   P7_state[LY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_Y
   P7_state[RX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_X
   P7_state[RY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_Y
	
   P8_state[KEY_START]      = 0;//RETRO_DEVICE_ID_JOYPAD_START
   P8_state[KEY_COIN]       = 0;//RETRO_DEVICE_ID_JOYPAD_SELECT
   P8_state[KEY_BUTTON_1]   = 0;//RETRO_DEVICE_ID_JOYPAD_A
   P8_state[KEY_BUTTON_2]   = 0;//RETRO_DEVICE_ID_JOYPAD_B
   P8_state[KEY_BUTTON_3]   = 0;//RETRO_DEVICE_ID_JOYPAD_X
   P8_state[KEY_BUTTON_4]   = 0;//RETRO_DEVICE_ID_JOYPAD_Y
   P8_state[KEY_BUTTON_5]   = 0;//RETRO_DEVICE_ID_JOYPAD_L
   P8_state[KEY_BUTTON_6]   = 0;//RETRO_DEVICE_ID_JOYPAD_R
   P8_state[KEY_BUTTON_7]   = 0;//RETRO_DEVICE_ID_JOYPAD_L2
   P8_state[KEY_JOYSTICK_U] = 0;//RETRO_DEVICE_ID_JOYPAD_UP
   P8_state[KEY_JOYSTICK_D] = 0;//RETRO_DEVICE_ID_JOYPAD_DOWN
   P8_state[KEY_JOYSTICK_L] = 0;//RETRO_DEVICE_ID_JOYPAD_LEFT
   P8_state[KEY_JOYSTICK_R] = 0;//RETRO_DEVICE_ID_JOYPAD_RIGHT 
   P8_state[LX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_X
   P8_state[LY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_LEFT RETRO_DEVICE_ID_ANALOG_Y
   P8_state[RX] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_X
   P8_state[RY] = 0;            //RETRO_DEVICE_INDEX_ANALOG_RIGHT RETRO_DEVICE_ID_ANALOG_Y
	
   input_device_item_add_p1(P1_device, "LX",       &P1_state[LX],             ITEM_ID_XAXIS,     generic_axis_get_state);
   input_device_item_add_p1(P1_device, "LY",       &P1_state[LY],             ITEM_ID_YAXIS,     generic_axis_get_state);
   input_device_item_add_p1(P1_device, "RX",       &P1_state[RX],             ITEM_ID_RXAXIS,    generic_axis_get_state);
   input_device_item_add_p1(P1_device, "RY",       &P1_state[RY],             ITEM_ID_RYAXIS,    generic_axis_get_state);
   input_device_item_add_p1(P1_device, "P1 Start", &P1_state[KEY_START],      ITEM_ID_START,     pad1_get_state);
   input_device_item_add_p1(P1_device, "COIN1",    &P1_state[KEY_COIN],       ITEM_ID_SELECT,    pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 Up",    &P1_state[KEY_JOYSTICK_U], ITEM_ID_DPADUP,    pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 Down",  &P1_state[KEY_JOYSTICK_D], ITEM_ID_DPADDOWN,  pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 Left",  &P1_state[KEY_JOYSTICK_L], ITEM_ID_DPADLEFT,  pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 Right", &P1_state[KEY_JOYSTICK_R], ITEM_ID_DPADRIGHT, pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 B1",    &P1_state[KEY_BUTTON_1],   ITEM_ID_BUTTON1,   pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 B2",    &P1_state[KEY_BUTTON_2],   ITEM_ID_BUTTON2,   pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 B3",    &P1_state[KEY_BUTTON_3],   ITEM_ID_BUTTON3,   pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 B4",    &P1_state[KEY_BUTTON_4],   ITEM_ID_BUTTON4,   pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 B5",    &P1_state[KEY_BUTTON_5],   ITEM_ID_BUTTON5,   pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 B6",    &P1_state[KEY_BUTTON_6],   ITEM_ID_BUTTON6,   pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 B7",    &P1_state[KEY_BUTTON_7],   ITEM_ID_BUTTON7,   pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 B8",    &P1_state[KEY_BUTTON_8],   ITEM_ID_BUTTON8,   pad1_get_state);
   input_device_item_add_p1(P1_device, "P1 B9",    &P1_state[KEY_BUTTON_9],   ITEM_ID_BUTTON9,   pad1_get_state);

   input_device_item_add_p2(P2_device, "LX",       &P2_state[LX],             ITEM_ID_XAXIS,     generic_axis_get_state);
   input_device_item_add_p2(P2_device, "LY",       &P2_state[LY],             ITEM_ID_YAXIS,     generic_axis_get_state);
   input_device_item_add_p2(P2_device, "RX",       &P2_state[RX],             ITEM_ID_RXAXIS,    generic_axis_get_state);
   input_device_item_add_p2(P2_device, "RY",       &P2_state[RY],             ITEM_ID_RYAXIS,    generic_axis_get_state);
   input_device_item_add_p2(P2_device, "P2 Start", &P2_state[KEY_START],      ITEM_ID_START,     pad2_get_state);
   input_device_item_add_p2(P2_device, "COIN2",    &P2_state[KEY_COIN],       ITEM_ID_SELECT,    pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 Up",    &P2_state[KEY_JOYSTICK_U], ITEM_ID_DPADUP,    pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 Down",  &P2_state[KEY_JOYSTICK_D], ITEM_ID_DPADDOWN,  pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 Left",  &P2_state[KEY_JOYSTICK_L], ITEM_ID_DPADLEFT,  pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 Right", &P2_state[KEY_JOYSTICK_R], ITEM_ID_DPADRIGHT, pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 B1",    &P2_state[KEY_BUTTON_1],   ITEM_ID_BUTTON1,   pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 B2",    &P2_state[KEY_BUTTON_2],   ITEM_ID_BUTTON2,   pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 B3",    &P2_state[KEY_BUTTON_3],   ITEM_ID_BUTTON3,   pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 B4",    &P2_state[KEY_BUTTON_4],   ITEM_ID_BUTTON4,   pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 B5",    &P2_state[KEY_BUTTON_5],   ITEM_ID_BUTTON5,   pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 B6",    &P2_state[KEY_BUTTON_6],   ITEM_ID_BUTTON6,   pad2_get_state);
   input_device_item_add_p2(P2_device, "P2 B7",    &P2_state[KEY_BUTTON_7],   ITEM_ID_BUTTON7,   pad2_get_state); 

   input_device_item_add_p3(P3_device, "LX",       &P3_state[LX],             ITEM_ID_XAXIS,     generic_axis_get_state);
   input_device_item_add_p3(P3_device, "LY",       &P3_state[LY],             ITEM_ID_YAXIS,     generic_axis_get_state);
   input_device_item_add_p3(P3_device, "RX",       &P3_state[RX],             ITEM_ID_RXAXIS,    generic_axis_get_state);
   input_device_item_add_p3(P3_device, "RY",       &P3_state[RY],             ITEM_ID_RYAXIS,    generic_axis_get_state);
   input_device_item_add_p3(P3_device, "P3 Start", &P3_state[KEY_START],      ITEM_ID_START,     pad3_get_state);
   input_device_item_add_p3(P3_device, "COIN3",    &P3_state[KEY_COIN],       ITEM_ID_SELECT,    pad3_get_state);
   input_device_item_add_p3(P3_device, "P3 Up",    &P3_state[KEY_JOYSTICK_U], ITEM_ID_DPADUP,    pad3_get_state);
   input_device_item_add_p3(P3_device, "P3 Down",  &P3_state[KEY_JOYSTICK_D], ITEM_ID_DPADDOWN,  pad3_get_state);
   input_device_item_add_p3(P3_device, "P3 Left",  &P3_state[KEY_JOYSTICK_L], ITEM_ID_DPADLEFT,  pad3_get_state);
   input_device_item_add_p3(P3_device, "P3 Right", &P3_state[KEY_JOYSTICK_R], ITEM_ID_DPADRIGHT, pad3_get_state);
   input_device_item_add_p3(P3_device, "P3 B1",    &P3_state[KEY_BUTTON_1],   ITEM_ID_BUTTON1,   pad3_get_state);
   input_device_item_add_p3(P3_device, "P3 B2",    &P3_state[KEY_BUTTON_2],   ITEM_ID_BUTTON2,   pad3_get_state);
   input_device_item_add_p3(P3_device, "P3 B3",    &P3_state[KEY_BUTTON_3],   ITEM_ID_BUTTON3,   pad3_get_state);
   input_device_item_add_p3(P3_device, "P3 B4",    &P3_state[KEY_BUTTON_4],   ITEM_ID_BUTTON4,   pad3_get_state);
   input_device_item_add_p3(P3_device, "P3 B5",    &P3_state[KEY_BUTTON_5],   ITEM_ID_BUTTON5,   pad3_get_state);
   input_device_item_add_p3(P3_device, "P3 B6",    &P3_state[KEY_BUTTON_6],   ITEM_ID_BUTTON6,   pad3_get_state);
   input_device_item_add_p3(P3_device, "P3 B7",    &P3_state[KEY_BUTTON_7],   ITEM_ID_BUTTON7,   pad3_get_state); 

   input_device_item_add_p4(P4_device, "LX",       &P4_state[LX],             ITEM_ID_XAXIS,     generic_axis_get_state);
   input_device_item_add_p4(P4_device, "LY",       &P4_state[LY],             ITEM_ID_YAXIS,     generic_axis_get_state);
   input_device_item_add_p4(P4_device, "RX",       &P4_state[RX],             ITEM_ID_RXAXIS,    generic_axis_get_state);
   input_device_item_add_p4(P4_device, "RY",       &P4_state[RY],             ITEM_ID_RYAXIS,    generic_axis_get_state);
   input_device_item_add_p4(P4_device, "P4 Start", &P4_state[KEY_START],      ITEM_ID_START,     pad4_get_state);
   input_device_item_add_p4(P4_device, "COIN4",    &P4_state[KEY_COIN],       ITEM_ID_SELECT,    pad4_get_state);
   input_device_item_add_p4(P4_device, "P4 Up",    &P4_state[KEY_JOYSTICK_U], ITEM_ID_DPADUP,    pad4_get_state);
   input_device_item_add_p4(P4_device, "P4 Down",  &P4_state[KEY_JOYSTICK_D], ITEM_ID_DPADDOWN,  pad4_get_state);
   input_device_item_add_p4(P4_device, "P4 Left",  &P4_state[KEY_JOYSTICK_L], ITEM_ID_DPADLEFT,  pad4_get_state);
   input_device_item_add_p4(P4_device, "P4 Right", &P4_state[KEY_JOYSTICK_R], ITEM_ID_DPADRIGHT, pad4_get_state);
   input_device_item_add_p4(P4_device, "P4 B1",    &P4_state[KEY_BUTTON_1],   ITEM_ID_BUTTON1,   pad4_get_state);
   input_device_item_add_p4(P4_device, "P4 B2",    &P4_state[KEY_BUTTON_2],   ITEM_ID_BUTTON2,   pad4_get_state);
   input_device_item_add_p4(P4_device, "P4 B3",    &P4_state[KEY_BUTTON_3],   ITEM_ID_BUTTON3,   pad4_get_state);
   input_device_item_add_p4(P4_device, "P4 B4",    &P4_state[KEY_BUTTON_4],   ITEM_ID_BUTTON4,   pad4_get_state);
   input_device_item_add_p4(P4_device, "P4 B5",    &P4_state[KEY_BUTTON_5],   ITEM_ID_BUTTON5,   pad4_get_state);
   input_device_item_add_p4(P4_device, "P4 B6",    &P4_state[KEY_BUTTON_6],   ITEM_ID_BUTTON6,   pad4_get_state);
   input_device_item_add_p4(P4_device, "P4 B7",    &P4_state[KEY_BUTTON_7],   ITEM_ID_BUTTON7,   pad4_get_state); 
	
   input_device_item_add_p5(P5_device, "LX",       &P5_state[LX],             ITEM_ID_XAXIS,     generic_axis_get_state);
   input_device_item_add_p5(P5_device, "LY",       &P5_state[LY],             ITEM_ID_YAXIS,     generic_axis_get_state);
   input_device_item_add_p5(P5_device, "RX",       &P5_state[RX],             ITEM_ID_RXAXIS,    generic_axis_get_state);
   input_device_item_add_p5(P5_device, "RY",       &P5_state[RY],             ITEM_ID_RYAXIS,    generic_axis_get_state);
   input_device_item_add_p5(P5_device, "P5 Start", &P5_state[KEY_START],      ITEM_ID_START,     pad5_get_state);
   input_device_item_add_p5(P5_device, "COIN5",    &P5_state[KEY_COIN],       ITEM_ID_SELECT,    pad5_get_state);
   input_device_item_add_p5(P5_device, "P5 Up",    &P5_state[KEY_JOYSTICK_U], ITEM_ID_DPADUP,    pad5_get_state);
   input_device_item_add_p5(P5_device, "P5 Down",  &P5_state[KEY_JOYSTICK_D], ITEM_ID_DPADDOWN,  pad5_get_state);
   input_device_item_add_p5(P5_device, "P5 Left",  &P5_state[KEY_JOYSTICK_L], ITEM_ID_DPADLEFT,  pad5_get_state);
   input_device_item_add_p5(P5_device, "P5 Right", &P5_state[KEY_JOYSTICK_R], ITEM_ID_DPADRIGHT, pad5_get_state);
   input_device_item_add_p5(P5_device, "P5 B1",    &P5_state[KEY_BUTTON_1],   ITEM_ID_BUTTON1,   pad5_get_state);
   input_device_item_add_p5(P5_device, "P5 B2",    &P5_state[KEY_BUTTON_2],   ITEM_ID_BUTTON2,   pad5_get_state);
   input_device_item_add_p5(P5_device, "P5 B3",    &P5_state[KEY_BUTTON_3],   ITEM_ID_BUTTON3,   pad5_get_state);
   input_device_item_add_p5(P5_device, "P5 B4",    &P5_state[KEY_BUTTON_4],   ITEM_ID_BUTTON4,   pad5_get_state);
   input_device_item_add_p5(P5_device, "P5 B5",    &P5_state[KEY_BUTTON_5],   ITEM_ID_BUTTON5,   pad5_get_state);
   input_device_item_add_p5(P5_device, "P5 B6",    &P5_state[KEY_BUTTON_6],   ITEM_ID_BUTTON6,   pad5_get_state);
   input_device_item_add_p5(P5_device, "P5 B7",    &P5_state[KEY_BUTTON_7],   ITEM_ID_BUTTON7,   pad5_get_state); 
	
   input_device_item_add_p6(P6_device, "LX",       &P6_state[LX],             ITEM_ID_XAXIS,     generic_axis_get_state);
   input_device_item_add_p6(P6_device, "LY",       &P6_state[LY],             ITEM_ID_YAXIS,     generic_axis_get_state);
   input_device_item_add_p6(P6_device, "RX",       &P6_state[RX],             ITEM_ID_RXAXIS,    generic_axis_get_state);
   input_device_item_add_p6(P6_device, "RY",       &P6_state[RY],             ITEM_ID_RYAXIS,    generic_axis_get_state);
   input_device_item_add_p6(P6_device, "P6 Start", &P6_state[KEY_START],      ITEM_ID_START,     pad6_get_state);
   input_device_item_add_p6(P6_device, "COIN6",    &P6_state[KEY_COIN],       ITEM_ID_SELECT,    pad6_get_state);
   input_device_item_add_p6(P6_device, "P6 Up",    &P6_state[KEY_JOYSTICK_U], ITEM_ID_DPADUP,    pad6_get_state);
   input_device_item_add_p6(P6_device, "P6 Down",  &P6_state[KEY_JOYSTICK_D], ITEM_ID_DPADDOWN,  pad6_get_state);
   input_device_item_add_p6(P6_device, "P6 Left",  &P6_state[KEY_JOYSTICK_L], ITEM_ID_DPADLEFT,  pad6_get_state);
   input_device_item_add_p6(P6_device, "P6 Right", &P6_state[KEY_JOYSTICK_R], ITEM_ID_DPADRIGHT, pad6_get_state);
   input_device_item_add_p6(P6_device, "P6 B1",    &P6_state[KEY_BUTTON_1],   ITEM_ID_BUTTON1,   pad6_get_state);
   input_device_item_add_p6(P6_device, "P6 B2",    &P6_state[KEY_BUTTON_2],   ITEM_ID_BUTTON2,   pad6_get_state);
   input_device_item_add_p6(P6_device, "P6 B3",    &P6_state[KEY_BUTTON_3],   ITEM_ID_BUTTON3,   pad6_get_state);
   input_device_item_add_p6(P6_device, "P6 B4",    &P6_state[KEY_BUTTON_4],   ITEM_ID_BUTTON4,   pad6_get_state);
   input_device_item_add_p6(P6_device, "P6 B5",    &P6_state[KEY_BUTTON_5],   ITEM_ID_BUTTON5,   pad6_get_state);
   input_device_item_add_p6(P6_device, "P6 B6",    &P6_state[KEY_BUTTON_6],   ITEM_ID_BUTTON6,   pad6_get_state);
   input_device_item_add_p6(P6_device, "P6 B7",    &P6_state[KEY_BUTTON_7],   ITEM_ID_BUTTON7,   pad6_get_state); 

   input_device_item_add_p7(P7_device, "LX",       &P7_state[LX],             ITEM_ID_XAXIS,     generic_axis_get_state);
   input_device_item_add_p7(P7_device, "LY",       &P7_state[LY],             ITEM_ID_YAXIS,     generic_axis_get_state);
   input_device_item_add_p7(P7_device, "RX",       &P7_state[RX],             ITEM_ID_RXAXIS,    generic_axis_get_state);
   input_device_item_add_p7(P7_device, "RY",       &P7_state[RY],             ITEM_ID_RYAXIS,    generic_axis_get_state);
   input_device_item_add_p7(P7_device, "P7 Start", &P7_state[KEY_START],      ITEM_ID_START,     pad7_get_state);
   input_device_item_add_p7(P7_device, "COIN7",    &P7_state[KEY_COIN],       ITEM_ID_SELECT,    pad7_get_state);
   input_device_item_add_p7(P7_device, "P7 Up",    &P7_state[KEY_JOYSTICK_U], ITEM_ID_DPADUP,    pad7_get_state);
   input_device_item_add_p7(P7_device, "P7 Down",  &P7_state[KEY_JOYSTICK_D], ITEM_ID_DPADDOWN,  pad7_get_state);
   input_device_item_add_p7(P7_device, "P7 Left",  &P7_state[KEY_JOYSTICK_L], ITEM_ID_DPADLEFT,  pad7_get_state);
   input_device_item_add_p7(P7_device, "P7 Right", &P7_state[KEY_JOYSTICK_R], ITEM_ID_DPADRIGHT, pad7_get_state);
   input_device_item_add_p7(P7_device, "P7 B1",    &P7_state[KEY_BUTTON_1],   ITEM_ID_BUTTON1,   pad7_get_state);
   input_device_item_add_p7(P7_device, "P7 B2",    &P7_state[KEY_BUTTON_2],   ITEM_ID_BUTTON2,   pad7_get_state);
   input_device_item_add_p7(P7_device, "P7 B3",    &P7_state[KEY_BUTTON_3],   ITEM_ID_BUTTON3,   pad7_get_state);
   input_device_item_add_p7(P7_device, "P7 B4",    &P7_state[KEY_BUTTON_4],   ITEM_ID_BUTTON4,   pad7_get_state);
   input_device_item_add_p7(P7_device, "P7 B5",    &P7_state[KEY_BUTTON_5],   ITEM_ID_BUTTON5,   pad7_get_state);
   input_device_item_add_p7(P7_device, "P7 B6",    &P7_state[KEY_BUTTON_6],   ITEM_ID_BUTTON6,   pad7_get_state);
   input_device_item_add_p7(P7_device, "P7 B7",    &P7_state[KEY_BUTTON_7],   ITEM_ID_BUTTON7,   pad7_get_state); 

   input_device_item_add_p8(P8_device, "LX",       &P8_state[LX],             ITEM_ID_XAXIS,     generic_axis_get_state);
   input_device_item_add_p8(P8_device, "LY",       &P8_state[LY],             ITEM_ID_YAXIS,     generic_axis_get_state);
   input_device_item_add_p8(P8_device, "RX",       &P8_state[RX],             ITEM_ID_RXAXIS,    generic_axis_get_state);
   input_device_item_add_p8(P8_device, "RY",       &P8_state[RY],             ITEM_ID_RYAXIS,    generic_axis_get_state);
   input_device_item_add_p8(P8_device, "P8 Start", &P8_state[KEY_START],      ITEM_ID_START,     pad8_get_state);
   input_device_item_add_p8(P8_device, "COIN8",    &P8_state[KEY_COIN],       ITEM_ID_SELECT,    pad8_get_state);
   input_device_item_add_p8(P8_device, "P8 Up",    &P8_state[KEY_JOYSTICK_U], ITEM_ID_DPADUP,    pad8_get_state);
   input_device_item_add_p8(P8_device, "P8 Down",  &P8_state[KEY_JOYSTICK_D], ITEM_ID_DPADDOWN,  pad8_get_state);
   input_device_item_add_p8(P8_device, "P8 Left",  &P8_state[KEY_JOYSTICK_L], ITEM_ID_DPADLEFT,  pad8_get_state);
   input_device_item_add_p8(P8_device, "P8 Right", &P8_state[KEY_JOYSTICK_R], ITEM_ID_DPADRIGHT, pad8_get_state);
   input_device_item_add_p8(P8_device, "P8 B1",    &P8_state[KEY_BUTTON_1],   ITEM_ID_BUTTON1,   pad8_get_state);
   input_device_item_add_p8(P8_device, "P8 B2",    &P8_state[KEY_BUTTON_2],   ITEM_ID_BUTTON2,   pad8_get_state);
   input_device_item_add_p8(P8_device, "P8 B3",    &P8_state[KEY_BUTTON_3],   ITEM_ID_BUTTON3,   pad8_get_state);
   input_device_item_add_p8(P8_device, "P8 B4",    &P8_state[KEY_BUTTON_4],   ITEM_ID_BUTTON4,   pad8_get_state);
   input_device_item_add_p8(P8_device, "P8 B5",    &P8_state[KEY_BUTTON_5],   ITEM_ID_BUTTON5,   pad8_get_state);
   input_device_item_add_p8(P8_device, "P8 B6",    &P8_state[KEY_BUTTON_6],   ITEM_ID_BUTTON6,   pad8_get_state);
   input_device_item_add_p8(P8_device, "P8 B7",    &P8_state[KEY_BUTTON_7],   ITEM_ID_BUTTON7,   pad8_get_state); 

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

   if (mouse_mode == 2)
   {
   gun1X = 2 * (input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X));
   gun1Y = 2 * (input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y));
   gun2X = 2 * (input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X));
   gun2Y = 2 * (input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y));
   gun3X = 2 * (input_state_cb(2, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X));
   gun3Y = 2 * (input_state_cb(2, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y));
   gun4X = 2 * (input_state_cb(3, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X));
   gun4Y = 2 * (input_state_cb(3, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y));
   gun5X = 2 * (input_state_cb(4, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X));
   gun5Y = 2 * (input_state_cb(4, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y));
   gun6X = 2 * (input_state_cb(5, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X));
   gun6Y = 2 * (input_state_cb(5, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y));
   gun7X = 2 * (input_state_cb(6, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X));
   gun7Y = 2 * (input_state_cb(6, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y));
   gun8X = 2 * (input_state_cb(7, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X));
   gun8Y = 2 * (input_state_cb(7, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y));
   }
	
   if (mouse_mode == 1)
   {
   mouse1X = (input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X))*(INPUT_RELATIVE_PER_PIXEL);
   mouse1Y = (input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y))*(INPUT_RELATIVE_PER_PIXEL);
   mouse2X = (input_state_cb(1, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X))*(INPUT_RELATIVE_PER_PIXEL);
   mouse2Y = (input_state_cb(1, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y))*(INPUT_RELATIVE_PER_PIXEL);
   mouse3X = (input_state_cb(2, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X))*(INPUT_RELATIVE_PER_PIXEL);
   mouse3Y = (input_state_cb(2, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y))*(INPUT_RELATIVE_PER_PIXEL);
   mouse4X = (input_state_cb(3, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X))*(INPUT_RELATIVE_PER_PIXEL);
   mouse4Y = (input_state_cb(3, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y))*(INPUT_RELATIVE_PER_PIXEL);
   mouse5X = (input_state_cb(4, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X))*(INPUT_RELATIVE_PER_PIXEL);
   mouse5Y = (input_state_cb(4, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y))*(INPUT_RELATIVE_PER_PIXEL);
   mouse6X = (input_state_cb(5, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X))*(INPUT_RELATIVE_PER_PIXEL);
   mouse6Y = (input_state_cb(5, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y))*(INPUT_RELATIVE_PER_PIXEL);
   mouse7X = (input_state_cb(6, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X))*(INPUT_RELATIVE_PER_PIXEL);
   mouse7Y = (input_state_cb(6, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y))*(INPUT_RELATIVE_PER_PIXEL);
   mouse8X = (input_state_cb(7, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X))*(INPUT_RELATIVE_PER_PIXEL);
   mouse8Y = (input_state_cb(7, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y))*(INPUT_RELATIVE_PER_PIXEL);
   }  

   P1_state[LX] = 2 * (input_state_cb(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X));
   P1_state[LY] = 2 * (input_state_cb(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y));
   P2_state[LX] = 2 * (input_state_cb(1, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X));
   P2_state[LY] = 2 * (input_state_cb(1, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y));
   P3_state[LX] = 2 * (input_state_cb(2, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X));
   P3_state[LY] = 2 * (input_state_cb(2, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y));
   P4_state[LX] = 2 * (input_state_cb(3, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X));
   P4_state[LY] = 2 * (input_state_cb(3, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y));
   P5_state[LX] = 2 * (input_state_cb(4, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X));
   P5_state[LY] = 2 * (input_state_cb(4, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y));
   P6_state[LX] = 2 * (input_state_cb(5, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X));
   P6_state[LY] = 2 * (input_state_cb(5, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y));
   P7_state[LX] = 2 * (input_state_cb(6, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X));
   P7_state[LY] = 2 * (input_state_cb(6, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y));
   P8_state[LX] = 2 * (input_state_cb(7, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X));
   P8_state[LY] = 2 * (input_state_cb(7, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y));
   
	
   P1_state[RX] = 2 * (input_state_cb(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X));
   P1_state[RY] = 2 * (input_state_cb(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y));
   P2_state[RX] = 2 * (input_state_cb(1, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X));
   P2_state[RY] = 2 * (input_state_cb(1, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y));
   P3_state[RX] = 2 * (input_state_cb(2, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X));
   P3_state[RY] = 2 * (input_state_cb(2, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y));
   P4_state[RX] = 2 * (input_state_cb(3, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X));
   P4_state[RY] = 2 * (input_state_cb(3, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y));
   P5_state[RX] = 2 * (input_state_cb(4, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X));
   P5_state[RY] = 2 * (input_state_cb(4, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y));
   P6_state[RX] = 2 * (input_state_cb(5, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X));
   P6_state[RY] = 2 * (input_state_cb(5, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y));
   P7_state[RX] = 2 * (input_state_cb(6, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X));
   P7_state[RY] = 2 * (input_state_cb(6, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y));
   P8_state[RX] = 2 * (input_state_cb(7, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X));
   P8_state[RY] = 2 * (input_state_cb(7, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y));
	
   P1_state[KEY_BUTTON_9]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3);
   P1_state[KEY_BUTTON_8]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3);
   P1_state[KEY_START]      = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START) || input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START);
   P1_state[KEY_COIN]       = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT) || input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SELECT);
   P1_state[KEY_BUTTON_1]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A) || input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER) || input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD);
   P1_state[KEY_BUTTON_2]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B) || input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_A);
   P1_state[KEY_BUTTON_3]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X) || input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_B);
   P1_state[KEY_BUTTON_4]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);
   P1_state[KEY_BUTTON_5]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
   P1_state[KEY_BUTTON_6]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);
   P1_state[KEY_BUTTON_7]   = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2);   
   P1_state[KEY_JOYSTICK_U] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
   P1_state[KEY_JOYSTICK_D] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
   P1_state[KEY_JOYSTICK_L] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
   P1_state[KEY_JOYSTICK_R] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);

   P2_state[KEY_START]      = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START) || input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START);
   P2_state[KEY_COIN]       = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT) || input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SELECT);
   P2_state[KEY_BUTTON_1]   = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A) || input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER) || input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD);
   P2_state[KEY_BUTTON_2]   = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B) || input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_A);
   P2_state[KEY_BUTTON_3]   = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X) || input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_B);
   P2_state[KEY_BUTTON_4]   = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);
   P2_state[KEY_BUTTON_5]   = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
   P2_state[KEY_BUTTON_6]   = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);
   P2_state[KEY_BUTTON_7]   = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2);
   P2_state[KEY_JOYSTICK_U] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
   P2_state[KEY_JOYSTICK_D] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
   P2_state[KEY_JOYSTICK_L] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
   P2_state[KEY_JOYSTICK_R] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);

   P3_state[KEY_START]      = input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START) || input_state_cb(2, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START);
   P3_state[KEY_COIN]       = input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT) || input_state_cb(2, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SELECT);
   P3_state[KEY_BUTTON_1]   = input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A) || input_state_cb(2, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER) || input_state_cb(2, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD);
   P3_state[KEY_BUTTON_2]   = input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B) || input_state_cb(2, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_A);
   P3_state[KEY_BUTTON_3]   = input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X) || input_state_cb(2, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_B);
   P3_state[KEY_BUTTON_4]   = input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);
   P3_state[KEY_BUTTON_5]   = input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
   P3_state[KEY_BUTTON_6]   = input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);
   P3_state[KEY_BUTTON_7]   = input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2);
   P3_state[KEY_JOYSTICK_U] = input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
   P3_state[KEY_JOYSTICK_D] = input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
   P3_state[KEY_JOYSTICK_L] = input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
   P3_state[KEY_JOYSTICK_R] = input_state_cb(2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);
	
   P4_state[KEY_START]      = input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START) || input_state_cb(3, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START);
   P4_state[KEY_COIN]       = input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT) || input_state_cb(3, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SELECT);
   P4_state[KEY_BUTTON_1]   = input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A) || input_state_cb(3, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER) || input_state_cb(3, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD);
   P4_state[KEY_BUTTON_2]   = input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B) || input_state_cb(3, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_A);
   P4_state[KEY_BUTTON_3]   = input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X) || input_state_cb(3, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_B);
   P4_state[KEY_BUTTON_4]   = input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);
   P4_state[KEY_BUTTON_5]   = input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
   P4_state[KEY_BUTTON_6]   = input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);
   P4_state[KEY_BUTTON_7]   = input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2);
   P4_state[KEY_JOYSTICK_U] = input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
   P4_state[KEY_JOYSTICK_D] = input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
   P4_state[KEY_JOYSTICK_L] = input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
   P4_state[KEY_JOYSTICK_R] = input_state_cb(3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);
	
   P5_state[KEY_START]      = input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START) || input_state_cb(4, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START);
   P5_state[KEY_COIN]       = input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT) || input_state_cb(4, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SELECT);
   P5_state[KEY_BUTTON_1]   = input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A) || input_state_cb(4, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER) || input_state_cb(4, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD);
   P5_state[KEY_BUTTON_2]   = input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B) || input_state_cb(4, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_A);
   P5_state[KEY_BUTTON_3]   = input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X) || input_state_cb(4, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_B);
   P5_state[KEY_BUTTON_4]   = input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);
   P5_state[KEY_BUTTON_5]   = input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
   P5_state[KEY_BUTTON_6]   = input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);
   P5_state[KEY_BUTTON_7]   = input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2);   
   P5_state[KEY_JOYSTICK_U] = input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
   P5_state[KEY_JOYSTICK_D] = input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
   P5_state[KEY_JOYSTICK_L] = input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
   P5_state[KEY_JOYSTICK_R] = input_state_cb(4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);

   P6_state[KEY_START]      = input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START) || input_state_cb(5, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START);
   P6_state[KEY_COIN]       = input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT) || input_state_cb(5, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SELECT);
   P6_state[KEY_BUTTON_1]   = input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A) || input_state_cb(5, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER) || input_state_cb(5, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD);
   P6_state[KEY_BUTTON_2]   = input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B) || input_state_cb(5, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_A);
   P6_state[KEY_BUTTON_3]   = input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X) || input_state_cb(5, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_B);
   P6_state[KEY_BUTTON_4]   = input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);
   P6_state[KEY_BUTTON_5]   = input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
   P6_state[KEY_BUTTON_6]   = input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);
   P6_state[KEY_BUTTON_7]   = input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2);
   P6_state[KEY_JOYSTICK_U] = input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
   P6_state[KEY_JOYSTICK_D] = input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
   P6_state[KEY_JOYSTICK_L] = input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
   P6_state[KEY_JOYSTICK_R] = input_state_cb(5, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);

   P7_state[KEY_START]      = input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START) || input_state_cb(6, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START);
   P7_state[KEY_COIN]       = input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT) || input_state_cb(6, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SELECT);
   P7_state[KEY_BUTTON_1]   = input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A) || input_state_cb(6, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER) || input_state_cb(6, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD);
   P7_state[KEY_BUTTON_2]   = input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B) || input_state_cb(6, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_A);
   P7_state[KEY_BUTTON_3]   = input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X) || input_state_cb(6, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_B);
   P7_state[KEY_BUTTON_4]   = input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);
   P7_state[KEY_BUTTON_5]   = input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
   P7_state[KEY_BUTTON_6]   = input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);
   P7_state[KEY_BUTTON_7]   = input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2);
   P7_state[KEY_JOYSTICK_U] = input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
   P7_state[KEY_JOYSTICK_D] = input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
   P7_state[KEY_JOYSTICK_L] = input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
   P7_state[KEY_JOYSTICK_R] = input_state_cb(6, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);
	
   P8_state[KEY_START]      = input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START) || input_state_cb(7, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_START);
   P8_state[KEY_COIN]       = input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT) || input_state_cb(7, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_SELECT);
   P8_state[KEY_BUTTON_1]   = input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A) || input_state_cb(7, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_TRIGGER) || input_state_cb(7, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD);
   P8_state[KEY_BUTTON_2]   = input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B) || input_state_cb(7, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_A);
   P8_state[KEY_BUTTON_3]   = input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X) || input_state_cb(7, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_AUX_B);
   P8_state[KEY_BUTTON_4]   = input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y);
   P8_state[KEY_BUTTON_5]   = input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
   P8_state[KEY_BUTTON_6]   = input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);
   P8_state[KEY_BUTTON_7]   = input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2);
   P8_state[KEY_JOYSTICK_U] = input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
   P8_state[KEY_JOYSTICK_D] = input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
   P8_state[KEY_JOYSTICK_L] = input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
   P8_state[KEY_JOYSTICK_R] = input_state_cb(7, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);
  
      //Cursor glued to top-left corner in LIGHTGUN mode when detected as offscreen
      //Optional Libretro "Gun Reload" input gives the same behavior in button form (again, for LIGHTGUN inputs only)
      //Mouse mode and Joystick input do not respond well to forced reload button, so that function is not available in those modes
      if (((mouse_mode == 2) && (input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN))) || input_state_cb(0, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD))
        {
	//      //top left
	      gun1X = -65534;
	      gun1Y = -65534;
        }

      if (((mouse_mode == 2) && (input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN))) || input_state_cb(1, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD))
        {
	//      //top left
	      gun2X = -65534;
	      gun2Y = -65534;
        }
	
      if (((mouse_mode == 2) && (input_state_cb(2, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN))) || input_state_cb(2, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD))
        {
	//      //top left
	      gun3X = -65534;
	      gun3Y = -65534;
        }
	
      if (((mouse_mode == 2) && (input_state_cb(3, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN))) || input_state_cb(3, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD))
        {
	//      //top left
	      gun4X = -65534;
	      gun4Y = -65534;
        }
	
      if (((mouse_mode == 2) && (input_state_cb(4, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN))) || input_state_cb(4, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD))
        {
	//      //top left
	      gun5X = -65534;
	      gun5Y = -65534;
        }
	
      if (((mouse_mode == 2) && (input_state_cb(5, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN))) || input_state_cb(5, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD))
        {
	//      //top left
	      gun6X = -65534;
	      gun6Y = -65534;
        }
	
      if (((mouse_mode == 2) && (input_state_cb(6, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN))) || input_state_cb(6, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD))
        {
	//      //top left
	      gun7X = -65534;
	      gun7Y = -65534;
        }
	
      if (((mouse_mode == 2) && (input_state_cb(7, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN))) || input_state_cb(7, RETRO_DEVICE_LIGHTGUN, 0, RETRO_DEVICE_ID_LIGHTGUN_RELOAD))
        {
	//      //top left
	      gun8X = -65534;
	      gun8Y = -65534;
        }

//Shell for CPU overclock setting. Search mame_current_overclock for other pieces
// if (oc) 
// {
//	if (device_t::get_clock_scale(0) != oc) 
//	{	printf("changing cpu - clockscale from %lf to%lf\n",device_t::get_clock_scale(0),oc);
//		device_t::set_clock_scale(0, oc);
//	}	
// }	
	
   switch (turbo_enable)
   {
      case 0:
         break;
      case 1:
         if (PLAYER1_PRESS(A))
            P1_state[KEY_BUTTON_1] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER2_PRESS(A))
            P2_state[KEY_BUTTON_1] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER3_PRESS(A))
            P3_state[KEY_BUTTON_1] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER4_PRESS(A))
            P4_state[KEY_BUTTON_1] = turbo_state < turbo_delay ? 0 : 1;
         break;      
      case 2:
         if (PLAYER1_PRESS(B))
            P1_state[KEY_BUTTON_2] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER2_PRESS(B))
            P2_state[KEY_BUTTON_2] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER3_PRESS(B))
            P3_state[KEY_BUTTON_3] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER4_PRESS(B))
            P4_state[KEY_BUTTON_2] = turbo_state < turbo_delay ? 0 : 1;
        break;
      case 3:
         if (PLAYER1_PRESS(R2))
            P1_state[KEY_BUTTON_1] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER2_PRESS(R2))
            P2_state[KEY_BUTTON_1] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER3_PRESS(R2))
            P3_state[KEY_BUTTON_1] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER4_PRESS(R2))
            P4_state[KEY_BUTTON_1] = turbo_state < turbo_delay ? 0 : 1;
        break;
      case 4:
         if (PLAYER1_PRESS(R2))
            P1_state[KEY_BUTTON_2] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER2_PRESS(R2))
            P2_state[KEY_BUTTON_2] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER3_PRESS(R2))
            P3_state[KEY_BUTTON_2] = turbo_state < turbo_delay ? 0 : 1;
         if (PLAYER4_PRESS(R2))
            P4_state[KEY_BUTTON_2] = turbo_state < turbo_delay ? 0 : 1;
       break;
   }
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
		retro_log(RETRO_LOG_INFO, "[MAME 2010] path=%s gamePath=%s gameName=%s\n", path, gamePath, gameName);
		return 1;
	}
	if (slashIndex < 0 || dotIndex < 0) {
		return 0;
	}

	strncpy(gamePath, path, slashIndex);
	gamePath[slashIndex] = 0;
	strncpy(gameName, path + (slashIndex + 1), dotIndex - (slashIndex + 1));
	gameName[dotIndex - (slashIndex + 1)] = 0;

	retro_log(RETRO_LOG_INFO, "[MAME 2010] path=%s gamePath=%s gameName=%s\n", path, gamePath, gameName);
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
				retro_log(RETRO_LOG_INFO, "[MAME 2010] %-18s\"%s\" rot=%i \n", drivers[drvindex]->name, drivers[drvindex]->description, *rotation);
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
		retro_log(RETRO_LOG_ERROR, "[MAME 2010] Parse path failed! path=%s\n", path);
		strcpy(MgameName,path);
	//	return -1;
	}

	//Find the game info. Exit if game driver was not found.
	if (getGameInfo(MgameName, &gameRot, &driverIndex) == 0) {
		retro_log(RETRO_LOG_ERROR, "[MAME 2010] Game not found: %s\n", MgameName);
		return -2;
	}

	//tate enabled
	if (tate) {
		//horizontal game
		if (gameRot == ROT0) {
			screenRot = 1;
		} else
		if (gameRot &  ORIENTATION_FLIP_X) {
			retro_log(RETRO_LOG_INFO, "[MAME 2010]  *********** flip X\n");
			screenRot = 3;
		}

	} else
	{
		if (gameRot != ROT0) {
			screenRot = 1;
			if (gameRot &  ORIENTATION_FLIP_X) {
				retro_log(RETRO_LOG_INFO, "[MAME 2010]  *********** flip X\n");
				screenRot = 2;
			}
		}
	}

	retro_log(RETRO_LOG_INFO, "[MAME 2010] Creating frontend... game=%s\n", MgameName);

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

	retro_log(RETRO_LOG_INFO, "[MAME 2010] Invoking MAME2010 CLI frontend. Parameter count: %i\n", paramCount);

    char parameters[1024];
	for (int i = 0; xargv[i] != NULL; i++)
 		snprintf(parameters, sizeof(parameters), "%s ",xargv[i]);

    retro_log(RETRO_LOG_INFO, "[MAME 2010] Parameter list: %s\n", parameters);

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
