/*************************************************************************

    Psikyo PS6807 (PS4)

*************************************************************************/

#define MASTER_CLOCK 57272700	// main oscillator frequency


class psikyo4_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, psikyo4_state(machine)); }

	psikyo4_state(running_machine &machine) { }

	/* memory pointers */
	uint32_t *       vidregs;
	uint32_t *       paletteram;
	uint32_t *       ram;
	uint32_t *       io_select;
	uint32_t *       bgpen_1;
	uint32_t *       bgpen_2;
	uint32_t *       spriteram;
	size_t         spriteram_size;

	/* video-related */
	double         oldbrt1, oldbrt2;

	/* misc */
	uint32_t         sample_offs;	// only used if ROMTEST = 1

	/* devices */
	running_device *maincpu;
};


/*----------- defined in video/psikyo4.c -----------*/

VIDEO_START( psikyo4 );
VIDEO_UPDATE( psikyo4 );
