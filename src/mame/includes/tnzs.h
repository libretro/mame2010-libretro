
#define	MAX_SAMPLES	0x2f		/* max samples */

enum
{
	MCU_NONE_INSECTX = 0,
	MCU_NONE_KAGEKI,
	MCU_NONE_TNZSB,
	MCU_NONE_KABUKIZ,
	MCU_EXTRMATN,
	MCU_ARKANOID,
	MCU_PLUMPOP,
	MCU_DRTOPPEL,
	MCU_CHUKATAI,
	MCU_TNZS
};

class tnzs_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, tnzs_state(machine)); }

	tnzs_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *  objram;
	uint8_t *  vdcram;
	uint8_t *  scrollram;
	uint8_t *  objctrl;
	uint8_t *  bg_flag;
//  uint8_t *  paletteram;    // currently this uses generic palette handling

	/* video-related */
	int      screenflip;

	/* sound-related */
	int16_t    *sampledata[MAX_SAMPLES];
	int      samplesize[MAX_SAMPLES];

	/* misc / mcu */
	int      kageki_csport_sel;
	int      input_select;
	int      mcu_type;
	int      mcu_initializing, mcu_coinage_init, mcu_command, mcu_readcredits;
	int      mcu_reportcoin;
	int      insertcoin;
	uint8_t    mcu_coinage[4];
	uint8_t    mcu_coins_a, mcu_coins_b, mcu_credits;
	int      bank1;
	int      bank2;

	/* game-specific */
	// champbwl
	uint8_t    last_trackball_val[2];
//  uint8_t *  nvram; // currently this uses generic_nvram
	// cchance
	uint8_t    hop_io, bell_io;


	/* devices */
	running_device *audiocpu;
	running_device *subcpu;
	running_device *mcu;
};


/*----------- defined in machine/tnzs.c -----------*/

READ8_HANDLER( tnzs_port1_r );
READ8_HANDLER( tnzs_port2_r );
WRITE8_HANDLER( tnzs_port2_w );
READ8_HANDLER( arknoid2_sh_f000_r );
READ8_HANDLER( tnzs_mcu_r );
WRITE8_HANDLER( tnzs_mcu_w );
WRITE8_HANDLER( tnzs_bankswitch_w );
WRITE8_HANDLER( tnzs_bankswitch1_w );
INTERRUPT_GEN( arknoid2_interrupt );

DRIVER_INIT( plumpop );
DRIVER_INIT( extrmatn );
DRIVER_INIT( arknoid2 );
DRIVER_INIT( drtoppel );
DRIVER_INIT( chukatai );
DRIVER_INIT( tnzs );
DRIVER_INIT( tnzsb );
DRIVER_INIT( kabukiz );
DRIVER_INIT( insectx );
DRIVER_INIT( kageki );

MACHINE_START( tnzs );
MACHINE_RESET( tnzs );
MACHINE_RESET( jpopnics );
MACHINE_START( jpopnics );


/*----------- defined in video/tnzs.c -----------*/

PALETTE_INIT( arknoid2 );
VIDEO_UPDATE( tnzs );
VIDEO_EOF( tnzs );
