/*************************************************************************

    Atari GT hardware

*************************************************************************/

#include "machine/atarigen.h"


#define CRAM_ENTRIES		0x4000
#define TRAM_ENTRIES		0x4000
#define MRAM_ENTRIES		0x8000

class atarigt_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, atarigt_state(machine)); }

	atarigt_state(running_machine &machine) { }

	atarigen_state	atarigen;
	uint8_t			is_primrage;
	uint16_t *		colorram;

	bitmap_t *		pf_bitmap;
	bitmap_t *		an_bitmap;

	uint8_t			playfield_tile_bank;
	uint8_t			playfield_color_bank;
	uint16_t			playfield_xscroll;
	uint16_t			playfield_yscroll;

	uint32_t			tram_checksum;

	uint32_t			expanded_mram[MRAM_ENTRIES * 3];

	uint32_t *		mo_command;

	void			(*protection_w)(const address_space *space, offs_t offset, uint16_t data);
	void			(*protection_r)(const address_space *space, offs_t offset, uint16_t *data);
};


/*----------- defined in video/atarigt.c -----------*/

void atarigt_colorram_w(atarigt_state *state, offs_t address, uint16_t data, uint16_t mem_mask);
uint16_t atarigt_colorram_r(atarigt_state *state, offs_t address);

VIDEO_START( atarigt );
VIDEO_UPDATE( atarigt );

void atarigt_scanline_update(screen_device &screen, int scanline);
