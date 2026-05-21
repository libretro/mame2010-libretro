
#define MASTER_CLOCK         XTAL_18_432MHz
#define SOUND_CLOCK          XTAL_3_579545MHz



class jackal_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, jackal_state(machine)); }

	jackal_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  videoctrl;
	uint8_t *  scrollram;
	uint8_t *  paletteram;

	/* video-related */
	tilemap_t  *bg_tilemap;

	/* misc */
	int      irq_enable;
	uint8_t    *rambank;
	uint8_t    *spritebank;

	/* devices */
	running_device *mastercpu;
	running_device *slavecpu;
};


/*----------- defined in video/jackal.c -----------*/

void jackal_mark_tile_dirty(running_machine *machine, int offset);

PALETTE_INIT( jackal );
VIDEO_START( jackal );
VIDEO_UPDATE( jackal );
