/*************************************************************************

Crazy Ballooon

*************************************************************************/


#define CRBALOON_MASTER_XTAL	(XTAL_9_987MHz)


/*----------- defined in audio/crbaloon.c -----------*/

WRITE8_DEVICE_HANDLER( crbaloon_audio_set_music_freq );
WRITE8_DEVICE_HANDLER( crbaloon_audio_set_music_enable );
void crbaloon_audio_set_explosion_enable(running_device *sn, int enabled);
void crbaloon_audio_set_breath_enable(running_device *sn, int enabled);
void crbaloon_audio_set_appear_enable(running_device *sn, int enabled);
WRITE8_DEVICE_HANDLER( crbaloon_audio_set_laugh_enable );

MACHINE_DRIVER_EXTERN( crbaloon_audio );


/*----------- defined in video/crbaloon.c -----------*/

extern uint8_t *crbaloon_videoram;
extern uint8_t *crbaloon_colorram;
extern uint8_t *crbaloon_spriteram;

PALETTE_INIT( crbaloon );
VIDEO_START( crbaloon );
VIDEO_UPDATE( crbaloon );

WRITE8_HANDLER( crbaloon_videoram_w );
WRITE8_HANDLER( crbaloon_colorram_w );

uint16_t crbaloon_get_collision_address(void);
void crbaloon_set_clear_collision_address(int _crbaloon_collision_address_clear);

