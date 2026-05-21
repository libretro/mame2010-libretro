/*************************************************************************

    Model Racing Dribbling hardware

*************************************************************************/



class dribling_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, dribling_state(machine)); }

	dribling_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  videoram;
	uint8_t *  colorram;

	/* misc */
	uint8_t    abca;
	uint8_t    dr, ds, sh;
	uint8_t    input_mux;
	uint8_t    di;

	/* devices */
	running_device *maincpu;
	running_device *ppi_0;
	running_device *ppi_1;
};


/*----------- defined in video/dribling.c -----------*/

PALETTE_INIT( dribling );
WRITE8_HANDLER( dribling_colorram_w );
VIDEO_UPDATE( dribling );
