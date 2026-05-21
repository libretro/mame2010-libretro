
class mugsmash_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mugsmash_state(machine)); }

	mugsmash_state(running_machine &machine) { }

	uint16_t *videoram1;
	uint16_t *videoram2;
	uint16_t *spriteram;
	uint16_t *regs1;
	uint16_t *regs2;

	tilemap_t *tilemap1;
	tilemap_t *tilemap2;

	running_device *maincpu;
	running_device *audiocpu;
};


/*----------- defined in video/mugsmash.c -----------*/

VIDEO_START( mugsmash );
VIDEO_UPDATE( mugsmash );

WRITE16_HANDLER( mugsmash_reg_w );
WRITE16_HANDLER( mugsmash_videoram2_w );
WRITE16_HANDLER( mugsmash_videoram1_w );
