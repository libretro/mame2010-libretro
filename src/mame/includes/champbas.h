/*************************************************************************

    Talbot - Champion Base Ball - Exciting Soccer

*************************************************************************/


#define CPUTAG_MCU "mcu"


class champbas_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, champbas_state(machine)); }

	champbas_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *        bg_videoram;
	uint8_t *        spriteram;
	uint8_t *        spriteram_2;
	size_t         spriteram_size;

	/* video-related */
	tilemap_t        *bg_tilemap;
	uint8_t          gfx_bank;
	uint8_t          palette_bank;

	/* misc */
	int            watchdog_count;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *mcu;
};


/*----------- defined in video/champbas.c -----------*/

WRITE8_HANDLER( champbas_bg_videoram_w );
WRITE8_HANDLER( champbas_gfxbank_w );
WRITE8_HANDLER( champbas_palette_bank_w );
WRITE8_HANDLER( champbas_flipscreen_w );

PALETTE_INIT( champbas );
PALETTE_INIT( exctsccr );
VIDEO_START( champbas );
VIDEO_START( exctsccr );
VIDEO_UPDATE( champbas );
VIDEO_UPDATE( exctsccr );


