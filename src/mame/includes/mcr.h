/*************************************************************************

    Driver for Midway MCR games

**************************************************************************/

#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "machine/6821pia.h"

/* constants */
#define MAIN_OSC_MCR_I		XTAL_19_968MHz


/*----------- defined in drivers/mcr.c -----------*/

WRITE8_DEVICE_HANDLER( mcr_ipu_sio_transmit );



/*----------- defined in machine/mcr.c -----------*/

extern attotime mcr68_timing_factor;

extern const z80_daisy_config mcr_daisy_chain[];
extern const z80_daisy_config mcr_ipu_daisy_chain[];
extern const z80ctc_interface mcr_ctc_intf;
extern const z80ctc_interface nflfoot_ctc_intf;
extern const z80pio_interface nflfoot_pio_intf;
extern const z80sio_interface nflfoot_sio_intf;
extern uint8_t mcr_cocktail_flip;

extern const gfx_layout mcr_bg_layout;
extern const gfx_layout mcr_sprite_layout;

extern uint32_t mcr_cpu_board;
extern uint32_t mcr_sprite_board;
extern uint32_t mcr_ssio_board;

extern const pia6821_interface zwackery_pia0_intf;
extern const pia6821_interface zwackery_pia1_intf;
extern const pia6821_interface zwackery_pia2_intf;

MACHINE_START( mcr );
MACHINE_RESET( mcr );
MACHINE_START( nflfoot );
MACHINE_START( mcr68 );
MACHINE_RESET( mcr68 );
MACHINE_START( zwackery );
MACHINE_RESET( zwackery );

INTERRUPT_GEN( mcr_interrupt );
INTERRUPT_GEN( mcr_ipu_interrupt );
INTERRUPT_GEN( mcr68_interrupt );

WRITE8_HANDLER( mcr_control_port_w );
WRITE8_HANDLER( mcrmono_control_port_w );
WRITE8_HANDLER( mcr_scroll_value_w );

WRITE16_HANDLER( mcr68_6840_upper_w );
WRITE16_HANDLER( mcr68_6840_lower_w );
READ16_HANDLER( mcr68_6840_upper_r );
READ16_HANDLER( mcr68_6840_lower_r );

WRITE8_HANDLER( mcr_ipu_laserdisk_w );
READ8_HANDLER( mcr_ipu_watchdog_r );
WRITE8_HANDLER( mcr_ipu_watchdog_w );


/*----------- defined in video/mcr.c -----------*/

extern int8_t mcr12_sprite_xoffs;
extern int8_t mcr12_sprite_xoffs_flip;

VIDEO_START( mcr );

WRITE8_HANDLER( mcr_91490_paletteram_w );

WRITE8_HANDLER( mcr_90009_videoram_w );
WRITE8_HANDLER( mcr_90010_videoram_w );
READ8_HANDLER( twotiger_videoram_r );
WRITE8_HANDLER( twotiger_videoram_w );
WRITE8_HANDLER( mcr_91490_videoram_w );

VIDEO_UPDATE( mcr );


/*----------- defined in video/mcr3.c -----------*/

extern uint8_t spyhunt_sprite_color_mask;
extern int16_t spyhunt_scrollx, spyhunt_scrolly;
extern int16_t spyhunt_scroll_offset;

extern uint8_t *spyhunt_alpharam;

WRITE8_HANDLER( mcr3_paletteram_w );
WRITE8_HANDLER( mcr3_videoram_w );
WRITE8_HANDLER( spyhunt_videoram_w );
WRITE8_HANDLER( spyhunt_alpharam_w );

VIDEO_START( mcrmono );
VIDEO_START( spyhunt );

PALETTE_INIT( spyhunt );

VIDEO_UPDATE( mcr3 );
VIDEO_UPDATE( spyhunt );


/*----------- defined in video/mcr68.c -----------*/

extern uint8_t mcr68_sprite_clip;
extern int8_t mcr68_sprite_xoffset;

WRITE16_HANDLER( mcr68_paletteram_w );
WRITE16_HANDLER( mcr68_videoram_w );

VIDEO_START( mcr68 );
VIDEO_UPDATE( mcr68 );

WRITE16_HANDLER( zwackery_paletteram_w );
WRITE16_HANDLER( zwackery_videoram_w );
WRITE16_HANDLER( zwackery_spriteram_w );

VIDEO_START( zwackery );
VIDEO_UPDATE( zwackery );
