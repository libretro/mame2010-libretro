/*************************************************************************

    Williams/Midway Y/Z-unit system

**************************************************************************/

#include "cpu/tms34010/tms34010.h"

/*----------- defined in machine/midyunit.c -----------*/

extern uint16_t *midyunit_cmos_ram;
extern uint32_t	midyunit_cmos_page;

WRITE16_HANDLER( midyunit_cmos_w );
READ16_HANDLER( midyunit_cmos_r );

WRITE16_HANDLER( midyunit_cmos_enable_w );
READ16_HANDLER( midyunit_protection_r );

READ16_HANDLER( midyunit_input_r );

DRIVER_INIT( narc );
DRIVER_INIT( trog );
DRIVER_INIT( smashtv );
DRIVER_INIT( hiimpact );
DRIVER_INIT( shimpact );
DRIVER_INIT( strkforc );
DRIVER_INIT( mkyunit );
DRIVER_INIT( mkyturbo );
DRIVER_INIT( mkyawdim );
DRIVER_INIT( term2 );
DRIVER_INIT( term2la3 );
DRIVER_INIT( term2la2 );
DRIVER_INIT( term2la1 );
DRIVER_INIT( totcarn );

MACHINE_RESET( midyunit );

WRITE16_HANDLER( midyunit_sound_w );


/*----------- defined in video/midyunit.c -----------*/

extern uint8_t *	midyunit_gfx_rom;
extern size_t	midyunit_gfx_rom_size;

VIDEO_START( midyunit_4bit );
VIDEO_START( midyunit_6bit );
VIDEO_START( mkyawdim );
VIDEO_START( midzunit );

READ16_HANDLER( midyunit_gfxrom_r );

WRITE16_HANDLER( midyunit_vram_w );
READ16_HANDLER( midyunit_vram_r );

void midyunit_to_shiftreg(const address_space *space, uint32_t address, uint16_t *shiftreg);
void midyunit_from_shiftreg(const address_space *space, uint32_t address, uint16_t *shiftreg);

WRITE16_HANDLER( midyunit_control_w );
WRITE16_HANDLER( midyunit_paletteram_w );

READ16_HANDLER( midyunit_dma_r );
WRITE16_HANDLER( midyunit_dma_w );

void midyunit_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);
