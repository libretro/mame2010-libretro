
class pgm_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, pgm_state(machine)); }

	pgm_state(running_machine &machine) { }

	/* memory pointers */
//  uint16_t *      mainram;  // currently this is also used by nvram handler
	uint16_t *      bg_videoram;
	uint16_t *      tx_videoram;
	uint16_t *      videoregs;
	uint16_t *      rowscrollram;
	uint16_t *      videoram;
	uint8_t  *      z80_mainram;
	uint32_t *      arm7_shareram;
	uint32_t *      svg_shareram[2];	//for 5585G MACHINE
	uint16_t *      sharedprotram;		// killbld & olds
	uint8_t  *      sprite_a_region;
	size_t        sprite_a_region_size;
	uint16_t *      spritebufferram; // buffered spriteram
//  uint16_t *      paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t       *bg_tilemap, *tx_tilemap;
	uint16_t        *sprite_temp_render;
	bitmap_t      *tmppgmbitmap;

	/* misc */
	// kov2
	uint32_t        kov2_latchdata_68k_w;
	uint32_t        kov2_latchdata_arm_w;
	// kovsh
	uint16_t        kovsh_highlatch_arm_w, kovsh_lowlatch_arm_w;
	uint16_t        kovsh_highlatch_68k_w, kovsh_lowlatch_68k_w;
	uint32_t        kovsh_counter;
	// svg
	int           svg_ram_sel;
	// killbld & olds
	int           kb_cmd;
	int           kb_reg;
	int           kb_ptr;
	int			  kb_region_sequence_position;
	uint32_t        kb_regs[0x10];
	uint16_t        olds_bs, olds_cmd3;
	// pstars
	uint16_t        pstars_key;
	uint16_t        pstars_int[2];
	uint32_t        pstars_regs[16];
	uint32_t        pstars_val;
	uint16_t        pstar_e7, pstar_b1, pstar_ce;
	uint16_t        pstar_ram[3];
	// ASIC 3 (oriental legends protection)
	uint8_t         asic3_reg, asic3_latch[3], asic3_x, asic3_y, asic3_z, asic3_h1, asic3_h2;
	uint16_t        asic3_hold;
	// ASIC28
	uint16_t        asic28_key;
	uint16_t        asic28_regs[10];
	uint16_t        asic_params[256];
	uint16_t        asic28_rcnt;
	uint32_t        eoregs[16];

	/* calendar */
	uint8_t        cal_val, cal_mask, cal_com, cal_cnt;
	system_time  systime;

	/* devices */
	cpu_device *soundcpu;
	cpu_device *prot;
	running_device *ics;
};

extern uint16_t *pgm_mainram;	// used by nvram handler, we cannot move it to driver data struct

/*----------- defined in machine/pgmcrypt.c -----------*/

void pgm_kov_decrypt(running_machine *machine);
void pgm_kovsh_decrypt(running_machine *machine);
void pgm_kov2_decrypt(running_machine *machine);
void pgm_kov2p_decrypt(running_machine *machine);
void pgm_mm_decrypt(running_machine *machine);
void pgm_dw2_decrypt(running_machine *machine);
void pgm_photoy2k_decrypt(running_machine *machine);
void pgm_py2k2_decrypt(running_machine *machine);
void pgm_dw3_decrypt(running_machine *machine);
void pgm_killbld_decrypt(running_machine *machine);
void pgm_pstar_decrypt(running_machine *machine);
void pgm_puzzli2_decrypt(running_machine *machine);
void pgm_theglad_decrypt(running_machine *machine);
void pgm_ddp2_decrypt(running_machine *machine);
void pgm_dfront_decrypt(running_machine *machine);
void pgm_oldsplus_decrypt(running_machine *machine);
void pgm_kovshp_decrypt(running_machine *machine);
void pgm_killbldp_decrypt(running_machine *machine);
void pgm_svg_decrypt(running_machine *machine);
void pgm_dw2001_decrypt(running_machine *machine);

/*----------- defined in machine/pgmprot.c -----------*/

READ16_HANDLER( pstars_protram_r );
READ16_HANDLER( pstars_r );
WRITE16_HANDLER( pstars_w );

READ16_HANDLER( pgm_asic3_r );
WRITE16_HANDLER( pgm_asic3_w );
WRITE16_HANDLER( pgm_asic3_reg_w );

READ16_HANDLER( sango_protram_r );
READ16_HANDLER( asic28_r );
WRITE16_HANDLER( asic28_w );

READ16_HANDLER( dw2_d80000_r );


/*----------- defined in video/pgm.c -----------*/

WRITE16_HANDLER( pgm_tx_videoram_w );
WRITE16_HANDLER( pgm_bg_videoram_w );

VIDEO_START( pgm );
VIDEO_EOF( pgm );
VIDEO_UPDATE( pgm );
