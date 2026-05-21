/***************************************************************************

    fileio.h

    Core file I/O interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include "corefile.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _mame_file mame_file;
typedef struct _mame_path mame_path;

typedef struct _core_options core_options;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- core initialization ----- */

/* initialize the fileio system */
void fileio_init(running_machine *machine);



/* ----- file open/close ----- */

/* open a file in the given search path with the specified filename */
file_error mame_fopen(const char *searchpath, const char *filename, uint32_t openflags, mame_file **file);

/* open a file in the given search path with the specified filename or a matching CRC */
file_error mame_fopen_crc(const char *searchpath, const char *filename, uint32_t crc, uint32_t openflags, mame_file **file);

/* open a file in the given search path with the specified filename, using the specified options */
file_error mame_fopen_options(core_options *opts, const char *searchpath, const char *filename, uint32_t openflags, mame_file **file);

/* open a file in the given search path with the specified filename or a matching CRC, using the specified options */
file_error mame_fopen_crc_options(core_options *opts, const char *searchpath, const char *filename, uint32_t crc, uint32_t openflags, mame_file **file);

/* open a "file" which is actually data in RAM */
file_error mame_fopen_ram(const void *data, uint32_t length, uint32_t openflags, mame_file **file);

/* close an open file */
void mame_fclose(mame_file *file);

/* close an open file, and open the next entry in the original searchpath*/
file_error mame_fclose_and_open_next(mame_file **file, const char *filename, uint32_t openflags);

/* enable/disable streaming file compression via zlib; level is 0 to disable compression, or up to 9 for max compression */
file_error mame_fcompress(mame_file *file, int compress);



/* ----- file positioning ----- */

/* adjust the file pointer within the file */
int mame_fseek(mame_file *file, int64_t offset, int whence);

/* return the current file pointer */
uint64_t mame_ftell(mame_file *file);

/* return true if we are at the EOF */
int mame_feof(mame_file *file);

/* return the total size of the file */
uint64_t mame_fsize(mame_file *file);



/* ----- file read ----- */

/* standard binary read from a file */
uint32_t mame_fread(mame_file *file, void *buffer, uint32_t length);

/* read one character from the file */
int mame_fgetc(mame_file *file);

/* put back one character from the file */
int mame_ungetc(int c, mame_file *file);

/* read a full line of text from the file */
char *mame_fgets(char *s, int n, mame_file *file);



/* ----- file write ----- */

/* standard binary write to a file */
uint32_t mame_fwrite(mame_file *file, const void *buffer, uint32_t length);

/* write a line of text to the file */
int mame_fputs(mame_file *f, const char *s);

/* printf-style text write to a file */
int CLIB_DECL mame_fprintf(mame_file *f, const char *fmt, ...) ATTR_PRINTF(2,3);



/* ----- file enumeration ----- */

/* open a search path (multiple directories) for iteration */
mame_path *mame_openpath(core_options *opts, const char *searchpath);

/* return information about the next file in the search path */
const osd_directory_entry *mame_readpath(mame_path *path);

/* close an open seach path */
void mame_closepath(mame_path *path);



/* ----- file misc ----- */

/* return the core_file underneath the mame_file */
core_file *mame_core_file(mame_file *file);

/* return the full filename for a given mame_file */
const astring &mame_file_full_name(mame_file *file);

/* return a hash string for the file with the given functions */
const char *mame_fhash(mame_file *file, uint32_t functions);



#endif	/* __FILEIO_H__ */
