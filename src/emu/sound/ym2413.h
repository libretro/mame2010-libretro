#pragma once

#ifndef __YM2413_H__
#define __YM2413_H__

/* select output bits size of output : 8 or 16 */
#define SAMPLE_BITS 16

/* compiler dependence */
#ifndef __OSDCOMM_H__
#define __OSDCOMM_H__
typedef unsigned char	uint8_t;   /* unsigned  8bit */
typedef unsigned short	uint16_t;  /* unsigned 16bit */
typedef unsigned int	uint32_t;  /* unsigned 32bit */
typedef signed char		int8_t;    /* signed  8bit   */
typedef signed short	int16_t;   /* signed 16bit   */
typedef signed int		int32_t;   /* signed 32bit   */
#endif

typedef stream_sample_t SAMP;
/*
#if (SAMPLE_BITS==16)
typedef int16_t SAMP;
#endif
#if (SAMPLE_BITS==8)
typedef int8_t SAMP;
#endif
*/



void *ym2413_init(running_device *device, int clock, int rate);
void ym2413_shutdown(void *chip);
void ym2413_reset_chip(void *chip);
void ym2413_write(void *chip, int a, int v);
unsigned char ym2413_read(void *chip, int a);
void ym2413_update_one(void *chip, SAMP **buffers, int length);

typedef void (*OPLL_UPDATEHANDLER)(void *param,int min_interval_us);

void ym2413_set_update_handler(void *chip, OPLL_UPDATEHANDLER UpdateHandler, void *param);

#endif /*__YM2413_H__*/
