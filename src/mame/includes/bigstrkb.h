class bigstrkb_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, bigstrkb_state(machine)); }

	bigstrkb_state(running_machine &machine) { }

	tilemap_t *tilemap;
	tilemap_t *tilemap2;
	tilemap_t *tilemap3;

	uint16_t *videoram;
	uint16_t *videoram2;
	uint16_t *videoram3;

	uint16_t *vidreg1;
	uint16_t *vidreg2;
	uint16_t *spriteram;
};


/*----------- defined in video/bigstrkb.c -----------*/

WRITE16_HANDLER( bsb_videoram_w );
WRITE16_HANDLER( bsb_videoram2_w );
WRITE16_HANDLER( bsb_videoram3_w );
VIDEO_START(bigstrkb);
VIDEO_UPDATE(bigstrkb);
