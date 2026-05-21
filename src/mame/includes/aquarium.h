


class aquarium_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, aquarium_state(machine)); }

	aquarium_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *  scroll;
	uint16_t *  txt_videoram;
	uint16_t *  mid_videoram;
	uint16_t *  bak_videoram;
	uint16_t *  spriteram;
//  uint16_t *  paletteram;   // currently this uses generic palette handling
	size_t    spriteram_size;

	/* video-related */
	tilemap_t  *txt_tilemap, *mid_tilemap, *bak_tilemap;

	/* misc */
	int aquarium_snd_ack;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/aquarium.c -----------*/

WRITE16_HANDLER( aquarium_txt_videoram_w );
WRITE16_HANDLER( aquarium_mid_videoram_w );
WRITE16_HANDLER( aquarium_bak_videoram_w );

VIDEO_START(aquarium);
VIDEO_UPDATE(aquarium);
