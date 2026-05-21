/***************************************************************************

    Finalizer

***************************************************************************/

class finalizr_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, finalizr_state(machine)); }

	finalizr_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *       videoram;
	uint8_t *       colorram;
	uint8_t *       videoram2;
	uint8_t *       colorram2;
	uint8_t *       scroll;
	uint8_t *       spriteram;
	uint8_t *       spriteram_2;
	size_t        videoram_size;
	size_t        spriteram_size;

	/* video-related */
	tilemap_t       *fg_tilemap, *bg_tilemap;
	int           spriterambank, charbank;

	/* misc */
	int           T1_line;
	uint8_t         nmi_enable, irq_enable;

	/* devices */
	running_device *audio_cpu;
};


/*----------- defined in video/finalizr.c -----------*/

WRITE8_HANDLER( finalizr_videoctrl_w );

PALETTE_INIT( finalizr );
VIDEO_START( finalizr );
VIDEO_UPDATE( finalizr );
