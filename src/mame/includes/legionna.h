/*----------- defined in video/legionna.c -----------*/

extern uint16_t *legionna_back_data,*legionna_fore_data,*legionna_mid_data,*legionna_scrollram16,*legionna_textram;
extern uint8_t grainbow_pri_n;
extern uint16_t legionna_layer_disable;

void heatbrl_setgfxbank(uint16_t data);
void denjinmk_setgfxbank(uint16_t data);
WRITE16_HANDLER( legionna_background_w );
WRITE16_HANDLER( legionna_foreground_w );
WRITE16_HANDLER( legionna_midground_w );
WRITE16_HANDLER( legionna_text_w );

VIDEO_START( legionna );
VIDEO_START( cupsoc );
VIDEO_START( denjinmk );
VIDEO_UPDATE( legionna );
VIDEO_UPDATE( godzilla );
VIDEO_UPDATE( grainbow );
