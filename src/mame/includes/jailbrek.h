/***************************************************************************

    Jailbreak

***************************************************************************/

#define MASTER_CLOCK        XTAL_18_432MHz
#define VOICE_CLOCK         XTAL_3_579545MHz

class jailbrek_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, jailbrek_state(machine)); }

	jailbrek_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *      videoram;
	uint8_t *      colorram;
	uint8_t *      spriteram;
	uint8_t *      scroll_x;
	uint8_t *      scroll_dir;
	size_t       spriteram_size;

	/* video-related */
	tilemap_t      *bg_tilemap;

	/* misc */
	uint8_t        irq_enable, nmi_enable;
};


/*----------- defined in video/jailbrek.c -----------*/

WRITE8_HANDLER( jailbrek_videoram_w );
WRITE8_HANDLER( jailbrek_colorram_w );

PALETTE_INIT( jailbrek );
VIDEO_START( jailbrek );
VIDEO_UPDATE( jailbrek );
