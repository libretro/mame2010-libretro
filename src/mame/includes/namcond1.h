/***************************************************************************

  namcond1.h

  Common functions & declarations for the Namco ND-1 driver

***************************************************************************/

#define GFX_8X8_4BIT    0
#define GFX_16X16_4BIT  1
#define GFX_32X32_4BIT  2
#define GFX_64X64_4BIT  3
#define GFX_8X8_8BIT    4
#define GFX_16X16_8BIT  5

/*----------- defined in machine/namcond1.c -----------*/

extern uint8_t namcond1_gfxbank;

extern uint8_t namcond1_h8_irq5_enabled;
extern uint16_t *namcond1_shared_ram;

extern READ16_HANDLER( namcond1_shared_ram_r );
extern READ16_HANDLER( namcond1_cuskey_r );
extern WRITE16_HANDLER( namcond1_shared_ram_w );
extern WRITE16_HANDLER( namcond1_cuskey_w );

MACHINE_START( namcond1 );
MACHINE_RESET( namcond1 );

