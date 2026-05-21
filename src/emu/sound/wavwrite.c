#include "osdcore.h"
#include "sound/wavwrite.h"

struct _wav_file
{
	FILE *file;
	uint32_t total_offs;
	uint32_t data_offs;
};


wav_file *wav_open(const char *filename, int sample_rate, int channels)
{
	wav_file *wav;
	uint32_t bps, temp32;
	uint16_t align, temp16;

	/* allocate memory for the wav struct */
	wav = (wav_file *) osd_malloc(sizeof(struct _wav_file));
	if (!wav)
		return NULL;

	/* create the file */
	wav->file = fopen(filename, "wb");
	if (!wav->file)
	{
		osd_free(wav);
		return NULL;
	}

	/* write the 'RIFF' header */
	fwrite("RIFF", 1, 4, wav->file);

	/* write the total size */
	temp32 = 0;
	wav->total_offs = ftell(wav->file);
	fwrite(&temp32, 1, 4, wav->file);

	/* write the 'WAVE' type */
	fwrite("WAVE", 1, 4, wav->file);

	/* write the 'fmt ' tag */
	fwrite("fmt ", 1, 4, wav->file);

	/* write the format length */
	temp32 = LITTLE_ENDIANIZE_INT32(16);
	fwrite(&temp32, 1, 4, wav->file);

	/* write the format (PCM) */
	temp16 = LITTLE_ENDIANIZE_INT16(1);
	fwrite(&temp16, 1, 2, wav->file);

	/* write the channels */
	temp16 = LITTLE_ENDIANIZE_INT16(channels);
	fwrite(&temp16, 1, 2, wav->file);

	/* write the sample rate */
	temp32 = LITTLE_ENDIANIZE_INT32(sample_rate);
	fwrite(&temp32, 1, 4, wav->file);

	/* write the bytes/second */
	bps = sample_rate * 2 * channels;
	temp32 = LITTLE_ENDIANIZE_INT32(bps);
	fwrite(&temp32, 1, 4, wav->file);

	/* write the block align */
	align = 2 * channels;
	temp16 = LITTLE_ENDIANIZE_INT16(align);
	fwrite(&temp16, 1, 2, wav->file);

	/* write the bits/sample */
	temp16 = LITTLE_ENDIANIZE_INT16(16);
	fwrite(&temp16, 1, 2, wav->file);

	/* write the 'data' tag */
	fwrite("data", 1, 4, wav->file);

	/* write the data length */
	temp32 = 0;
	wav->data_offs = ftell(wav->file);
	fwrite(&temp32, 1, 4, wav->file);

	return wav;
}


void wav_close(wav_file *wav)
{
	uint32_t total = ftell(wav->file);
	uint32_t temp32;

	if (!wav) return;

	/* update the total file size */
	fseek(wav->file, wav->total_offs, SEEK_SET);
	temp32 = total - (wav->total_offs + 4);
	temp32 = LITTLE_ENDIANIZE_INT32(temp32);
	fwrite(&temp32, 1, 4, wav->file);

	/* update the data size */
	fseek(wav->file, wav->data_offs, SEEK_SET);
	temp32 = total - (wav->data_offs + 4);
	temp32 = LITTLE_ENDIANIZE_INT32(temp32);
	fwrite(&temp32, 1, 4, wav->file);

	fclose(wav->file);
	osd_free(wav);
}


void wav_add_data_16(wav_file *wav, int16_t *data, int samples)
{
	if (!wav) return;

	/* just write and flush the data */
	fwrite(data, 2, samples, wav->file);
	fflush(wav->file);
}


void wav_add_data_32(wav_file *wav, int32_t *data, int samples, int shift)
{
	int16_t *temp;
	int i;

	if (!wav) return;

	/* allocate temp memory */
	temp = (int16_t *)osd_malloc(samples * sizeof(temp[0]));
	if (!temp)
		return;

	/* clamp */
	for (i = 0; i < samples; i++)
	{
		int val = data[i] >> shift;
		temp[i] = (val < -32768) ? -32768 : (val > 32767) ? 32767 : val;
	}

	/* write and flush */
	fwrite(temp, 2, samples, wav->file);
	fflush(wav->file);

	/* free memory */
	osd_free(temp);
}


void wav_add_data_16lr(wav_file *wav, int16_t *left, int16_t *right, int samples)
{
	int16_t *temp;
	int i;

	if (!wav) return;

	/* allocate temp memory */
	temp = (int16_t *)osd_malloc(samples * 2 * sizeof(temp[0]));
	if (!temp)
		return;

	/* interleave */
	for (i = 0; i < samples * 2; i++)
		temp[i] = (i & 1) ? right[i / 2] : left[i / 2];

	/* write and flush */
	fwrite(temp, 4, samples, wav->file);
	fflush(wav->file);

	/* free memory */
	osd_free(temp);
}


void wav_add_data_32lr(wav_file *wav, int32_t *left, int32_t *right, int samples, int shift)
{
	int16_t *temp;
	int i;

	if (!wav) return;

	/* allocate temp memory */
	temp = (int16_t *)osd_malloc(samples * 2 * sizeof(temp[0]));
	if (!temp)
		return;

	/* interleave */
	for (i = 0; i < samples * 2; i++)
	{
		int val = (i & 1) ? right[i / 2] : left[i / 2];
		val >>= shift;
		temp[i] = (val < -32768) ? -32768 : (val > 32767) ? 32767 : val;
	}

	/* write and flush */
	fwrite(temp, 4, samples, wav->file);
	fflush(wav->file);

	/* free memory */
	osd_free(temp);
}
