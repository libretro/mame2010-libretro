/*----------- defined in video/tubep.c -----------*/

void tubep_vblank_end(void);
PALETTE_INIT( tubep );
VIDEO_UPDATE( tubep );
PALETTE_INIT( rjammer );
VIDEO_UPDATE( rjammer );
VIDEO_START( tubep );
VIDEO_RESET( tubep );

extern uint8_t *rjammer_backgroundram;
extern uint8_t *tubep_backgroundram;
extern uint8_t *tubep_textram;
extern uint8_t *tubep_sprite_colorsharedram;

extern WRITE8_HANDLER( tubep_textram_w );
extern WRITE8_HANDLER( rjammer_background_LS377_w );
extern WRITE8_HANDLER( rjammer_background_page_w );

extern WRITE8_HANDLER( tubep_colorproms_A4_line_w );
extern WRITE8_HANDLER( tubep_background_romselect_w );
extern WRITE8_HANDLER( tubep_background_a000_w );
extern WRITE8_HANDLER( tubep_background_c000_w );

extern WRITE8_HANDLER( tubep_sprite_control_w );

