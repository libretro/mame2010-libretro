/*************************************************************************

    Othello Derby

*************************************************************************/

#define OTHLDRBY_VREG_SIZE   18

class othldrby_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, othldrby_state(machine)); }

	othldrby_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *     vram;
	uint16_t *     buf_spriteram;
	uint16_t *     buf_spriteram2;

	/* video-related */
	tilemap_t    *bg_tilemap[3];
	uint16_t       vreg[OTHLDRBY_VREG_SIZE];
	uint32_t       vram_addr, vreg_addr;

	/* misc */
	int          toggle;
};


/*----------- defined in video/othldrby.c -----------*/

WRITE16_HANDLER( othldrby_videoram_addr_w );
READ16_HANDLER( othldrby_videoram_r );
WRITE16_HANDLER( othldrby_videoram_w );
WRITE16_HANDLER( othldrby_vreg_addr_w );
WRITE16_HANDLER( othldrby_vreg_w );

VIDEO_START( othldrby );
VIDEO_EOF( othldrby );
VIDEO_UPDATE( othldrby );
