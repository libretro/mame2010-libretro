/***************************************************************************

    Blomby Car

***************************************************************************/

class blmbycar_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, blmbycar_state(machine)); }

	blmbycar_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *    vram_0;
	uint16_t *    scroll_0;
	uint16_t *    vram_1;
	uint16_t *    scroll_1;
	uint16_t *    spriteram;
	uint16_t *    paletteram;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *tilemap_0, *tilemap_1;

	/* input-related */
	uint8_t       pot_wheel;	// blmbycar
	int         old_val;	// blmbycar
	int         retvalue;	// waterball
};


/*----------- defined in video/blmbycar.c -----------*/

WRITE16_HANDLER( blmbycar_palette_w );

WRITE16_HANDLER( blmbycar_vram_0_w );
WRITE16_HANDLER( blmbycar_vram_1_w );

VIDEO_START( blmbycar );
VIDEO_UPDATE( blmbycar );
