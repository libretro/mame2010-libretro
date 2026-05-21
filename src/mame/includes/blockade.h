#include "sound/discrete.h"
#include "sound/samples.h"

class blockade_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, blockade_state(machine)); }

	blockade_state(running_machine &machine) { }

	uint8_t *  videoram;

	/* video-related */
	tilemap_t  *bg_tilemap;

	/* input-related */
	uint8_t coin_latch;  /* Active Low */
	uint8_t just_been_reset;
};


/*----------- defined in video/blockade.c -----------*/

WRITE8_HANDLER( blockade_videoram_w );

VIDEO_START( blockade );
VIDEO_UPDATE( blockade );

/*----------- defined in audio/blockade.c -----------*/

extern const samples_interface blockade_samples_interface;
DISCRETE_SOUND_EXTERN( blockade );

WRITE8_DEVICE_HANDLER( blockade_sound_freq_w );
WRITE8_HANDLER( blockade_env_on_w );
WRITE8_HANDLER( blockade_env_off_w );
