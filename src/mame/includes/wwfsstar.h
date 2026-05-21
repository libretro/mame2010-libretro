class wwfsstar_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, wwfsstar_state(machine)); }

	wwfsstar_state(running_machine &machine) { }

	int vblank;
	int scrollx;
	int scrolly;
	uint16_t *spriteram;
	uint16_t *fg0_videoram;
	uint16_t *bg0_videoram;
	tilemap_t *fg0_tilemap;
	tilemap_t *bg0_tilemap;
};


/*----------- defined in video/wwfsstar.c -----------*/

VIDEO_START( wwfsstar );
VIDEO_UPDATE( wwfsstar );
WRITE16_HANDLER( wwfsstar_fg0_videoram_w );
WRITE16_HANDLER( wwfsstar_bg0_videoram_w );
