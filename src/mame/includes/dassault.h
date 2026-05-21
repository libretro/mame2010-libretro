/*************************************************************************

    Desert Assault

*************************************************************************/

#include "sound/okim6295.h"
#include "video/deco16ic.h"

class dassault_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, dassault_state(machine)); }

	dassault_state(running_machine &machine)
		: maincpu(machine.device<cpu_device>("maincpu")),
		  audiocpu(machine.device<cpu_device>("audiocpu")),
		  subcpu(machine.device<cpu_device>("sub")),
		  deco16ic(machine.device<deco16ic_device>("deco_custom")),
		  oki2(machine.device<okim6295_device>("oki2")) { }

	/* memory pointers */
	uint16_t *  pf2_rowscroll;
	uint16_t *  pf4_rowscroll;
	uint16_t *  ram;
	uint16_t *  ram2;
	uint16_t *  shared_ram;

	/* devices */
	cpu_device *maincpu;
	cpu_device *audiocpu;
	cpu_device *subcpu;
	deco16ic_device *deco16ic;
	okim6295_device *oki2;
};



/*----------- defined in video/dassault.c -----------*/

VIDEO_UPDATE( dassault );
