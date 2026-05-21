/*************************************************************************

    Gottlieb Exterminator hardware

*************************************************************************/

/*----------- defined in video/exterm.c -----------*/

extern uint16_t *exterm_master_videoram;
extern uint16_t *exterm_slave_videoram;

PALETTE_INIT( exterm );
void exterm_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);

void exterm_to_shiftreg_master(const address_space *space, uint32_t address, uint16_t* shiftreg);
void exterm_from_shiftreg_master(const address_space *space, uint32_t address, uint16_t* shiftreg);
void exterm_to_shiftreg_slave(const address_space *space, uint32_t address, uint16_t* shiftreg);
void exterm_from_shiftreg_slave(const address_space *space, uint32_t address, uint16_t* shiftreg);
