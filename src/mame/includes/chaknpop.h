

#define MCU_INITIAL_SEED	0x81


class chaknpop_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, chaknpop_state(machine)); }

	chaknpop_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  mcu_ram;
	uint8_t *  tx_ram;
	uint8_t *  spr_ram;
	uint8_t *  attr_ram;
	size_t   spr_ram_size;

	/* mcu-related */
	uint8_t mcu_seed;
	uint8_t mcu_select;
	uint8_t mcu_result;


	/* video-related */
	tilemap_t  *tx_tilemap;
	uint8_t    *vram1, *vram2, *vram3, *vram4;
	uint8_t    gfxmode;
	uint8_t    flip_x, flip_y;
};



/*----------- defined in machine/chaknpop.c -----------*/

READ8_HANDLER( chaknpop_mcu_port_a_r );
READ8_HANDLER( chaknpop_mcu_port_b_r );
READ8_HANDLER( chaknpop_mcu_port_c_r );
WRITE8_HANDLER( chaknpop_mcu_port_a_w );
WRITE8_HANDLER( chaknpop_mcu_port_b_w );
WRITE8_HANDLER( chaknpop_mcu_port_c_w );


/*----------- defined in video/chaknpop.c -----------*/

PALETTE_INIT( chaknpop );
VIDEO_START( chaknpop );
VIDEO_UPDATE( chaknpop );

READ8_HANDLER( chaknpop_gfxmode_r );
WRITE8_HANDLER( chaknpop_gfxmode_w );
WRITE8_HANDLER( chaknpop_txram_w );
WRITE8_HANDLER( chaknpop_attrram_w );
