class shootout_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, shootout_state(machine)); }

	shootout_state(running_machine &machine) { }

	tilemap_t *background;
	tilemap_t *foreground;
	uint8_t *spriteram;
	uint8_t *videoram;
	uint8_t *textram;
};


/*----------- defined in video/shootout.c -----------*/

WRITE8_HANDLER( shootout_videoram_w );
WRITE8_HANDLER( shootout_textram_w );

PALETTE_INIT( shootout );
VIDEO_START( shootout );
VIDEO_UPDATE( shootout );
VIDEO_UPDATE( shootouj );
