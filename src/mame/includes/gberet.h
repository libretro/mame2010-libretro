/***************************************************************************

    Green Beret

***************************************************************************/

class gberet_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gberet_state(machine)); }

	gberet_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *     videoram;
	uint8_t *     colorram;
	uint8_t *     spriteram;
	uint8_t *     spriteram2;
	uint8_t *     scrollram;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *bg_tilemap;
	uint8_t       spritebank;

	/* misc */
	uint8_t       nmi_enable, irq_enable;
};


/*----------- defined in video/gberet.c -----------*/

WRITE8_HANDLER( gberet_videoram_w );
WRITE8_HANDLER( gberet_colorram_w );
WRITE8_HANDLER( gberet_scroll_w );
WRITE8_HANDLER( gberetb_scroll_w );
WRITE8_HANDLER( gberet_sprite_bank_w );

PALETTE_INIT( gberet );
VIDEO_START( gberet );
VIDEO_UPDATE( gberet );
VIDEO_UPDATE( gberetb );
