

class changela_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, changela_state(machine)); }

	changela_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  videoram;
	uint8_t *  colorram;
	uint8_t *  spriteram;

	/* video-related */
	bitmap_t *obj0_bitmap, *river_bitmap, *tree0_bitmap, *tree1_bitmap;
	uint8_t*   tree_ram;
	uint8_t*   memory_devices;
	uint32_t   mem_dev_selected;	/* an offset within memory_devices area */
	uint32_t   slopeROM_bank;
	uint8_t    tree_en;
	uint8_t    horizon;
	uint8_t    v_count_river;
	uint8_t    v_count_tree;
	int      tree_on[2];
	emu_timer* scanline_timer;

	/* misc */
	uint8_t    tree0_col;
	uint8_t    tree1_col;
	uint8_t    left_bank_col;
	uint8_t    right_bank_col;
	uint8_t    boat_shore_col;
	uint8_t    collision_reset;
	uint8_t    tree_collision_reset;
	uint8_t    prev_value_31;
	int      dir_31;

	/* mcu-related */
	uint8_t    port_a_in, port_a_out, ddr_a;
	uint8_t    port_b_out, ddr_b;
	uint8_t    port_c_in, port_c_out, ddr_c;

	uint8_t    mcu_out;
	uint8_t    mcu_in;
	uint8_t    mcu_pc_1;
	uint8_t    mcu_pc_0;

	/* devices */
	running_device *mcu;
};

/*----------- defined in video/changela.c -----------*/

VIDEO_START( changela );
VIDEO_UPDATE( changela );

WRITE8_HANDLER( changela_colors_w );
WRITE8_HANDLER( changela_mem_device_select_w );
WRITE8_HANDLER( changela_mem_device_w );
READ8_HANDLER( changela_mem_device_r );
WRITE8_HANDLER( changela_slope_rom_addr_hi_w );
WRITE8_HANDLER( changela_slope_rom_addr_lo_w );
