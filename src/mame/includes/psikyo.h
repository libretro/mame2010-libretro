/*************************************************************************

    Psikyo Games

*************************************************************************/

class psikyo_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, psikyo_state(machine)); }

	psikyo_state(running_machine &machine) { }

	/* memory pointers */
	uint32_t *       vram_0;
	uint32_t *       vram_1;
	uint32_t *       vregs;
	uint32_t *       spritebuf1;
	uint32_t *       spritebuf2;
	uint32_t *       bootleg_spritebuffer;
//      uint32_t *       paletteram;  // currently this uses generic palette handling
//  uint32_t *       spriteram;   // currently this uses generic buffered spriteram
//  size_t         spriteram_size;

	/* video-related */
	tilemap_t        *tilemap_0_size0, *tilemap_0_size1, *tilemap_0_size2, *tilemap_0_size3;
	tilemap_t        *tilemap_1_size0, *tilemap_1_size1, *tilemap_1_size2, *tilemap_1_size3;
	int            tilemap_0_bank, tilemap_1_bank;
	int            ka302c_banking;

	/* misc */
	uint8_t          soundlatch;
	int            z80_nmi, mcu_status;

	/* devices */
	running_device *audiocpu;

	/* game-specific */
	// 1945 MCU
	uint8_t          s1945_mcu_direction, s1945_mcu_latch1, s1945_mcu_latch2, s1945_mcu_inlatch, s1945_mcu_index;
	uint8_t          s1945_mcu_latching, s1945_mcu_mode, s1945_mcu_control, s1945_mcu_bctrl;
	const uint8_t    *s1945_mcu_table;
};


/*----------- defined in video/psikyo.c -----------*/

void psikyo_switch_banks(running_machine *machine, int tmap, int bank);

WRITE32_HANDLER( psikyo_vram_0_w );
WRITE32_HANDLER( psikyo_vram_1_w );

VIDEO_START( sngkace );
VIDEO_START( psikyo );
VIDEO_UPDATE( psikyo );
VIDEO_UPDATE( psikyo_bootleg );
VIDEO_EOF( psikyo );
