

class fantland_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, fantland_state(machine)); }

	fantland_state(running_machine &machine) { }

	/* memory pointers */
//  uint8_t *    spriteram;   // currently directly used in a 16bit map...
//  uint8_t *    spriteram_2; // currently directly used in a 16bit map...
//  uint8_t *    paletteram;  // currently this uses generic palette handling

	/* misc */
	uint8_t      nmi_enable;
	int        old_x[2], old_y[2], old_f[2];
	uint8_t      input_ret[2];
	int        adpcm_playing[4];
	int        adpcm_addr[2][4];
	int        adpcm_nibble[4];

	/* devices */
	running_device *audio_cpu;
	running_device *msm1;
	running_device *msm2;
	running_device *msm3;
	running_device *msm4;
};


/*----------- defined in video/fantland.c -----------*/

VIDEO_UPDATE( fantland );
