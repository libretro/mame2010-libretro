/*************************************************************************

    Slapshot / Operation Wolf 3

*************************************************************************/

struct slapshot_tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class slapshot_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, slapshot_state(machine)); }

	slapshot_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *    color_ram;
	uint16_t *    spriteram;
	uint16_t *    spriteext;
	uint16_t *    spriteram_buffered, *spriteram_delayed;
//  uint16_t *    paletteram;    // currently this uses generic palette handling
	size_t      spriteext_size;
	size_t      spriteram_size;

	/* video-related */
	struct      slapshot_tempsprite *spritelist;
	int32_t       sprites_disabled, sprites_active_area, sprites_master_scrollx, sprites_master_scrolly;
	int         sprites_flipscreen;
	int         prepare_sprites;

	uint16_t      spritebank[8];

	/* misc */
	int32_t      banknum;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *tc0140syt;
	running_device *tc0480scp;
	running_device *tc0360pri;
	running_device *tc0640fio;
};


/*----------- defined in video/slapshot.c -----------*/

VIDEO_START( slapshot );
VIDEO_UPDATE( slapshot );
VIDEO_EOF( taito_no_buffer );
