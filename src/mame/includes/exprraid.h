/*************************************************************************

    Express Raider

*************************************************************************/


class exprraid_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, exprraid_state(machine)); }

	exprraid_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *        main_ram;
	uint8_t *        videoram;
	uint8_t *        colorram;
	uint8_t *        spriteram;
	size_t         spriteram_size;

	/* video-related */
	tilemap_t        *bg_tilemap, *fg_tilemap;
	int            bg_index[4];

	/* misc */
	//int          coin;    // used in the commented out INTERRUPT_GEN - can this be removed?

	/* devices */
	running_device *maincpu;
	running_device *slave;
};


/*----------- defined in video/exprraid.c -----------*/

extern WRITE8_HANDLER( exprraid_videoram_w );
extern WRITE8_HANDLER( exprraid_colorram_w );
extern WRITE8_HANDLER( exprraid_flipscreen_w );
extern WRITE8_HANDLER( exprraid_bgselect_w );
extern WRITE8_HANDLER( exprraid_scrollx_w );
extern WRITE8_HANDLER( exprraid_scrolly_w );

extern VIDEO_START( exprraid );
extern VIDEO_UPDATE( exprraid );
