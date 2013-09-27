
#include "options.h"
#include "osdepend.h"


//============================================================
//  TYPE DEFINITIONS
//============================================================
void osd_init(running_machine* machine);
void osd_update(running_machine* machine,int skip_redraw);
void osd_update_audio_stream(running_machine* machine,short *buffer, int samples_this_frame);
void osd_set_mastervolume(int attenuation);
void osd_customize_input_type_list(input_type_desc *typelist);
void osd_exit(running_machine &machine);

//============================================================
//  GLOBAL VARIABLES
//============================================================
extern int osd_num_processors;

// use if you want to print something with the verbose flag
void CLIB_DECL mame_printf_verbose(const char *text, ...) ATTR_PRINTF(1,2);
