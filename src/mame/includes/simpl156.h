/*************************************************************************

    Simple 156 based board

*************************************************************************/

#include "machine/eeprom.h"
#include "sound/okim6295.h"
#include "video/deco16ic.h"

class simpl156_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, simpl156_state(machine)); }

	simpl156_state(running_machine &machine)
		: maincpu(machine.device<cpu_device>("maincpu")),
		  deco16ic(machine.device<deco16ic_device>("deco_custom")),
		  eeprom(machine.device<eeprom_device>("eeprom")),
		  okimusic(machine.device<okim6295_device>("okimusic")) { }

	/* memory pointers */
	uint16_t *  pf1_rowscroll;
	uint16_t *  pf2_rowscroll;
	uint32_t *  mainram;
	uint32_t *  systemram;

	/* devices */
	cpu_device *maincpu;
	deco16ic_device *deco16ic;
	eeprom_device *eeprom;
	okim6295_device *okimusic;
};



/*----------- defined in video/simpl156.c -----------*/

VIDEO_START( simpl156 );
VIDEO_UPDATE( simpl156 );
