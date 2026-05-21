/***************************************************************************

    Z80 CTC (Z8430) implementation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#ifndef __Z80CTC_H__
#define __Z80CTC_H__

#include "cpu/z80/z80daisy.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int NOTIMER_0 = (1<<0);
const int NOTIMER_1 = (1<<1);
const int NOTIMER_2 = (1<<2);
const int NOTIMER_3 = (1<<3);



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define Z80CTC_INTERFACE(name) \
	const z80ctc_interface (name)=


#define MDRV_Z80CTC_ADD(_tag, _clock, _intrf) \
	MDRV_DEVICE_ADD(_tag, Z80CTC, _clock) \
	MDRV_DEVICE_CONFIG(_intrf)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> z80ctc_interface

struct z80ctc_interface
{
	uint8_t				m_notimer;	// timer disabler mask
	devcb_write_line	m_intr;		// callback when change interrupt status
	devcb_write_line	m_zc0;		// ZC/TO0 callback
	devcb_write_line	m_zc1;		// ZC/TO1 callback
	devcb_write_line	m_zc2;		// ZC/TO2 callback
};



// ======================> z80ctc_device_config

class z80ctc_device_config :	public device_config,
								public device_config_z80daisy_interface,
								public z80ctc_interface
{
	friend class z80ctc_device;

	// construction/destruction
	z80ctc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, uint32_t clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, uint32_t clock);
	virtual device_t *alloc_device(running_machine &machine) const;

protected:
	// device_config overrides
	virtual void device_config_complete();
};



// ======================> z80ctc_device

class z80ctc_device :	public device_t,
						public device_z80daisy_interface
{
	friend class z80ctc_device_config;

	// construction/destruction
	z80ctc_device(running_machine &_machine, const z80ctc_device_config &_config);

public:
	// state getters
	attotime period(int ch) const { return m_channel[ch].period(); }

	// I/O operations
	uint8_t read(int ch) { return m_channel[ch].read(); }
	void write(int ch, uint8_t data) { m_channel[ch].write(data); }
	void trigger(int ch, uint8_t data) { m_channel[ch].trigger(data); }

private:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// z80daisy_interface overrides
	virtual int z80daisy_irq_state();
	virtual int z80daisy_irq_ack();
	virtual void z80daisy_irq_reti();

	// internal helpers
	void interrupt_check();
	void timercallback(int chanindex);

	// a single channel within the CTC
	class ctc_channel
	{
	public:
		ctc_channel();

		void start(z80ctc_device *device, int index, bool notimer, const devcb_write_line *write_line);
		void reset();

		uint8_t read();
		void write(uint8_t data);

		attotime period() const;
		void trigger(uint8_t data);
		void timer_callback();

		z80ctc_device *	m_device;				// pointer back to our device
		int				m_index;				// our channel index
		devcb_resolved_write_line m_zc;			// zero crossing callbacks
		bool			m_notimer;				// timer disabled?
		uint16_t			m_mode;					// current mode
		uint16_t			m_tconst;				// time constant
		uint16_t			m_down;					// down counter (clock mode only)
		uint8_t			m_extclk;				// current signal from the external clock
		emu_timer *		m_timer;				// array of active timers
		uint8_t			m_int_state;			// interrupt status (for daisy chain)

	private:
		static TIMER_CALLBACK( static_timer_callback ) { reinterpret_cast<z80ctc_device::ctc_channel *>(ptr)->timer_callback(); }
	};

	// internal state
	const z80ctc_device_config &m_config;
	devcb_resolved_write_line m_intr;			// interrupt callback

	uint8_t				m_vector;				// interrupt vector
	attotime			m_period16;				// 16/system clock
	attotime			m_period256;			// 256/system clock
	ctc_channel			m_channel[4];			// data for each channel
};


// device type definition
extern const device_type Z80CTC;



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE8_DEVICE_HANDLER( z80ctc_w );
READ8_DEVICE_HANDLER( z80ctc_r );

WRITE_LINE_DEVICE_HANDLER( z80ctc_trg0_w );
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg1_w );
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg2_w );
WRITE_LINE_DEVICE_HANDLER( z80ctc_trg3_w );


#endif
