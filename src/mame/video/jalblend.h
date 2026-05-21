extern uint8_t *jal_blend_table;
rgb_t jal_blend_func(rgb_t dest, rgb_t addMe, uint8_t alpha);
void jal_blend_drawgfx(bitmap_t *dest_bmp,const rectangle *clip,const gfx_element *gfx,
							uint32_t code,uint32_t color,int flipx,int flipy,int offsx,int offsy,
							int transparent_color);
