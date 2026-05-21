

class fgoal_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, fgoal_state(machine)); }

	fgoal_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    video_ram;

	/* video-related */
	bitmap_t   *bgbitmap, *fgbitmap;
	uint8_t      xpos, ypos;
	int        current_color;

	/* misc */
	int        fgoal_player;
	uint8_t      row, col;
	int        prev_coin;

	/* devices */
	running_device *maincpu;
	running_device *mb14241;
};


/*----------- defined in video/fgoal.c -----------*/

VIDEO_START( fgoal );
VIDEO_UPDATE( fgoal );

WRITE8_HANDLER( fgoal_color_w );
WRITE8_HANDLER( fgoal_xpos_w );
WRITE8_HANDLER( fgoal_ypos_w );

