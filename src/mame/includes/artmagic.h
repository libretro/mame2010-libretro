/*************************************************************************

    Art & Magic hardware

**************************************************************************/

/*----------- defined in video/artmagic.c -----------*/

extern uint16_t *artmagic_vram0;
extern uint16_t *artmagic_vram1;

extern int artmagic_xor[16], artmagic_is_stoneball;

VIDEO_START( artmagic );

void artmagic_to_shiftreg(const address_space *space, offs_t address, uint16_t *data);
void artmagic_from_shiftreg(const address_space *space, offs_t address, uint16_t *data);

READ16_HANDLER( artmagic_blitter_r );
WRITE16_HANDLER( artmagic_blitter_w );

void artmagic_scanline(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);
