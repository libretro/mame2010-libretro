/*************************************************************************

    Atari Liberator hardware

*************************************************************************/

/*----------- defined in video/liberatr.c -----------*/

extern uint8_t *liberatr_base_ram;
extern uint8_t *liberatr_planet_frame;
extern uint8_t *liberatr_planet_select;
extern uint8_t *liberatr_x;
extern uint8_t *liberatr_y;
extern uint8_t *liberatr_bitmapram;
extern uint8_t *liberatr_colorram;

VIDEO_START( liberatr );
VIDEO_UPDATE( liberatr );

WRITE8_HANDLER( liberatr_bitmap_w );
READ8_HANDLER( liberatr_bitmap_xy_r );
WRITE8_HANDLER( liberatr_bitmap_xy_w );
