//============================================================
//
//  minidir.c - Minimal core directory access functions
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

//============================================================
//
//  sdldir.c - SDL core directory access functions
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifdef SDLMAME_LINUX
#define __USE_LARGEFILE64
#endif
#ifndef SDLMAME_BSD
#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif
#define _XOPEN_SOURCE 500
#endif
#include <stdlib.h>
#include <unistd.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef __USE_BSD
#define __USE_BSD	// to get DT_xxx on Linux
#endif
#undef _POSIX_C_SOURCE  // to get DT_xxx on OS X
#include <dirent.h>

#include "osdcore.h"


#if defined(SDLMAME_WIN32) || defined(SDLMAME_OS2)
#define PATHSEPCH '\\'
#define INVPATHSEPCH '/'
#else
#define PATHSEPCH '/'
#define INVPATHSEPCH '\\'
#endif

#if defined(SDLMAME_DARWIN) || defined(SDLMAME_WIN32) || defined(SDLMAME_NO64BITIO) || defined(SDLMAME_BSD) || defined(SDLMAME_OS2)
typedef struct dirent sdl_dirent;
typedef struct stat sdl_stat;
#define sdl_readdir readdir
#define sdl_stat_fn stat
#else
typedef struct dirent64 sdl_dirent;
typedef struct stat64 sdl_stat;
#define sdl_readdir readdir64
#define sdl_stat_fn stat64
#endif

#define HAS_DT_XXX defined(SDLMAME_LINUX) || defined(SDLMAME_BSD) || defined(SDLMAME_DARWIN)

struct _osd_directory
{
	osd_directory_entry ent;
	sdl_dirent *data;
	DIR *fd;
	char *path;
};

static char *build_full_path(const char *path, const char *file)
{
	char *ret = (char *) osd_malloc(strlen(path)+strlen(file)+2);
	char *p = ret;

	strcpy(p, path);
	p += strlen(path);
	*p++ = PATHSEPCH;
	strcpy(p, file);
	return ret;
}


#if HAS_DT_XXX
static osd_dir_entry_type get_attributes_enttype(int attributes)
{
	if (attributes == DT_DIR)
		return ENTTYPE_DIR;
	else
		return ENTTYPE_FILE;
}
#else

static osd_dir_entry_type get_attributes_stat(const char *file)
{
	sdl_stat st;
	if(sdl_stat_fn(file, &st))
		return (osd_dir_entry_type) 0;

	if (S_ISDIR(st.st_mode))
		return ENTTYPE_DIR;
	else
		return ENTTYPE_FILE;
}
#endif

static UINT64 osd_get_file_size(const char *file)
{
	sdl_stat st;
	if(sdl_stat_fn(file, &st))
		return 0;
	return st.st_size;
}


char *osd_getenv(const char *name)
{
	//return "./";	
	return getenv(name);
}

//============================================================
//  osd_setenv
//============================================================

int osd_setenv(const char *name, const char *value, int overwrite)
{
	return setenv(name, value, overwrite);
}

//============================================================
//  osd_opendir
//============================================================

osd_directory *osd_opendir(const char *dirname)
{
	osd_directory *dir = NULL;
	char *tmpstr, *envstr;
	int i, j;

	dir = (osd_directory *) osd_malloc(sizeof(osd_directory));
	if (dir)
	{
		memset(dir, 0, sizeof(osd_directory));
		dir->fd = NULL;
	}

	tmpstr = (char *) osd_malloc(strlen(dirname)+1);
	strcpy(tmpstr, dirname);

	if (tmpstr[0] == '$')
	{
		char *envval;
		envstr = (char *) osd_malloc(strlen(tmpstr)+1);

		strcpy(envstr, tmpstr);

		i = 0;
		while (envstr[i] != PATHSEPCH && envstr[i] != INVPATHSEPCH && envstr[i] != 0 && envstr[i] != '.')
		{
			i++;
		}

		envstr[i] = '\0';

		envval = osd_getenv(&envstr[1]);
		if (envval != NULL)
		{
			j = strlen(envval) + strlen(tmpstr) + 1;
			osd_free(tmpstr);
			tmpstr = (char *) osd_malloc(j);

			// start with the value of $HOME
			strcpy(tmpstr, envval);
			// replace the null with a path separator again
			envstr[i] = PATHSEPCH;
			// append it
			strcat(tmpstr, &envstr[i]);
		}
		else
			fprintf(stderr, "Warning: osd_opendir environment variable %s not found.\n", envstr);
		osd_free(envstr);
	}

	dir->fd = opendir(tmpstr);
	dir->path = tmpstr;

	if (dir && (dir->fd == NULL))
	{
		osd_free(dir->path);
		osd_free(dir);
		dir = NULL;
	}

	return dir;
}


//============================================================
//  osd_readdir
//============================================================

const osd_directory_entry *osd_readdir(osd_directory *dir)
{
	char *temp;
	dir->data = sdl_readdir(dir->fd);

	if (dir->data == NULL)
		return NULL;

	dir->ent.name = dir->data->d_name;
	temp = build_full_path(dir->path, dir->data->d_name);
	#if HAS_DT_XXX
	dir->ent.type = get_attributes_enttype(dir->data->d_type);
	#else
	dir->ent.type = get_attributes_stat(temp);
	#endif
	dir->ent.size = osd_get_file_size(temp);
	osd_free(temp);
	return &dir->ent;
}


//============================================================
//  osd_closedir
//============================================================

void osd_closedir(osd_directory *dir)
{
	if (dir->fd != NULL)
		closedir(dir->fd);
	osd_free(dir->path);
	osd_free(dir);
}

