/***************************************************************************

    JPM IMPACT with Video hardware

****************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "includes/jpmimpct.h"


uint16_t *jpmimpct_vram;


/*************************************
 *
 *  Brooktree Bt477 RAMDAC
 *
 *************************************/

static struct
{
	uint8_t address;
	uint8_t addr_cnt;
	uint8_t pixmask;
	uint8_t command;
	rgb_t color;
} bt477;


/*
 *  0 0 0    Address register (RAM write mode)
 *  0 0 1    Color palette RAMs
 *  0 1 0    Pixel read mask register
 *  0 1 1    Address register (RAM read mode)
 *  1 0 0    Address register (overlay write mode)
 *  1 1 1    Address register (overlay read mode)
 *  1 0 1    Overlay register
 *  1 1 0    Command register
 */

WRITE16_HANDLER( jpmimpct_bt477_w )
{
	uint8_t val = data & 0xff;

	switch (offset)
	{
		case 0x0:
		{
			bt477.address = val;
			bt477.addr_cnt = 0;
			break;
		}
		case 0x1:
		{
			uint8_t *addr_cnt = &bt477.addr_cnt;
			rgb_t *color = &bt477.color;

			color[*addr_cnt] = val;

			if (++*addr_cnt == 3)
			{
				palette_set_color(space->machine, bt477.address, MAKE_RGB(color[0], color[1], color[2]));
				*addr_cnt = 0;

				/* Address register increments */
				bt477.address++;
			}
			break;
		}
		case 0x2:
		{
			bt477.pixmask = val;
			break;
		}
		case 0x6:
		{
			bt477.command = val;
			break;
		}
		default:
		{
			popmessage("Bt477: Unhandled write access (offset:%x, data:%x)", offset, val);
		}
	}
}

READ16_HANDLER( jpmimpct_bt477_r )
{
	popmessage("Bt477: Unhandled read access (offset:%x)", offset);
	return 0;
}


/*************************************
 *
 *  VRAM shift register callbacks
 *
 *************************************/

void jpmimpct_to_shiftreg(const address_space *space, uint32_t address, uint16_t *shiftreg)
{
	memcpy(shiftreg, &jpmimpct_vram[TOWORD(address)], 512 * sizeof(uint16_t));
}

void jpmimpct_from_shiftreg(const address_space *space, uint32_t address, uint16_t *shiftreg)
{
	memcpy(&jpmimpct_vram[TOWORD(address)], shiftreg, 512 * sizeof(uint16_t));
}


/*************************************
 *
 *  Main video refresh
 *
 *************************************/

void jpmimpct_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params)
{
	uint16_t *vram = &jpmimpct_vram[(params->rowaddr << 8) & 0x3ff00];
	uint32_t *dest = BITMAP_ADDR32(bitmap, scanline, 0);
	int coladdr = params->coladdr;
	int x;

	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		uint16_t pixels = vram[coladdr++ & 0xff];
		dest[x + 0]	= screen.machine->pens[pixels & 0xff];
		dest[x + 1] = screen.machine->pens[pixels >> 8];
	}
}


/*************************************
 *
 *  Video emulation start
 *
 *************************************/

VIDEO_START( jpmimpct )
{
	memset(&bt477, 0, sizeof(bt477));

	state_save_register_global(machine, bt477.address);
	state_save_register_global(machine, bt477.addr_cnt);
	state_save_register_global(machine, bt477.pixmask);
	state_save_register_global(machine, bt477.command);
	state_save_register_global(machine, bt477.color);
}
