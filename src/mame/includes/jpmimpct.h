/***************************************************************************

    JPM IMPACT with Video hardware

****************************************************************************/

/*----------- defined in video/jpmimpct.c -----------*/

extern uint16_t *jpmimpct_vram;

READ16_HANDLER( jpmimpct_bt477_r );
WRITE16_HANDLER( jpmimpct_bt477_w );

void jpmimpct_to_shiftreg(const address_space *space, uint32_t address, uint16_t *shiftreg);
void jpmimpct_from_shiftreg(const address_space *space, uint32_t address, uint16_t *shiftreg);
void jpmimpct_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);

VIDEO_START( jpmimpct );
