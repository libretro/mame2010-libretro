#pragma once

#ifndef __C352_H__
#define __C352_H__

#include  "devlegcy.h"

/* Per-instance configuration. Pass via MDRV_SOUND_CONFIG.
 *
 *   divider:  clock-to-output-sample-rate divider. The C352 chip's per-voice
 *             processing cycle is 288 clocks (the spec-correct value). Older
 *             drivers in this codebase were tuned against an effective 192;
 *             leaving `divider` zero / not passing an interface preserves
 *             that legacy behaviour. Newer drivers should pass 288 together
 *             with a hardware-measured clock value. */
typedef struct _c352_interface c352_interface;
struct _c352_interface
{
	int divider;
};

READ16_DEVICE_HANDLER( c352_r );
WRITE16_DEVICE_HANDLER( c352_w );

DECLARE_LEGACY_SOUND_DEVICE(C352, c352);

#endif /* __C352_H__ */

