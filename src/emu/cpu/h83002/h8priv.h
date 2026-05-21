/***************************************************************************

 h8priv.h : Private constants and other definitions for the H8/3002 emulator.

****************************************************************************/

#pragma once

#ifndef __H8PRIV_H__
#define __H8PRIV_H__

typedef struct _h83xx_state h83xx_state;
struct _h83xx_state
{
	// main CPU stuff
	uint32_t h8err;
	uint32_t regs[8];
	uint32_t pc, ppc;

	uint32_t h8_IRQrequestH, h8_IRQrequestL;
	int32_t cyccnt;

	uint8_t  ccr;
	uint8_t  h8nflag, h8vflag, h8cflag, h8zflag, h8iflag, h8hflag;
	uint8_t  h8uflag, h8uiflag;
	uint8_t  incheckirqs;

	device_irq_callback irq_cb;
	legacy_cpu_device *device;

	const address_space *program;
	const address_space *io;

	// onboard peripherals stuff
	uint8_t per_regs[256];

	uint16_t h8TCNT[5];
	uint8_t h8TSTR;

	uint8_t STCR, TCR[2], TCSR[2], TCORA[2], TCORB[2], TCNT[2];
	uint16_t FRC;

	emu_timer *timer[5];
	emu_timer *frctimer;

	int mode_8bit;
};
extern h83xx_state h8;

INLINE h83xx_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == H83002 ||
		   device->type() == H83007 ||
		   device->type() == H83044 ||
		   device->type() == H83334);
	return (h83xx_state *)downcast<legacy_cpu_device *>(device)->token();
}

uint8_t h8_register_read8(h83xx_state *h8, uint32_t address);
uint8_t h8_3007_register_read8(h83xx_state *h8, uint32_t address);
uint8_t h8_3007_register1_read8(h83xx_state *h8, uint32_t address);
void h8_register_write8(h83xx_state *h8, uint32_t address, uint8_t val);
void h8_3007_register_write8(h83xx_state *h8, uint32_t address, uint8_t val);
void h8_3007_register1_write8(h83xx_state *h8, uint32_t address, uint8_t val);

void h8_itu_init(h83xx_state *h8);
void h8_3007_itu_init(h83xx_state *h8);
void h8_itu_reset(h83xx_state *h8);
uint8_t h8_itu_read8(h83xx_state *h8, uint8_t reg);
uint8_t h8_3007_itu_read8(h83xx_state *h8, uint8_t reg);
void h8_itu_write8(h83xx_state *h8, uint8_t reg, uint8_t val);
void h8_3007_itu_write8(h83xx_state *h8, uint8_t reg, uint8_t val);

#endif /* __H8PRIV_H__ */
