
class lemmings_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, lemmings_state(machine)); }

	lemmings_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *  pixel_0_data;
	uint16_t *  pixel_1_data;
	uint16_t *  vram_data;
	uint16_t *  control_data;
	uint16_t *  paletteram;
//  uint16_t *  spriteram;    // this currently uses generic buffered spriteram
//  uint16_t *  spriteram2;   // this currently uses generic buffered spriteram

	/* video-related */
	bitmap_t *bitmap0;
	tilemap_t *vram_tilemap;
	uint16_t *sprite_triple_buffer_0,*sprite_triple_buffer_1;
	uint8_t *vram_buffer;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/lemmings.c -----------*/

WRITE16_HANDLER( lemmings_pixel_0_w );
WRITE16_HANDLER( lemmings_pixel_1_w );
WRITE16_HANDLER( lemmings_vram_w );

VIDEO_START( lemmings );
VIDEO_EOF( lemmings );
VIDEO_UPDATE( lemmings );
