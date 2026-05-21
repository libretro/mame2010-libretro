#ifndef _68681_H
#define _68681_H

#include "devlegcy.h"

typedef struct _duart68681_config duart68681_config;
struct _duart68681_config
{
	void (*irq_handler)(running_device *device, uint8_t vector);
	void (*tx_callback)(running_device *device, int channel, uint8_t data);
	uint8_t (*input_port_read)(running_device *device);
	void (*output_port_write)(running_device *device, uint8_t data);

	/* clocks for external baud rates */
	int32_t ip3clk, ip4clk, ip5clk, ip6clk;
};

DECLARE_LEGACY_DEVICE(DUART68681, duart68681);

#define MDRV_DUART68681_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, DUART68681, _clock) \
	MDRV_DEVICE_CONFIG(_config)


READ8_DEVICE_HANDLER(duart68681_r);
WRITE8_DEVICE_HANDLER(duart68681_w);

void duart68681_rx_data( running_device* device, int ch, uint8_t data );

#endif //_68681_H
