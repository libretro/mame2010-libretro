class cabal_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cabal_state(machine)); }

	cabal_state(running_machine &machine) { }

	uint16_t *spriteram;
	uint16_t *colorram;
	uint16_t *videoram;
	size_t spriteram_size;
	tilemap_t *background_layer;
	tilemap_t *text_layer;
	int sound_command1;
	int sound_command2;
	int last[4];
};


/*----------- defined in video/cabal.c -----------*/

extern VIDEO_START( cabal );
extern VIDEO_UPDATE( cabal );
WRITE16_HANDLER( cabal_flipscreen_w );
WRITE16_HANDLER( cabal_background_videoram16_w );
WRITE16_HANDLER( cabal_text_videoram16_w );
