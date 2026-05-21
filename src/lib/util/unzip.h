/***************************************************************************

    unzip.h

    ZIP file management.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __UNZIP_H__
#define __UNZIP_H__

#include "osdcore.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define ZIP_DECOMPRESS_BUFSIZE	16384

/* Error types */
enum _zip_error
{
	ZIPERR_NONE = 0,
	ZIPERR_OUT_OF_MEMORY,
	ZIPERR_FILE_ERROR,
	ZIPERR_BAD_SIGNATURE,
	ZIPERR_DECOMPRESS_ERROR,
	ZIPERR_FILE_TRUNCATED,
	ZIPERR_FILE_CORRUPT,
	ZIPERR_UNSUPPORTED,
	ZIPERR_BUFFER_TOO_SMALL
};
typedef enum _zip_error zip_error;



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* contains extracted file header information */
typedef struct _zip_file_header zip_file_header;
struct _zip_file_header
{
	uint32_t			signature;				/* central file header signature */
	uint16_t			version_created;		/* version made by */
	uint16_t			version_needed;			/* version needed to extract */
	uint16_t			bit_flag;				/* general purpose bit flag */
	uint16_t			compression;			/* compression method */
	uint16_t			file_time;				/* last mod file time */
	uint16_t			file_date;				/* last mod file date */
	uint32_t			crc;					/* crc-32 */
	uint32_t			compressed_length;		/* compressed size */
	uint32_t			uncompressed_length;	/* uncompressed size */
	uint16_t			filename_length;		/* filename length */
	uint16_t			extra_field_length;		/* extra field length */
	uint16_t			file_comment_length;	/* file comment length */
	uint16_t			start_disk_number;		/* disk number start */
	uint16_t			internal_attributes;	/* internal file attributes */
	uint32_t			external_attributes;	/* external file attributes */
	uint32_t			local_header_offset;	/* relative offset of local header */
	const char *	filename;				/* filename */

	uint8_t *			raw;					/* pointer to the raw data */
	uint32_t			rawlength;				/* length of the raw data */
	uint8_t			saved;					/* saved byte from after filename */
};


/* contains extracted end of central directory information */
typedef struct _zip_ecd zip_ecd;
struct _zip_ecd
{
	uint32_t			signature;				/* end of central dir signature */
	uint16_t			disk_number;			/* number of this disk */
	uint16_t			cd_start_disk_number;	/* number of the disk with the start of the central directory */
	uint16_t			cd_disk_entries;		/* total number of entries in the central directory on this disk */
	uint16_t			cd_total_entries;		/* total number of entries in the central directory */
	uint32_t			cd_size;				/* size of the central directory */
	uint32_t			cd_start_disk_offset;	/* offset of start of central directory with respect to the starting disk number */
	uint16_t			comment_length;			/* .ZIP file comment length */
	const char *	comment;				/* .ZIP file comment */

	uint8_t *			raw;					/* pointer to the raw data */
	uint32_t			rawlength;				/* length of the raw data */
};


/* describes an open ZIP file */
typedef struct _zip_file zip_file;
struct _zip_file
{
	const char *	filename;				/* copy of ZIP filename (for caching) */
	osd_file *		file;					/* OSD file handle */
	uint64_t			length;					/* length of zip file */

	zip_ecd			ecd;					/* end of central directory */

	uint8_t *			cd;						/* central directory raw data */
	uint32_t			cd_pos;					/* position in central directory */
	zip_file_header	header;					/* current file header */

	uint8_t			buffer[ZIP_DECOMPRESS_BUFSIZE];	/* buffer for decompression */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- ZIP file access ----- */

/* open a ZIP file and parse its central directory */
zip_error zip_file_open(const char *filename, zip_file **zip);

/* close a ZIP file (may actually be left open due to caching) */
void zip_file_close(zip_file *zip);

/* clear out all open ZIP files from the cache */
void zip_file_cache_clear(void);


/* ----- contained file access ----- */

/* find the first file in the ZIP */
const zip_file_header *zip_file_first_file(zip_file *zip);

/* find the next file in the ZIP */
const zip_file_header *zip_file_next_file(zip_file *zip);

/* decompress the most recently found file in the ZIP */
zip_error zip_file_decompress(zip_file *zip, void *buffer, uint32_t length);


#endif	/* __UNZIP_H__ */
