/*************************************************************************

     Microprose Games 3D hardware

*************************************************************************/

#include "devlegcy.h"
#include "cpu/tms34010/tms34010.h"


#define HOST_MONITOR_DISPLAY		0
#define VGB_MONITOR_DISPLAY			0
#define DRMATH_MONITOR_DISPLAY		0

class micro3d_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, micro3d_state(machine)); }

	micro3d_state(running_machine &machine) { }

	struct
	{
		union
		{
			struct
			{
				uint8_t gpdr;
				uint8_t aer;
				uint8_t ddr;
				uint8_t iera;
				uint8_t ierb;
				uint8_t ipra;
				uint8_t iprb;
				uint8_t isra;
				uint8_t isrb;
				uint8_t imra;
				uint8_t imrb;
				uint8_t vr;
				uint8_t tacr;
				uint8_t tbcr;
				uint8_t tcdcr;
				uint8_t tadr;
				uint8_t tbdr;
				uint8_t tcdr;
				uint8_t tddr;
				uint8_t scr;
				uint8_t ucr;
				uint8_t rsr;
				uint8_t tsr;
				uint8_t udr;
			};
			uint8_t regs[24];
		};

		emu_timer *timer_a;
	} mc68901;

	uint16_t				*shared_ram;
	running_device		*duart68681;
	uint8_t				m68681_tx0;

	/* Sound */
	uint8_t				sound_port_latch[4];
	uint8_t				dac_data;

	/* TI UART */
	uint8_t				ti_uart[9];
	int					ti_uart_mode_cycle;
	int					ti_uart_sync_cycle;

	/* ADC */
	uint8_t				adc_val;

	/* Hardware version-check latch for BOTSS 1.1a */
	uint8_t				botssa_latch;

	/* MAC */
	uint32_t				*mac_sram;
	uint32_t				sram_r_addr;
	uint32_t				sram_w_addr;
	uint32_t				vtx_addr;
	uint32_t				mrab11;
	uint32_t				mac_stat;
	uint32_t				mac_inst;

	/* 2D video */
	uint16_t				*micro3d_sprite_vram;
	uint16_t				creg;
	uint16_t				xfer3dk;

	/* 3D pipeline */
	uint32_t				pipe_data;
	uint32_t				pipeline_state;
	int32_t				vtx_fifo[512];
	uint32_t				fifo_idx;
	uint32_t				draw_cmd;
	int					draw_state;
	int32_t				x_min;
	int32_t				x_max;
	int32_t				y_min;
	int32_t				y_max;
	int32_t				z_min;
	int32_t				z_max;
	int32_t				x_mid;
	int32_t				y_mid;
	int					dpram_bank;
	uint32_t				draw_dpram[1024];
	uint16_t				*frame_buffers[2];
	uint16_t				*tmp_buffer;
	int					drawing_buffer;
	int					display_buffer;
};

typedef struct _micro3d_vtx_
{
	int32_t x, y, z;
} micro3d_vtx;

/*----------- defined in drivers/micro3d.c -----------*/
extern uint16_t *micro3d_sprite_vram;
void m68901_int_gen(running_machine *machine, int source);

/*----------- defined in machine/micro3d.c -----------*/
READ16_HANDLER( micro3d_mc68901_r );
WRITE16_HANDLER( micro3d_mc68901_w );

READ16_HANDLER( micro3d_ti_uart_r );
WRITE16_HANDLER( micro3d_ti_uart_w );

READ32_HANDLER( micro3d_scc_r );
WRITE32_HANDLER( micro3d_scc_w );

READ16_HANDLER( micro3d_tms_host_r );
WRITE16_HANDLER( micro3d_tms_host_w );

READ16_HANDLER( micro3d_adc_r );
WRITE16_HANDLER( micro3d_adc_w );

WRITE16_HANDLER( host_drmath_int_w );
WRITE16_HANDLER( micro3d_reset_w );

READ16_HANDLER( micro3d_encoder_l_r );
READ16_HANDLER( micro3d_encoder_h_r );

CUSTOM_INPUT( botssa_hwchk_r );
READ16_HANDLER( botssa_140000_r );
READ16_HANDLER( botssa_180000_r );

READ32_HANDLER( micro3d_shared_r );
WRITE32_HANDLER( micro3d_shared_w );

WRITE32_HANDLER( drmath_int_w );
WRITE32_HANDLER( drmath_intr2_ack );

WRITE32_HANDLER( micro3d_mac1_w );
WRITE32_HANDLER( micro3d_mac2_w );
READ32_HANDLER( micro3d_mac2_r );

void micro3d_duart_irq_handler(running_device *device, uint8_t vector);
uint8_t micro3d_duart_input_r(running_device *device);
void micro3d_duart_output_w(running_device *device, uint8_t data);
void micro3d_duart_tx(running_device *device, int channel, uint8_t data);

MACHINE_RESET( micro3d );
DRIVER_INIT( micro3d );
DRIVER_INIT( botssa );

/*----------- defined in audio/micro3d.c -----------*/
WRITE8_DEVICE_HANDLER( micro3d_upd7759_w );
WRITE8_HANDLER( micro3d_snd_dac_a );
WRITE8_HANDLER( micro3d_snd_dac_b );
READ8_HANDLER( micro3d_sound_io_r );
WRITE8_HANDLER( micro3d_sound_io_w );

void micro3d_noise_sh_w(running_machine *machine, uint8_t data);

DECLARE_LEGACY_SOUND_DEVICE(MICRO3D, micro3d_sound);

/*----------- defined in video/micro3d.c -----------*/
VIDEO_START( micro3d );
VIDEO_UPDATE( micro3d );
VIDEO_RESET( micro3d );

void micro3d_tms_interrupt(running_device *device, int state);
void micro3d_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params);

WRITE16_HANDLER( micro3d_clut_w );
WRITE16_HANDLER( micro3d_creg_w );
WRITE16_HANDLER( micro3d_xfer3dk_w );
READ32_HANDLER( micro3d_pipe_r );
WRITE32_HANDLER( micro3d_fifo_w );
WRITE32_HANDLER( micro3d_alt_fifo_w );

INTERRUPT_GEN( micro3d_vblank );
