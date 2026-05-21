/*************************************************************************

    Driver for Atari polygon racer games

**************************************************************************/

#include "cpu/tms34010/tms34010.h"
#include "machine/atarigen.h"

class harddriv_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, harddriv_state(machine)); }

	harddriv_state(running_machine &machine)
		: maincpu(machine.device<cpu_device>("maincpu")),
		  gsp(machine.device<cpu_device>("gsp")),
		  msp(machine.device<cpu_device>("msp")),
		  adsp(machine.device<cpu_device>("adsp")),
		  soundcpu(machine.device<cpu_device>("soundcpu")),
		  sounddsp(machine.device<cpu_device>("sounddsp")),
		  jsacpu(machine.device<cpu_device>("jsa")),
		  dsp32(machine.device<cpu_device>("dsp32")),
		  duart_timer(machine.device<timer_device>("duart_timer")) { }

	atarigen_state			atarigen;

	cpu_device *			maincpu;
	cpu_device *			gsp;
	cpu_device *			msp;
	cpu_device *			adsp;
	cpu_device *			soundcpu;
	cpu_device *			sounddsp;
	cpu_device *			jsacpu;
	cpu_device *			dsp32;

	uint8_t					hd34010_host_access;
	uint8_t					dsk_pio_access;

	uint16_t *				msp_ram;
	uint16_t *				dsk_ram;
	uint16_t *				dsk_rom;
	uint16_t *				dsk_zram;
	uint16_t *				m68k_slapstic_base;
	uint16_t *				m68k_sloop_alt_base;

	uint16_t *				adsp_data_memory;
	uint32_t *				adsp_pgm_memory;

	uint16_t *				gsp_protection;
	uint16_t *				stmsp_sync[3];

	uint16_t *				gsp_speedup_addr[2];
	offs_t					gsp_speedup_pc;

	uint16_t *				msp_speedup_addr;
	offs_t					msp_speedup_pc;

	uint16_t *				ds3_speedup_addr;
	offs_t					ds3_speedup_pc;
	offs_t					ds3_transfer_pc;

	uint32_t *				rddsp32_sync[2];

	uint32_t					gsp_speedup_count[4];
	uint32_t					msp_speedup_count[4];
	uint32_t					adsp_speedup_count[4];

	uint16_t *				sounddsp_ram;

	uint8_t					gsp_multisync;
	uint8_t *					gsp_vram;
	uint16_t *				gsp_control_lo;
	uint16_t *				gsp_control_hi;
	uint16_t *				gsp_paletteram_lo;
	uint16_t *				gsp_paletteram_hi;
	size_t					gsp_vram_size;

	/* driver state */
	uint32_t *				rddsp32_speedup;
	offs_t					rddsp32_speedup_pc;

	/* machine state */
	uint8_t					irq_state;
	uint8_t					gsp_irq_state;
	uint8_t					msp_irq_state;
	uint8_t					adsp_irq_state;
	uint8_t					duart_irq_state;

	uint8_t					duart_read_data[16];
	uint8_t					duart_write_data[16];
	uint8_t					duart_output_port;
	timer_device *			duart_timer;

	uint8_t					last_gsp_shiftreg;

	uint8_t					m68k_zp1;
	uint8_t					m68k_zp2;
	uint8_t					m68k_adsp_buffer_bank;

	uint8_t					adsp_halt;
	uint8_t					adsp_br;
	uint8_t					adsp_xflag;
	uint16_t					adsp_sim_address;
	uint16_t					adsp_som_address;
	uint32_t					adsp_eprom_base;

	uint16_t *				sim_memory;
	uint32_t					sim_memory_size;
	uint16_t					som_memory[0x8000/2];
	uint16_t *				adsp_pgm_memory_word;

	uint8_t					ds3_gcmd;
	uint8_t					ds3_gflag;
	uint8_t					ds3_g68irqs;
	uint8_t					ds3_gfirqs;
	uint8_t					ds3_g68flag;
	uint8_t					ds3_send;
	uint8_t					ds3_reset;
	uint16_t					ds3_gdata;
	uint16_t					ds3_g68data;
	uint32_t					ds3_sim_address;

	uint16_t					adc_control;
	uint8_t					adc8_select;
	uint8_t					adc8_data;
	uint8_t					adc12_select;
	uint8_t					adc12_byte;
	uint16_t					adc12_data;

	uint16_t					hdc68k_last_wheel;
	uint16_t					hdc68k_last_port1;
	uint8_t					hdc68k_wheel_edge;
	uint8_t					hdc68k_shifter_state;

	uint8_t					st68k_sloop_bank;
	offs_t					st68k_last_alt_sloop_offset;

	#define MAX_MSP_SYNC	16
	uint32_t *				dataptr[MAX_MSP_SYNC];
	uint32_t					dataval[MAX_MSP_SYNC];
	int 					next_msp_sync;

	/* audio state */
	uint8_t					soundflag;
	uint8_t					mainflag;
	uint16_t					sounddata;
	uint16_t					maindata;

	uint8_t					dacmute;
	uint8_t					cramen;
	uint8_t					irq68k;

	offs_t					sound_rom_offs;

	uint8_t *					rombase;
	uint32_t					romsize;
	uint16_t					comram[0x400/2];
	uint64_t					last_bio_cycles;

	/* video state */
	offs_t					vram_mask;

	uint8_t					shiftreg_enable;

	uint32_t					mask_table[65536 * 4];
	uint8_t *					gsp_shiftreg_source;

	int8_t					gfx_finescroll;
	uint8_t					gfx_palettebank;
};


/*----------- defined in machine/harddriv.c -----------*/

/* Driver/Multisync board */
MACHINE_START( harddriv );
MACHINE_RESET( harddriv );

INTERRUPT_GEN( hd68k_irq_gen );
WRITE16_HANDLER( hd68k_irq_ack_w );
void hdgsp_irq_gen(running_device *device, int state);
void hdmsp_irq_gen(running_device *device, int state);

READ16_HANDLER( hd68k_gsp_io_r );
WRITE16_HANDLER( hd68k_gsp_io_w );

READ16_HANDLER( hd68k_msp_io_r );
WRITE16_HANDLER( hd68k_msp_io_w );

READ16_HANDLER( hd68k_port0_r );
READ16_HANDLER( hd68k_adc8_r );
READ16_HANDLER( hd68k_adc12_r );
READ16_HANDLER( hdc68k_port1_r );
READ16_HANDLER( hda68k_port1_r );
READ16_HANDLER( hdc68k_wheel_r );
READ16_HANDLER( hd68k_sound_reset_r );

WRITE16_HANDLER( hd68k_adc_control_w );
WRITE16_HANDLER( hd68k_wr0_write );
WRITE16_HANDLER( hd68k_wr1_write );
WRITE16_HANDLER( hd68k_wr2_write );
WRITE16_HANDLER( hd68k_nwr_w );
WRITE16_HANDLER( hdc68k_wheel_edge_reset_w );

READ16_HANDLER( hd68k_zram_r );
WRITE16_HANDLER( hd68k_zram_w );

TIMER_DEVICE_CALLBACK( hd68k_duart_callback );
READ16_HANDLER( hd68k_duart_r );
WRITE16_HANDLER( hd68k_duart_w );

WRITE16_HANDLER( hdgsp_io_w );

WRITE16_HANDLER( hdgsp_protection_w );

WRITE16_HANDLER( stmsp_sync0_w );
WRITE16_HANDLER( stmsp_sync1_w );
WRITE16_HANDLER( stmsp_sync2_w );

/* ADSP board */
READ16_HANDLER( hd68k_adsp_program_r );
WRITE16_HANDLER( hd68k_adsp_program_w );

READ16_HANDLER( hd68k_adsp_data_r );
WRITE16_HANDLER( hd68k_adsp_data_w );

READ16_HANDLER( hd68k_adsp_buffer_r );
WRITE16_HANDLER( hd68k_adsp_buffer_w );

WRITE16_HANDLER( hd68k_adsp_control_w );
WRITE16_HANDLER( hd68k_adsp_irq_clear_w );
READ16_HANDLER( hd68k_adsp_irq_state_r );

READ16_HANDLER( hdadsp_special_r );
WRITE16_HANDLER( hdadsp_special_w );

/* DS III board */
WRITE16_HANDLER( hd68k_ds3_control_w );
READ16_HANDLER( hd68k_ds3_girq_state_r );
READ16_HANDLER( hd68k_ds3_sirq_state_r );
READ16_HANDLER( hd68k_ds3_gdata_r );
WRITE16_HANDLER( hd68k_ds3_gdata_w );
READ16_HANDLER( hd68k_ds3_sdata_r );
WRITE16_HANDLER( hd68k_ds3_sdata_w );

READ16_HANDLER( hdds3_special_r );
WRITE16_HANDLER( hdds3_special_w );
READ16_HANDLER( hdds3_control_r );
WRITE16_HANDLER( hdds3_control_w );

READ16_HANDLER( hd68k_ds3_program_r );
WRITE16_HANDLER( hd68k_ds3_program_w );

/* DSK board */
void hddsk_update_pif(running_device *device, uint32_t pins);
WRITE16_HANDLER( hd68k_dsk_control_w );
READ16_HANDLER( hd68k_dsk_ram_r );
WRITE16_HANDLER( hd68k_dsk_ram_w );
READ16_HANDLER( hd68k_dsk_zram_r );
WRITE16_HANDLER( hd68k_dsk_zram_w );
READ16_HANDLER( hd68k_dsk_small_rom_r );
READ16_HANDLER( hd68k_dsk_rom_r );
WRITE16_HANDLER( hd68k_dsk_dsp32_w );
READ16_HANDLER( hd68k_dsk_dsp32_r );
WRITE32_HANDLER( rddsp32_sync0_w );
WRITE32_HANDLER( rddsp32_sync1_w );

/* DSPCOM board */
WRITE16_HANDLER( hddspcom_control_w );

WRITE16_HANDLER( rd68k_slapstic_w );
READ16_HANDLER( rd68k_slapstic_r );

/* Game-specific protection */
WRITE16_HANDLER( st68k_sloop_w );
READ16_HANDLER( st68k_sloop_r );
READ16_HANDLER( st68k_sloop_alt_r );
WRITE16_HANDLER( st68k_protosloop_w );
READ16_HANDLER( st68k_protosloop_r );

/* GSP optimizations */
READ16_HANDLER( hdgsp_speedup_r );
WRITE16_HANDLER( hdgsp_speedup1_w );
WRITE16_HANDLER( hdgsp_speedup2_w );
READ16_HANDLER( rdgsp_speedup1_r );
WRITE16_HANDLER( rdgsp_speedup1_w );

/* MSP optimizations */
READ16_HANDLER( hdmsp_speedup_r );
WRITE16_HANDLER( hdmsp_speedup_w );
READ16_HANDLER( stmsp_speedup_r );

/* ADSP optimizations */
READ16_HANDLER( hdadsp_speedup_r );
READ16_HANDLER( hdds3_speedup_r );


/*----------- defined in audio/harddriv.c -----------*/

void hdsnd_init(running_machine *machine);

READ16_HANDLER( hd68k_snd_data_r );
READ16_HANDLER( hd68k_snd_status_r );
WRITE16_HANDLER( hd68k_snd_data_w );
WRITE16_HANDLER( hd68k_snd_reset_w );

READ16_HANDLER( hdsnd68k_data_r );
WRITE16_HANDLER( hdsnd68k_data_w );

READ16_HANDLER( hdsnd68k_switches_r );
READ16_HANDLER( hdsnd68k_320port_r );
READ16_HANDLER( hdsnd68k_status_r );

WRITE16_HANDLER( hdsnd68k_latches_w );
WRITE16_HANDLER( hdsnd68k_speech_w );
WRITE16_HANDLER( hdsnd68k_irqclr_w );

READ16_HANDLER( hdsnd68k_320ram_r );
WRITE16_HANDLER( hdsnd68k_320ram_w );
READ16_HANDLER( hdsnd68k_320ports_r );
WRITE16_HANDLER( hdsnd68k_320ports_w );
READ16_HANDLER( hdsnd68k_320com_r );
WRITE16_HANDLER( hdsnd68k_320com_w );

READ16_HANDLER( hdsnddsp_get_bio );

WRITE16_DEVICE_HANDLER( hdsnddsp_dac_w );
WRITE16_HANDLER( hdsnddsp_comport_w );
WRITE16_HANDLER( hdsnddsp_mute_w );
WRITE16_HANDLER( hdsnddsp_gen68kirq_w );
WRITE16_HANDLER( hdsnddsp_soundaddr_w );

READ16_HANDLER( hdsnddsp_rom_r );
READ16_HANDLER( hdsnddsp_comram_r );
READ16_HANDLER( hdsnddsp_compare_r );


/*----------- defined in video/harddriv.c -----------*/

VIDEO_START( harddriv );
void hdgsp_write_to_shiftreg(const address_space *space, uint32_t address, uint16_t *shiftreg);
void hdgsp_read_from_shiftreg(const address_space *space, uint32_t address, uint16_t *shiftreg);

READ16_HANDLER( hdgsp_control_lo_r );
WRITE16_HANDLER( hdgsp_control_lo_w );
READ16_HANDLER( hdgsp_control_hi_r );
WRITE16_HANDLER( hdgsp_control_hi_w );

READ16_HANDLER( hdgsp_vram_2bpp_r );
WRITE16_HANDLER( hdgsp_vram_1bpp_w );
WRITE16_HANDLER( hdgsp_vram_2bpp_w );

READ16_HANDLER( hdgsp_paletteram_lo_r );
WRITE16_HANDLER( hdgsp_paletteram_lo_w );
READ16_HANDLER( hdgsp_paletteram_hi_r );
WRITE16_HANDLER( hdgsp_paletteram_hi_w );

void harddriv_scanline_driver(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);
void harddriv_scanline_multisync(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);
