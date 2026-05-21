
class crshrace_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, crshrace_state(machine)); }

	crshrace_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *  videoram1;
	uint16_t *  videoram2;
//  uint16_t *  spriteram1;   // currently this uses generic buffered spriteram
//  uint16_t *  spriteram2;   // currently this uses generic buffered spriteram
//      uint16_t *  paletteram;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t   *tilemap1, *tilemap2;
	int       roz_bank, gfxctrl, flipscreen;

	/* misc */
	int pending_command;

	/* devices */
	running_device *audiocpu;
	running_device *k053936;
};

/*----------- defined in video/crshrace.c -----------*/

WRITE16_HANDLER( crshrace_videoram1_w );
WRITE16_HANDLER( crshrace_videoram2_w );
WRITE16_HANDLER( crshrace_roz_bank_w );
WRITE16_HANDLER( crshrace_gfxctrl_w );

VIDEO_START( crshrace );
VIDEO_EOF( crshrace );
VIDEO_UPDATE( crshrace );
