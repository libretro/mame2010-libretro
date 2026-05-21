/*************************************************************************

    Bishi Bashi Champ Mini Game Senshuken

*************************************************************************/

#define CPU_CLOCK       (XTAL_24MHz / 2)		/* 68000 clock */
#define SOUND_CLOCK     XTAL_16_9344MHz		/* YMZ280 clock */

class bishi_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, bishi_state(machine)); }

	bishi_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *    ram;
//  uint8_t *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[4];

	/* misc */
	uint16_t     cur_control, cur_control2;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k007232;
	running_device *k056832;
	running_device *k054338;
	running_device *k055555;
};

/*----------- defined in video/bishi.c -----------*/

extern void bishi_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);

VIDEO_START(bishi);
VIDEO_UPDATE(bishi);
