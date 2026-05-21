/*************************************************************************

    Vapour Trail

*************************************************************************/

class vaportra_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, vaportra_state(machine)); }

	vaportra_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *  pf1_rowscroll;
	uint16_t *  pf2_rowscroll;
	uint16_t *  pf3_rowscroll;
	uint16_t *  pf4_rowscroll;

	/* misc */
	uint16_t    priority[2];

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *deco16ic;
};



/*----------- defined in video/vaportra.c -----------*/

WRITE16_HANDLER( vaportra_priority_w );
WRITE16_HANDLER( vaportra_palette_24bit_rg_w );
WRITE16_HANDLER( vaportra_palette_24bit_b_w );

VIDEO_UPDATE( vaportra );
