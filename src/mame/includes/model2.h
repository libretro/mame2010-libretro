/*----------- defined in drivers/model2.c -----------*/

extern uint32_t geo_read_start_address;
extern uint32_t geo_write_start_address;
extern uint32_t *model2_bufferram;
extern uint32_t *model2_colorxlat;
extern uint32_t *model2_textureram0;
extern uint32_t *model2_textureram1;
extern uint32_t *model2_lumaram;
extern uint32_t *model2_paletteram32;


/*----------- defined in video/model2.c -----------*/

VIDEO_START(model2);
VIDEO_UPDATE(model2);

void model2_3d_set_zclip( uint8_t clip );
