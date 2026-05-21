
class mcatadv_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mcatadv_state(machine)); }

	mcatadv_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *     videoram1;
	uint16_t *     videoram2;
	uint16_t *     scroll1;
	uint16_t *     scroll2;
	uint16_t *     spriteram;
	uint16_t *     spriteram_old;
	uint16_t *     vidregs;
	uint16_t *     vidregs_old;
//  uint16_t *     paletteram;    // this currently uses generic palette handlers
	size_t       spriteram_size;

	/* video-related */
	tilemap_t    *tilemap1,  *tilemap2;
	int palette_bank1, palette_bank2;

	/* devices */
	running_device *maincpu;
	running_device *soundcpu;
};

/*----------- defined in video/mcatadv.c -----------*/

VIDEO_UPDATE( mcatadv );
VIDEO_START( mcatadv );
VIDEO_EOF( mcatadv );

WRITE16_HANDLER( mcatadv_videoram1_w );
WRITE16_HANDLER( mcatadv_videoram2_w );
