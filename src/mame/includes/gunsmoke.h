/*************************************************************************

    Gun.Smoke

*************************************************************************/

class gunsmoke_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gunsmoke_state(machine)); }

	gunsmoke_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    colorram;
	uint8_t *    spriteram;
	uint8_t *    scrollx;
	uint8_t *    scrolly;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap, *fg_tilemap;
	uint8_t      chon, objon, bgon;
	uint8_t      sprite3bank;
};


/*----------- defined in video/gunsmoke.c -----------*/

WRITE8_HANDLER( gunsmoke_c804_w );
WRITE8_HANDLER( gunsmoke_d806_w );
WRITE8_HANDLER( gunsmoke_videoram_w );
WRITE8_HANDLER( gunsmoke_colorram_w );

PALETTE_INIT( gunsmoke );
VIDEO_START( gunsmoke );
VIDEO_UPDATE( gunsmoke );

