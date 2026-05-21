/***************************************************************************

    Aeroboto

***************************************************************************/

class aeroboto_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, aeroboto_state(machine)); }

	aeroboto_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t * mainram;
	uint8_t * spriteram;
	uint8_t * videoram;
	uint8_t * hscroll;
	uint8_t * vscroll;
	uint8_t * tilecolor;
	uint8_t * starx;
	uint8_t * stary;
	uint8_t * bgcolor;
	size_t  spriteram_size;

	/* stars layout */
	uint8_t * stars_rom;
	int     stars_length;

	/* video-related */
	tilemap_t *bg_tilemap;
	int     charbank, starsoff;
	int     sx, sy;
	uint8_t   ox, oy;

	/* misc */
	int count;
	int disable_irq;
};


/*----------- defined in video/aeroboto.c -----------*/

VIDEO_START( aeroboto );
VIDEO_UPDATE( aeroboto );

READ8_HANDLER( aeroboto_in0_r );
WRITE8_HANDLER( aeroboto_3000_w );
WRITE8_HANDLER( aeroboto_videoram_w );
WRITE8_HANDLER( aeroboto_tilecolor_w );
