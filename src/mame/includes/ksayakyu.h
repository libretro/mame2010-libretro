/*************************************************************************

    Kusayakyuu

*************************************************************************/

class ksayakyu_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ksayakyu_state(machine)); }

	ksayakyu_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    videoram;
	uint8_t *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *tilemap, *textmap;
	int        video_ctrl, flipscreen;

	/* misc */
	int        sound_status;
};


/*----------- defined in video/ksayakyu.c -----------*/

WRITE8_HANDLER( ksayakyu_videoram_w );
WRITE8_HANDLER( ksayakyu_videoctrl_w );
PALETTE_INIT( ksayakyu );
VIDEO_START( ksayakyu );
VIDEO_UPDATE( ksayakyu );
