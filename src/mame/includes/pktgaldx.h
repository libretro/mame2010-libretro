/*************************************************************************

    Pocket Gal Deluxe

*************************************************************************/

class pktgaldx_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, pktgaldx_state(machine)); }

	pktgaldx_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *  pf1_rowscroll;
	uint16_t *  pf2_rowscroll;
	uint16_t *  spriteram;
//  uint16_t *  paletteram;    // currently this uses generic palette handling (in deco16ic.c)
	size_t    spriteram_size;

	uint16_t*   pktgaldb_fgram;
	uint16_t*   pktgaldb_sprites;

	/* devices */
	running_device *maincpu;
	running_device *deco16ic;
};



/*----------- defined in video/pktgaldx.c -----------*/

VIDEO_UPDATE( pktgaldx );
VIDEO_UPDATE( pktgaldb );
