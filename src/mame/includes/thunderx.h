/*************************************************************************

    Super Contra / Thunder Cross

*************************************************************************/

class thunderx_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, thunderx_state(machine)); }

	thunderx_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    ram;
	uint8_t *    pmcram;
//  uint8_t *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;

	/* misc */
	int        priority;
	uint8_t      _1f98_data;
	int        palette_selected;
	int        rambank, pmcbank;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k007232;
	running_device *k052109;
	running_device *k051960;
};


/*----------- defined in video/thunderx.c -----------*/

extern void thunderx_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
extern void thunderx_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow);

VIDEO_START( scontra );
VIDEO_UPDATE( scontra );
