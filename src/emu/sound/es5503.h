#pragma once

#ifndef __ES5503_H__
#define __ES5503_H__

#include "devlegcy.h"

typedef struct _es5503_interface es5503_interface;
struct _es5503_interface
{
	void (*irq_callback)(running_device *device, int state);
	read8_device_func adc_read;
	uint8_t *wave_memory;
};

READ8_DEVICE_HANDLER( es5503_r );
WRITE8_DEVICE_HANDLER( es5503_w );
void es5503_set_base(running_device *device, uint8_t *wavemem);

DECLARE_LEGACY_SOUND_DEVICE(ES5503, es5503);

#endif /* __ES5503_H__ */
