/*************************************************************************

    Skyfox

*************************************************************************/

class skyfox_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, skyfox_state(machine)); }

	skyfox_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	uint8_t      vreg[8];
	int        bg_pos, bg_ctrl;

	/* misc */
	int        palette_selected;

	/* devices */
	running_device *maincpu;
};

/*----------- defined in video/skyfox.c -----------*/

WRITE8_HANDLER( skyfox_vregs_w );

PALETTE_INIT( skyfox );

VIDEO_UPDATE( skyfox );

