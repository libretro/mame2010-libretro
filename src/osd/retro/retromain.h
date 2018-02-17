/***************************************************************************

retromain.h 

****************************************************************************/

#pragma once

#ifndef __RETROMAIN_H__
#define __RETROMAIN_H__

#include "options.h"
#include "osdepend.h"

extern int RLOOP;
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

extern const char* core_name;

extern char libretro_content_directory[];
extern char libretro_save_directory[];
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
extern char comment_directory[];

#endif	/* __RETROMAIN_H__ */