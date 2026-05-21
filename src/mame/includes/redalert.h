/***************************************************************************

    Irem Red Alert hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

****************************************************************************/

/*----------- defined in audio/redalert.c -----------*/

WRITE8_HANDLER( redalert_audio_command_w );
WRITE8_HANDLER( redalert_voice_command_w );

WRITE8_HANDLER( demoneye_audio_command_w );

MACHINE_DRIVER_EXTERN( redalert_audio );
MACHINE_DRIVER_EXTERN( ww3_audio );
MACHINE_DRIVER_EXTERN( demoneye_audio );


/*----------- defined in video/redalert.c -----------*/

extern uint8_t *redalert_bitmap_videoram;
extern uint8_t *redalert_bitmap_color;
extern uint8_t *redalert_charmap_videoram;
extern uint8_t *redalert_video_control;
WRITE8_HANDLER( redalert_bitmap_videoram_w );

MACHINE_DRIVER_EXTERN( ww3_video );
MACHINE_DRIVER_EXTERN( panther_video );
MACHINE_DRIVER_EXTERN( redalert_video );
MACHINE_DRIVER_EXTERN( demoneye_video );
