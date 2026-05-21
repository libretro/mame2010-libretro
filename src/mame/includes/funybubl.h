

class funybubl_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, funybubl_state(machine)); }

	funybubl_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    banked_vram;
	uint8_t *    paletteram;

	/* devices */
	running_device *audiocpu;
};



/*----------- defined in video/funybubl.c -----------*/

WRITE8_HANDLER ( funybubl_paldatawrite );

VIDEO_START(funybubl);
VIDEO_UPDATE(funybubl);
