class tunhunt_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, tunhunt_state(machine)); }

	tunhunt_state(running_machine &machine) { }

	uint8_t control;
	uint8_t *workram;
	uint8_t *spriteram;
	uint8_t *videoram;
	tilemap_t *fg_tilemap;
};


/*----------- defined in video/tunhunt.c -----------*/

WRITE8_HANDLER( tunhunt_videoram_w );

PALETTE_INIT( tunhunt );
VIDEO_START( tunhunt );
VIDEO_UPDATE( tunhunt );
