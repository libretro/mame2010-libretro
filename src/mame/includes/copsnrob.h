/*************************************************************************

    Atari Cops'n Robbers hardware

*************************************************************************/

class copsnrob_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, copsnrob_state(machine)); }

	copsnrob_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *        videoram;
	uint8_t *        trucky;
	uint8_t *        truckram;
	uint8_t *        bulletsram;
	uint8_t *        cary;
	uint8_t *        carimage;
	size_t         videoram_size;

	/* misc */
	uint8_t          misc;
};


/*----------- defined in machine/copsnrob.c -----------*/

READ8_HANDLER( copsnrob_gun_position_r );


/*----------- defined in video/copsnrob.c -----------*/

VIDEO_UPDATE( copsnrob );
