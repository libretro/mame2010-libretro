	/***************************************************************************

    ESD 16 Bit Games

***************************************************************************/

class esd16_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, esd16_state(machine)); }

	esd16_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *       vram_0;
	uint16_t *       vram_1;
	uint16_t *       scroll_0;
	uint16_t *       scroll_1;
	uint16_t *       spriteram;
	uint16_t *       head_layersize;
	uint16_t *       headpanic_platform_x;
	uint16_t *       headpanic_platform_y;
//  uint16_t *       paletteram;  // currently this uses generic palette handling
	size_t         spriteram_size;

	/* video-related */
	tilemap_t       *tilemap_0_16x16, *tilemap_1_16x16;
	tilemap_t       *tilemap_0, *tilemap_1;
	int           tilemap0_color;

	/* devices */
	running_device *audio_cpu;
	running_device *eeprom;
};


/*----------- defined in video/esd16.c -----------*/

WRITE16_HANDLER( esd16_vram_0_w );
WRITE16_HANDLER( esd16_vram_1_w );
WRITE16_HANDLER( esd16_tilemap0_color_w );

VIDEO_START( esd16 );
VIDEO_UPDATE( esd16 );
VIDEO_UPDATE( hedpanic );
VIDEO_UPDATE( hedpanio );
