/***************************************************************************

    chd.c

    MAME Compressed Hunks of Data file format

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

/* The container itself is read and written by libretro-common's rchd;
   this keeps MAME's chd_* interface over it, supplying the bytes rchd
   asks for from the core_file, writing diffs through it, and unpacking
   A/V hunks into the buffers a laserdisc player hands over. */

#include "chd.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <formats/rchd.h>
/* lrc_hash.h writes "static INLINE": give it a plain inline, then restore
   the core's */
#undef INLINE
#define INLINE inline
#include <lrc_hash.h>
#undef INLINE
#define INLINE static inline



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* the CHD file itself */
struct _chd_file
{
	rchd_t *				rchd;			/* the decoder */
	core_file *				file;			/* handle to the open core file */
	chd_file *				parent;			/* the image this one differences against */
	uint8_t *				fetch;			/* holds the bytes of a request while rchd reads them */
	osd_work_queue *		workqueue;		/* work queue for async operations */
	osd_work_item *			workitem;		/* active work item, or NULL if none */
	void *					async_buffer;	/* buffer for the async read */
	uint64_t				file_end;		/* size of a writeable file */
	chd_header				header;			/* the header, as MAME reports it */
	av_codec_decompress_config avconfig;	/* where A/V hunks are unpacked to */
	uint32_t				fetch_size;		/* size of fetch */
	uint32_t				async_hunknum;	/* hunk for the async read */
	uint8_t					owns_file;		/* flag indicating if this file should be closed on chd_close() */
	uint8_t					have_avconfig;	/* avconfig has been set */
};


/* one checksummed metadata entry's contribution to the combined SHA-1 */
typedef struct _metadata_hash metadata_hash;
struct _metadata_hash
{
	uint8_t					tag[4];			/* tag of the metadata in big-endian */
	uint8_t					sha1[CHD_SHA1_BYTES]; /* hash data */
};



/***************************************************************************
    FILE ACCESS
***************************************************************************/

static chd_error chd_from_rchd(int err, chd_error dataerr)
{
	switch (err)
	{
		case RCHD_OK:					return CHDERR_NONE;
		case RCHD_ERROR_DATA:
		case RCHD_ERROR_CRC:			return dataerr;
		case RCHD_ERROR_PARAM:			return CHDERR_INVALID_PARAMETER;
		case RCHD_ERROR_MEM:			return CHDERR_OUT_OF_MEMORY;
		case RCHD_ERROR_UNSUPPORTED:	return (dataerr == CHDERR_INVALID_FILE) ? CHDERR_UNSUPPORTED_VERSION : CHDERR_UNSUPPORTED_FORMAT;
		case RCHD_ERROR_NO_PARENT:		return CHDERR_REQUIRES_PARENT;
		case RCHD_ERROR_STATE:			return CHDERR_INVALID_STATE;
		default:						return CHDERR_READ_ERROR;
	}
}


/*-------------------------------------------------
    chd_supply - satisfy what rchd asks for, until
    an open or a read is done; a parent's bytes
    come from the parent's file
-------------------------------------------------*/

static chd_error chd_supply(chd_file *chd, int opening, chd_error dataerr)
{
	for (;;)
	{
		rchd_request_t req;
		core_file *source;
		int err = opening ? rchd_open_step(chd->rchd, &req) : rchd_read_step(chd->rchd, &req);

		if (err != RCHD_PENDING)
			return chd_from_rchd(err, dataerr);

		source = (req.source == RCHD_SOURCE_PARENT) ? chd->parent->file : chd->file;
		if (req.length > chd->fetch_size)
		{
			uint8_t *grown = (uint8_t *)realloc(chd->fetch, req.length);
			if (grown == NULL)
				return CHDERR_OUT_OF_MEMORY;
			chd->fetch = grown;
			chd->fetch_size = req.length;
		}

		/* the bytes are lent rather than copied again */
		core_fseek(source, req.offset, SEEK_SET);
		if (core_fread(source, chd->fetch, req.length) != req.length
				|| rchd_feed_borrow(chd->rchd, req.offset, req.source, chd->fetch, req.length) != RCHD_OK)
			return CHDERR_READ_ERROR;
	}
}


static int chd_sink(void *ctx, uint64_t offset, const void *data, size_t len)
{
	core_file *file = (core_file *)ctx;
	core_fseek(file, offset, SEEK_SET);
	return core_fwrite(file, data, (uint32_t)len) == len;
}



/***************************************************************************
    A/V HUNKS
***************************************************************************/

/*-------------------------------------------------
    av_unpack - copy a decoded A/V hunk into the
    configured buffers, natively ordered
-------------------------------------------------*/

static chd_error av_unpack(chd_file *chd, const uint8_t *hunk)
{
	const av_codec_decompress_config *config = &chd->avconfig;
	rchd_av_frame_t frame;
	uint32_t chnum, x, y, i;

	if (rchd_av_parse(hunk, chd->header.hunkbytes, &frame) != RCHD_OK)
		return CHDERR_DECOMPRESSION_ERROR;

	/* the frame has to fit what it is unpacked into */
	if (config->video != NULL && (config->video->width < (int)frame.width || config->video->height < (int)frame.height))
		return CHDERR_DECOMPRESSION_ERROR;
	for (chnum = 0; chnum < frame.channels; chnum++)
		if (config->audio[chnum] != NULL && config->maxsamples < frame.samples)
			return CHDERR_DECOMPRESSION_ERROR;
	if (config->metadata != NULL && config->maxmetalength < frame.meta_size)
		return CHDERR_DECOMPRESSION_ERROR;

	if (config->actsamples != NULL)
		*config->actsamples = frame.samples;
	if (config->actmetalength != NULL)
		*config->actmetalength = frame.meta_size;
	if (config->metadata != NULL && frame.meta_size > 0)
		memcpy(config->metadata, frame.meta, frame.meta_size);

	/* the hunk holds samples and pixels big-endian */
	for (chnum = 0; chnum < frame.channels; chnum++)
		if (config->audio[chnum] != NULL)
		{
			const uint8_t *src = (const uint8_t *)frame.audio[chnum];
			int16_t *dst = config->audio[chnum];
			for (i = 0; i < frame.samples; i++)
				dst[i] = (int16_t)((src[i * 2] << 8) | src[i * 2 + 1]);
		}

	if (config->video != NULL)
		for (y = 0; y < frame.height; y++)
		{
			const uint8_t *src = frame.video + y * frame.stride;
			uint16_t *dst = (uint16_t *)config->video->base + y * config->video->rowpixels;
			for (x = 0; x < frame.width; x++)
				dst[x] = (uint16_t)((src[x * 2] << 8) | src[x * 2 + 1]);
		}

	return CHDERR_NONE;
}


/*-------------------------------------------------
    chd_read_hunk - read a hunk into buffer, or,
    without one, unpack an A/V hunk into the
    configured buffers
-------------------------------------------------*/

static chd_error chd_read_hunk(chd_file *chd, uint32_t hunknum, uint8_t *buffer)
{
	chd_error err;

	if (rchd_read_hunk_begin(chd->rchd, hunknum, buffer) != RCHD_OK)
		return CHDERR_HUNK_OUT_OF_RANGE;
	err = chd_supply(chd, FALSE, CHDERR_DECOMPRESSION_ERROR);
	if (err != CHDERR_NONE)
		return err;

	/* decoded in place, without a copy */
	if (buffer == NULL && chd->have_avconfig)
		return av_unpack(chd, rchd_hunk_cache(chd->rchd));
	return CHDERR_NONE;
}


static void *async_read_callback(void *param, int threadid)
{
	chd_file *chd = (chd_file *)param;
	return (void *)(ptrdiff_t)chd_read_hunk(chd, chd->async_hunknum, (uint8_t *)chd->async_buffer);
}


static void wait_for_pending_async(chd_file *chd)
{
	/* if something is pending, wait for it */
	if (chd->workitem != NULL)
	{
		/* 10 seconds should be enough for anything! */
		int wait_successful = osd_work_item_wait(chd->workitem, 10 * osd_ticks_per_second());
		if (!wait_successful)
			osd_break_into_debugger("Pending async operation never completed!");
	}
}



/***************************************************************************
    CHD FILE MANAGEMENT
***************************************************************************/

static int CLIB_DECL metadata_hash_compare(const void *elem1, const void *elem2)
{
	return memcmp(elem1, elem2, sizeof(metadata_hash));
}


/*-------------------------------------------------
    diff_sha1 - the combined SHA-1 MAME stores in
    a diff: of its raw SHA-1, which is zero, and
    the sorted hashes of the metadata entries
    flagged for checksumming
-------------------------------------------------*/

static chd_error diff_sha1(rchd_t *parent, uint8_t *sha1)
{
	static const uint8_t zero[CHD_SHA1_BYTES] = { 0 };
	uint32_t count = rchd_metadata_count(parent);
	metadata_hash *hasharray;
	struct sha1_state state;
	uint32_t i, n = 0;

	hasharray = (metadata_hash *)malloc((count ? count : 1) * sizeof(hasharray[0]));
	if (hasharray == NULL)
		return CHDERR_OUT_OF_MEMORY;
	for (i = 0; i < count; i++)
	{
		const rchd_metadata_t *m = rchd_metadata(parent, i);
		if (!(m->flags & CHD_MDFLAGS_CHECKSUM))
			continue;
		hasharray[n].tag[0] = (uint8_t)(m->tag >> 24);
		hasharray[n].tag[1] = (uint8_t)(m->tag >> 16);
		hasharray[n].tag[2] = (uint8_t)(m->tag >> 8);
		hasharray[n].tag[3] = (uint8_t)m->tag;
		SHA1Digest(m->data, m->length, hasharray[n].sha1);
		n++;
	}
	if (n > 0)
		qsort(hasharray, n, sizeof(hasharray[0]), metadata_hash_compare);

	sha1_stream_init(&state);
	sha1_stream_update(&state, zero, sizeof(zero));
	sha1_stream_update(&state, (const uint8_t *)hasharray, n * sizeof(hasharray[0]));
	sha1_stream_final(&state, sha1);
	free(hasharray);
	return CHDERR_NONE;
}


/*-------------------------------------------------
    chd_create_file - create a diff of the parent;
    the size, hunk size and metadata are the
    parent's
-------------------------------------------------*/

chd_error chd_create_file(core_file *file, uint64_t logicalbytes, uint32_t hunkbytes, uint32_t compression, chd_file *parent)
{
	uint8_t sha1[CHD_SHA1_BYTES];
	chd_error err;

	if (file == NULL || parent == NULL)
		return CHDERR_INVALID_PARAMETER;
	if (compression != CHDCOMPRESSION_NONE)
		return CHDERR_NOT_SUPPORTED;

	/* with no metadata to clone, MAME leaves the combined SHA-1 zero */
	if (rchd_metadata_count(parent->rchd) > 0)
	{
		err = diff_sha1(parent->rchd, sha1);
		if (err != CHDERR_NONE)
			return err;
	}

	if (rchd_write_diff(parent->rchd, rchd_metadata_count(parent->rchd) > 0 ? sha1 : NULL, chd_sink, file) != RCHD_OK)
		return CHDERR_WRITE_ERROR;
	return CHDERR_NONE;
}


/*-------------------------------------------------
    chd_open_file - open a CHD file for access
-------------------------------------------------*/

chd_error chd_open_file(core_file *file, int mode, chd_file *parent, chd_file **chd)
{
	static const uint8_t nullmd5[CHD_MD5_BYTES] = { 0 };
	static const uint8_t nullsha1[CHD_SHA1_BYTES] = { 0 };
	static const uint32_t header_length[6] = { 0, CHD_V1_HEADER_SIZE, CHD_V2_HEADER_SIZE, CHD_V3_HEADER_SIZE, CHD_V4_HEADER_SIZE, 124 };
	const rchd_info_t *info;
	chd_header *header;
	chd_file *newchd;
	chd_error err;

	if (file == NULL || (mode != CHD_OPEN_READ && mode != CHD_OPEN_READWRITE))
		return CHDERR_INVALID_PARAMETER;

	newchd = (chd_file *)calloc(1, sizeof(*newchd));
	if (newchd == NULL)
		return CHDERR_OUT_OF_MEMORY;
	newchd->file = file;
	newchd->parent = parent;
	newchd->rchd = rchd_new();
	if (newchd->rchd == NULL)
	{
		err = CHDERR_OUT_OF_MEMORY;
		goto cleanup;
	}

	/* the header, map and metadata */
	err = chd_supply(newchd, TRUE, CHDERR_INVALID_FILE);
	if (err != CHDERR_NONE)
		goto cleanup;
	info = rchd_info(newchd->rchd);

	header = &newchd->header;
	header->length = header_length[info->version];
	header->version = info->version;
	header->flags = info->flags;
	header->hunkbytes = info->hunk_bytes;
	header->totalhunks = info->hunk_count;
	header->logicalbytes = info->logical_bytes;
	if (info->compressors[0] == RCHD_CODEC_AVHUFF)
		header->compression = CHDCOMPRESSION_AV;
	else if (info->compressors[0] == RCHD_CODEC_NONE)
		header->compression = CHDCOMPRESSION_NONE;
	else
		header->compression = CHDCOMPRESSION_ZLIB;
	memcpy(header->md5, info->md5, sizeof(header->md5));
	memcpy(header->parentmd5, info->parent_md5, sizeof(header->parentmd5));
	memcpy(header->sha1, info->sha1, sizeof(header->sha1));
	memcpy(header->rawsha1, info->raw_sha1, sizeof(header->rawsha1));
	memcpy(header->parentsha1, info->parent_sha1, sizeof(header->parentsha1));

	/* only a writeable file of the current version opens writeable */
	if (mode == CHD_OPEN_READWRITE && !(header->flags & CHDFLAGS_IS_WRITEABLE))
	{
		err = CHDERR_FILE_NOT_WRITEABLE;
		goto cleanup;
	}
	if (mode == CHD_OPEN_READWRITE && header->version != CHD_HEADER_VERSION)
	{
		err = CHDERR_UNSUPPORTED_VERSION;
		goto cleanup;
	}

	/* a diff needs the parent it was made against */
	if (info->has_parent)
	{
		if (parent == NULL)
		{
			err = CHDERR_REQUIRES_PARENT;
			goto cleanup;
		}
		if ((memcmp(nullmd5, header->parentmd5, sizeof(header->parentmd5)) != 0 &&
				memcmp(nullmd5, parent->header.md5, sizeof(parent->header.md5)) != 0 &&
				memcmp(parent->header.md5, header->parentmd5, sizeof(header->parentmd5)) != 0) ||
			(memcmp(nullsha1, header->parentsha1, sizeof(header->parentsha1)) != 0 &&
				memcmp(nullsha1, parent->header.sha1, sizeof(parent->header.sha1)) != 0 &&
				memcmp(parent->header.sha1, header->parentsha1, sizeof(header->parentsha1)) != 0))
		{
			err = CHDERR_INVALID_PARENT;
			goto cleanup;
		}
		if (rchd_set_parent(newchd->rchd, parent->rchd) != RCHD_OK)
		{
			err = CHDERR_INVALID_PARENT;
			goto cleanup;
		}
	}

	if (mode == CHD_OPEN_READWRITE)
		newchd->file_end = core_fsize(file);

	*chd = newchd;
	return CHDERR_NONE;

cleanup:
	chd_close(newchd);
	return err;
}


/*-------------------------------------------------
    chd_open - open a CHD file by
    filename
-------------------------------------------------*/

chd_error chd_open(const char *filename, int mode, chd_file *parent, chd_file **chd)
{
	core_file *file = NULL;
	chd_error err;

	if (mode != CHD_OPEN_READ && mode != CHD_OPEN_READWRITE)
		return CHDERR_INVALID_PARAMETER;
	if (core_fopen(filename, (mode == CHD_OPEN_READ) ? OPEN_FLAG_READ : (OPEN_FLAG_READ | OPEN_FLAG_WRITE), &file) != FILERR_NONE)
		return CHDERR_FILE_NOT_FOUND;

	err = chd_open_file(file, mode, parent, chd);
	if (err != CHDERR_NONE)
	{
		core_fclose(file);
		return err;
	}

	/* we now own this file */
	(*chd)->owns_file = TRUE;
	return CHDERR_NONE;
}


/*-------------------------------------------------
    chd_close - close a CHD file for access
-------------------------------------------------*/

void chd_close(chd_file *chd)
{
	if (chd == NULL)
		return;

	wait_for_pending_async(chd);
	if (chd->workitem != NULL)
		osd_work_item_release(chd->workitem);
	if (chd->workqueue != NULL)
		osd_work_queue_free(chd->workqueue);

	rchd_free(chd->rchd);
	free(chd->fetch);
	if (chd->owns_file && chd->file != NULL)
		core_fclose(chd->file);
	free(chd);
}


/*-------------------------------------------------
    chd_error_string - return an error string for
    the given CHD error
-------------------------------------------------*/

const char *chd_error_string(chd_error err)
{
	switch (err)
	{
		case CHDERR_NONE:						return "no error";
		case CHDERR_NO_INTERFACE:				return "no drive interface";
		case CHDERR_OUT_OF_MEMORY:				return "out of memory";
		case CHDERR_INVALID_FILE:				return "invalid file";
		case CHDERR_INVALID_PARAMETER:			return "invalid parameter";
		case CHDERR_INVALID_DATA:				return "invalid data";
		case CHDERR_FILE_NOT_FOUND:				return "file not found";
		case CHDERR_REQUIRES_PARENT:			return "requires parent";
		case CHDERR_FILE_NOT_WRITEABLE:			return "file not writeable";
		case CHDERR_READ_ERROR:					return "read error";
		case CHDERR_WRITE_ERROR:				return "write error";
		case CHDERR_CODEC_ERROR:				return "codec error";
		case CHDERR_INVALID_PARENT:				return "invalid parent";
		case CHDERR_HUNK_OUT_OF_RANGE:			return "hunk out of range";
		case CHDERR_DECOMPRESSION_ERROR:		return "decompression error";
		case CHDERR_COMPRESSION_ERROR:			return "compression error";
		case CHDERR_CANT_CREATE_FILE:			return "can't create file";
		case CHDERR_CANT_VERIFY:				return "can't verify file";
		case CHDERR_NOT_SUPPORTED:				return "operation not supported";
		case CHDERR_METADATA_NOT_FOUND:			return "can't find metadata";
		case CHDERR_INVALID_METADATA_SIZE:		return "invalid metadata size";
		case CHDERR_UNSUPPORTED_VERSION:		return "unsupported CHD version";
		case CHDERR_VERIFY_INCOMPLETE:			return "incomplete verify";
		case CHDERR_INVALID_METADATA:			return "invalid metadata";
		case CHDERR_INVALID_STATE:				return "invalid state";
		case CHDERR_OPERATION_PENDING:			return "operation pending";
		case CHDERR_NO_ASYNC_OPERATION:			return "no async operation in progress";
		case CHDERR_UNSUPPORTED_FORMAT:			return "unsupported format";
		default:								return "undocumented error";
	}
}


/*-------------------------------------------------
    chd_get_header - return a pointer to the
    extracted header data
-------------------------------------------------*/

const chd_header *chd_get_header(chd_file *chd)
{
	return (chd != NULL) ? &chd->header : NULL;
}



/***************************************************************************
    CORE DATA READ/WRITE
***************************************************************************/

/*-------------------------------------------------
    chd_read - read a single hunk from the CHD
    file
-------------------------------------------------*/

chd_error chd_read(chd_file *chd, uint32_t hunknum, void *buffer)
{
	if (chd == NULL)
		return CHDERR_INVALID_PARAMETER;
	if (hunknum >= chd->header.totalhunks)
		return CHDERR_HUNK_OUT_OF_RANGE;

	wait_for_pending_async(chd);
	return chd_read_hunk(chd, hunknum, (uint8_t *)buffer);
}


/*-------------------------------------------------
    chd_read_async - read a single hunk from the
    CHD file asynchronously
-------------------------------------------------*/

chd_error chd_read_async(chd_file *chd, uint32_t hunknum, void *buffer)
{
	if (chd == NULL)
		return CHDERR_INVALID_PARAMETER;
	if (hunknum >= chd->header.totalhunks)
		return CHDERR_HUNK_OUT_OF_RANGE;

	wait_for_pending_async(chd);
	chd->async_hunknum = hunknum;
	chd->async_buffer = buffer;

	/* queue the work item, creating the queue on first use */
	if (chd->workqueue == NULL)
		chd->workqueue = osd_work_queue_alloc(WORK_QUEUE_FLAG_IO);
	if (chd->workqueue != NULL && chd->workitem == NULL)
	{
		chd->workitem = osd_work_item_queue(chd->workqueue, async_read_callback, chd, 0);
		if (chd->workitem != NULL)
			return CHDERR_OPERATION_PENDING;
	}

	/* if we fail, fall back on the sync version */
	return chd_read_hunk(chd, hunknum, (uint8_t *)buffer);
}


/*-------------------------------------------------
    chd_async_complete - wait for an outstanding
    async operation to complete
-------------------------------------------------*/

chd_error chd_async_complete(chd_file *chd)
{
	void *result;

	if (chd->workitem == NULL)
		return CHDERR_NO_ASYNC_OPERATION;

	wait_for_pending_async(chd);
	result = osd_work_item_result(chd->workitem);
	osd_work_item_release(chd->workitem);
	chd->workitem = NULL;
	return (chd_error)(ptrdiff_t)result;
}


/*-------------------------------------------------
    chd_write - write a single hunk to a CHD file
-------------------------------------------------*/

chd_error chd_write(chd_file *chd, uint32_t hunknum, const void *buffer)
{
	int err;

	if (chd == NULL || buffer == NULL)
		return CHDERR_INVALID_PARAMETER;
	if (hunknum >= chd->header.totalhunks)
		return CHDERR_HUNK_OUT_OF_RANGE;

	wait_for_pending_async(chd);
	err = rchd_update_hunk(chd->rchd, hunknum, (const uint8_t *)buffer, &chd->file_end, chd_sink, chd->file);
	if (err == RCHD_ERROR_UNSUPPORTED)
		return CHDERR_FILE_NOT_WRITEABLE;
	return (err == RCHD_OK) ? CHDERR_NONE : CHDERR_WRITE_ERROR;
}



/***************************************************************************
    METADATA MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    chd_get_metadata - get the indexed metadata
    of the given type
-------------------------------------------------*/

chd_error chd_get_metadata(chd_file *chd, uint32_t searchtag, uint32_t searchindex, void *output, uint32_t outputlen, uint32_t *resultlen, uint32_t *resulttag, uint8_t *resultflags)
{
	const rchd_metadata_t *m;

	wait_for_pending_async(chd);

	if (searchtag == CHDMETATAG_WILDCARD)
		m = rchd_metadata(chd->rchd, searchindex);
	else
		m = rchd_metadata_find(chd->rchd, searchtag, searchindex);
	if (m == NULL)
		return CHDERR_METADATA_NOT_FOUND;

	memcpy(output, m->data, MIN(outputlen, m->length));
	if (resultlen != NULL)
		*resultlen = m->length;
	if (resulttag != NULL)
		*resulttag = m->tag;
	if (resultflags != NULL)
		*resultflags = m->flags;
	return CHDERR_NONE;
}



/***************************************************************************
    CODEC INTERFACES
***************************************************************************/

/*-------------------------------------------------
    chd_codec_config - set internal codec
    parameters
-------------------------------------------------*/

chd_error chd_codec_config(chd_file *chd, int param, void *config)
{
	wait_for_pending_async(chd);

	if (param != AV_CODEC_DECOMPRESS_CONFIG || chd->header.compression != CHDCOMPRESSION_AV)
		return CHDERR_INVALID_PARAMETER;
	chd->avconfig = *(av_codec_decompress_config *)config;
	chd->have_avconfig = TRUE;
	return CHDERR_NONE;
}
