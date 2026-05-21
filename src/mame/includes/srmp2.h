class srmp2_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, srmp2_state(machine)); }

	srmp2_state(running_machine &machine) { }

	int color_bank;
	int gfx_bank;

	int adpcm_bank;
	int adpcm_data;
	uint32_t adpcm_sptr;
	uint32_t adpcm_eptr;

	int port_select;

	union
	{
		uint8_t *u8;
		uint16_t *u16;
	} spriteram1, spriteram2, spriteram3;
};


/*----------- defined in video/srmp2.c -----------*/

PALETTE_INIT( srmp2 );
VIDEO_UPDATE( srmp2 );
PALETTE_INIT( srmp3 );
VIDEO_UPDATE( srmp3 );
VIDEO_UPDATE( mjyuugi );
