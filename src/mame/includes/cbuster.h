/*************************************************************************

    Crude Buster

*************************************************************************/

class cbuster_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cbuster_state(machine)); }

	cbuster_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *  pf1_rowscroll;
	uint16_t *  pf2_rowscroll;
	uint16_t *  pf3_rowscroll;
	uint16_t *  pf4_rowscroll;
	uint16_t *  ram;

	/* misc */
	uint16_t    prot;
	int       pri;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *deco16ic;
};



/*----------- defined in video/cbuster.c -----------*/

WRITE16_HANDLER( twocrude_palette_24bit_rg_w );
WRITE16_HANDLER( twocrude_palette_24bit_b_w );

VIDEO_UPDATE( twocrude );
