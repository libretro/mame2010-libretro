/***************************************************************************

    PSX SPU

    preliminary version by smf.

***************************************************************************/

#pragma once

#ifndef __SOUND_PSX_H__
#define __SOUND_PSX_H__

#include "devlegcy.h"

WRITE32_DEVICE_HANDLER( psx_spu_w );
READ32_DEVICE_HANDLER( psx_spu_r );
WRITE32_DEVICE_HANDLER( psx_spu_delay_w );
READ32_DEVICE_HANDLER( psx_spu_delay_r );

typedef void ( *spu_handler )( running_machine *, uint32_t, int32_t );

typedef struct _psx_spu_interface psx_spu_interface;
struct _psx_spu_interface
{
	uint32_t **p_psxram;
	void (*irq_set)(running_device *,uint32_t);
	void (*spu_install_read_handler)(int,spu_handler);
	void (*spu_install_write_handler)(int,spu_handler);
};

DECLARE_LEGACY_SOUND_DEVICE(PSXSPU, psxspu);

#endif /* __SOUND_PSX_H__ */
