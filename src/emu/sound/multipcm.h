#pragma once

#ifndef __MULTIPCM_H__
#define __MULTIPCM_H__

#include "devlegcy.h"

WRITE8_DEVICE_HANDLER( multipcm_w );
READ8_DEVICE_HANDLER( multipcm_r );

void multipcm_set_bank(running_device *device, uint32_t leftoffs, uint32_t rightoffs);

DECLARE_LEGACY_SOUND_DEVICE(MULTIPCM, multipcm);

#endif /* __MULTIPCM_H__ */
