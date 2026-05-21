/*************************************************************************

    Hana Awase

*************************************************************************/

class hanaawas_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, hanaawas_state(machine)); }

	hanaawas_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    colorram;

	/* video-related */
	tilemap_t    *bg_tilemap;

	/* misc */
	int        mux;
};


/*----------- defined in video/hanaawas.c -----------*/

WRITE8_HANDLER( hanaawas_videoram_w );
WRITE8_HANDLER( hanaawas_colorram_w );
WRITE8_DEVICE_HANDLER( hanaawas_portB_w );

PALETTE_INIT( hanaawas );
VIDEO_START( hanaawas );
VIDEO_UPDATE( hanaawas );
