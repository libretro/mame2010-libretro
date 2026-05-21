/*************************************************************************

    Irem M107 hardware

*************************************************************************/


/*----------- defined in video/m107.c -----------*/

extern uint16_t *m107_vram_data;
extern uint8_t m107_spritesystem;
extern uint16_t m107_raster_irq_position;

WRITE16_HANDLER( m107_spritebuffer_w );
VIDEO_UPDATE( m107 );
VIDEO_START( m107 );
WRITE16_HANDLER( m107_control_w );
WRITE16_HANDLER( m107_vram_w );
