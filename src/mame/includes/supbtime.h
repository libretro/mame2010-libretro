/*************************************************************************

    Super Burger Time & China Town

*************************************************************************/

class supbtime_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, supbtime_state(machine)); }

	supbtime_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *  pf1_rowscroll;
	uint16_t *  pf2_rowscroll;
	uint16_t *  spriteram;
//  uint16_t *  paletteram;    // currently this uses generic palette handling (in deco16ic.c)
	size_t    spriteram_size;

	/* video-related */

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *deco16ic;
};



/*----------- defined in video/supbtime.c -----------*/

VIDEO_UPDATE( supbtime );
