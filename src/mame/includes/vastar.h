class vastar_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, vastar_state(machine)); }

	vastar_state(running_machine &machine) { }

	uint8_t *spriteram1;
	uint8_t *spriteram2;
	uint8_t *spriteram3;

	uint8_t *bg1videoram;
	uint8_t *bg2videoram;
	uint8_t *fgvideoram;
	uint8_t *bg1_scroll;
	uint8_t *bg2_scroll;
	uint8_t *sprite_priority;

	tilemap_t *fg_tilemap;
	tilemap_t *bg1_tilemap;
	tilemap_t *bg2_tilemap;

	uint8_t *sharedram;
};


/*----------- defined in video/vastar.c -----------*/

WRITE8_HANDLER( vastar_bg1videoram_w );
WRITE8_HANDLER( vastar_bg2videoram_w );
WRITE8_HANDLER( vastar_fgvideoram_w );
READ8_HANDLER( vastar_bg1videoram_r );
READ8_HANDLER( vastar_bg2videoram_r );

VIDEO_START( vastar );
VIDEO_UPDATE( vastar );
