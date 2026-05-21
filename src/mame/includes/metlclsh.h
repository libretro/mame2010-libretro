/*************************************************************************

    Metal Clash

*************************************************************************/

class metlclsh_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, metlclsh_state(machine)); }

	metlclsh_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *        bgram;
	uint8_t *        fgram;
	uint8_t *        scrollx;
	uint8_t *        otherram;
//      uint8_t *        paletteram;    // currently this uses generic palette handling
//      uint8_t *        paletteram2;    // currently this uses generic palette handling
	uint8_t *        spriteram;
	size_t         spriteram_size;

	/* video-related */
	tilemap_t      *bg_tilemap,*fg_tilemap;
	uint8_t          write_mask, gfxbank;

	/* devices */
	running_device *maincpu;
	running_device *subcpu;
};


/*----------- defined in video/metlclsh.c -----------*/

WRITE8_HANDLER( metlclsh_bgram_w );
WRITE8_HANDLER( metlclsh_fgram_w );
WRITE8_HANDLER( metlclsh_gfxbank_w );
WRITE8_HANDLER( metlclsh_rambank_w );

VIDEO_START( metlclsh );
VIDEO_UPDATE( metlclsh );
