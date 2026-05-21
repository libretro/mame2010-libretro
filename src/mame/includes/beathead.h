/*************************************************************************

    Atari "Stella on Steroids" hardware

*************************************************************************/

#include "machine/atarigen.h"

class beathead_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, beathead_state(machine)); }

	beathead_state(running_machine &machine) { }

	atarigen_state	atarigen;

	uint32_t *		vram_bulk_latch;
	uint32_t *		palette_select;

	uint32_t			finescroll;
	offs_t			vram_latch_offset;

	offs_t			hsyncram_offset;
	offs_t			hsyncram_start;
	uint8_t			hsyncram[0x800];

	uint32_t *		ram_base;
	uint32_t *		rom_base;

	double			hblank_offset;

	uint8_t			irq_line_state;
	uint8_t			irq_enable[3];
	uint8_t			irq_state[3];

	uint8_t			eeprom_enabled;
};


/*----------- defined in video/beathead.c -----------*/

VIDEO_START( beathead );
VIDEO_UPDATE( beathead );

WRITE32_HANDLER( beathead_vram_transparent_w );
WRITE32_HANDLER( beathead_vram_bulk_w );
WRITE32_HANDLER( beathead_vram_latch_w );
WRITE32_HANDLER( beathead_vram_copy_w );
WRITE32_HANDLER( beathead_finescroll_w );
WRITE32_HANDLER( beathead_palette_w );
READ32_HANDLER( beathead_hsync_ram_r );
WRITE32_HANDLER( beathead_hsync_ram_w );
