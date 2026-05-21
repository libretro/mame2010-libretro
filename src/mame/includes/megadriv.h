#define MASTER_CLOCK_NTSC 53693175
#define MASTER_CLOCK_PAL  53203424
#define SEGACD_CLOCK      12500000


extern DRIVER_INIT( megadriv_c2 );
extern DRIVER_INIT( megadrie );
extern DRIVER_INIT( megadriv );
extern DRIVER_INIT( megadrij );
extern DRIVER_INIT( _32x );
extern DRIVER_INIT( mpnew );

INPUT_PORTS_EXTERN( md_common );
INPUT_PORTS_EXTERN( megadriv );
INPUT_PORTS_EXTERN( aladbl );
INPUT_PORTS_EXTERN( ssgbl );
INPUT_PORTS_EXTERN( megadri6 );
INPUT_PORTS_EXTERN( ssf2ghw );
INPUT_PORTS_EXTERN( mk3ghw );
INPUT_PORTS_EXTERN( megdsvp );

MACHINE_DRIVER_EXTERN( megadriv_timers );

MACHINE_DRIVER_EXTERN( megdsvppal );
MACHINE_DRIVER_EXTERN( megadriv );
MACHINE_DRIVER_EXTERN( megadpal );
MACHINE_DRIVER_EXTERN( megdsvp );
MACHINE_DRIVER_EXTERN( genesis_32x );
MACHINE_DRIVER_EXTERN( genesis_32x_pal );
MACHINE_DRIVER_EXTERN( genesis_scd );
MACHINE_DRIVER_EXTERN( genesis_32x_scd );
MACHINE_DRIVER_EXTERN( md_bootleg );	// for topshoot.c & hshavoc.c

extern uint16_t* megadriv_backupram;
extern int megadriv_backupram_length;
extern uint16_t* megadrive_ram;
extern uint8_t megatech_bios_port_cc_dc_r(running_machine *machine, int offset, int ctrl);
extern void megadriv_stop_scanline_timer(void);

void megatech_set_megadrive_z80_as_megadrive_z80(running_machine *machine, const char* tag);

extern READ16_HANDLER( megadriv_vdp_r );
extern WRITE16_HANDLER( megadriv_vdp_w );

/* These handlers are needed by megaplay.c */
extern READ16_HANDLER( megadriv_68k_io_read );
extern WRITE16_HANDLER( megadriv_68k_io_write );

/* These handlers are needed by puckpkmn.c for his memory map */
extern READ8_DEVICE_HANDLER( megadriv_68k_YM2612_read);
extern WRITE8_DEVICE_HANDLER( megadriv_68k_YM2612_write);

/* These are needed to create external input handlers (see e.g. MESS) */
/* Regs are also used by Megaplay! */
extern uint8_t (*megadrive_io_read_data_port_ptr)(running_machine *machine, int offset);
extern void (*megadrive_io_write_data_port_ptr)(running_machine *machine, int offset, uint16_t data);
extern uint8_t megadrive_io_data_regs[3];
extern uint8_t megadrive_io_ctrl_regs[3];

MACHINE_START( megadriv );
MACHINE_RESET( megadriv );
VIDEO_START( megadriv );
VIDEO_UPDATE( megadriv );
VIDEO_EOF( megadriv );


extern uint16_t* megadrive_vdp_palette_lookup;
extern uint16_t* megadrive_vdp_palette_lookup_sprite; // for C2
extern uint16_t* megadrive_vdp_palette_lookup_shadow;
extern uint16_t* megadrive_vdp_palette_lookup_highlight;

extern int segac2_bg_pal_lookup[4];
extern int segac2_sp_pal_lookup[4];

extern int genvdp_use_cram;
extern int genesis_always_irq6;
extern int genesis_other_hacks;


/* Megaplay - Megatech specific */
/* It might be possible to move the following structs in the drivers */

#define MP_ROM  0x10
#define MP_GAME 0

struct _mplay_bios	/* once all the regs are saved in this structure, it would be better to reorganize it a bit... */
{
	uint32_t bios_mode;  // determines whether ROM banks or Game data
								   // is to read from 0x8000-0xffff

	uint32_t bios_bank; // ROM bank selection
	uint16_t game_banksel;  // Game bank selection
	uint32_t readpos;  // serial bank selection position (9-bit)
	uint32_t mp_bios_bank_addr;

	uint32_t bios_width;  // determines the way the game info ROM is read
	uint8_t bios_ctrl[6];
	uint8_t bios_6600;
	uint8_t bios_6403;
	uint8_t bios_6404;
};

extern struct _mplay_bios mplay_bios;	// defined in megaplay.c

struct _mtech_bios	/* once all the regs are saved in this structure, it would be better to reorganize it a bit... */
{
	uint8_t mt_cart_select_reg;
	uint32_t bios_port_ctrl;
	int current_game_is_sms; // is the current game SMS based (running on genesis z80, in VDP compatibility mode)
	uint32_t bios_ctrl_inputs;
	uint8_t bios_ctrl[6];

	int mt_bank_bank_pos;
	int mt_bank_partial;
	int mt_bank_addr;
};
