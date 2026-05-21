/***************************************************************************

  snesrtc.c

  File to handle emulation of the SNES "S-RTC" add-on chip.

  Based on C++ implementation by Byuu in BSNES.

  Byuu's code is released under GNU General Public License
  version 2 as published by the Free Software Foundation.
  The implementation below is released under the MAME license
  for use in MAME, MESS and derivatives by permission of the
  author

***************************************************************************/

enum
{
	RTCM_Ready,
	RTCM_Command,
	RTCM_Read,
	RTCM_Write
};

struct _snes_rtc_state
{
	uint8_t  ram[13];
	int32_t  mode;
	int8_t   index;
};

static _snes_rtc_state rtc_state;

static const uint8_t srtc_months[12] =
{
	31, 28, 31,
	30, 31, 30,
	31, 31, 30,
	31, 30, 31
};

static void srtc_update_time( running_machine *machine )
{
	system_time curtime, *systime = &curtime;
	machine->current_datetime(curtime);
	rtc_state.ram[0] = systime->local_time.second % 10;
	rtc_state.ram[1] = systime->local_time.second / 10;
	rtc_state.ram[2] = systime->local_time.minute % 10;
	rtc_state.ram[3] = systime->local_time.minute / 10;
	rtc_state.ram[4] = systime->local_time.hour % 10;
	rtc_state.ram[5] = systime->local_time.hour / 10;
	rtc_state.ram[6] = systime->local_time.mday % 10;
	rtc_state.ram[7] = systime->local_time.mday / 10;
	rtc_state.ram[8] = systime->local_time.month;
	rtc_state.ram[9] = (systime->local_time.year - 1000) % 10;
	rtc_state.ram[10] = ((systime->local_time.year - 1000) / 10) % 10;
	rtc_state.ram[11] = (systime->local_time.year - 1000) / 100;
	rtc_state.ram[12] = systime->local_time.weekday % 7;
}

// Returns day-of-week for specified date
// e.g. 0 = Sunday, 1 = Monday, ... 6 = Saturday
// Usage: weekday(2008, 1, 1) returns the weekday of January 1st, 2008
static uint8_t srtc_weekday( uint32_t year, uint32_t month, uint32_t day )
{
	uint32_t y = 1900, m = 1;	// Epoch is 1900-01-01
	uint32_t sum = 0;			// Number of days passed since epoch

	year = MAX(1900, year);
	month = MAX(1, MIN(12, month));
	day = MAX(1, MIN(31, day));

	while (y < year)
	{
		uint8_t leapyear = 0;
		if ((y % 4) == 0)
		{
			leapyear = 1;
			if ((y % 100) == 0 && (y % 400) != 0)
			{
				leapyear = 0;
			}
		}
		sum += leapyear ? 366 : 365;
		y++;
	}

	while (m < month)
	{
		uint32_t days = srtc_months[m - 1];
		if (days == 28)
		{
			uint8_t leapyear = 0;
			if ((y % 4) == 0)
			{
				leapyear = 1;
				if ((y % 100) == 0 && (y % 400) != 0)
				{
					leapyear = 0;
				}
			}
			days += leapyear ? 1 : 0;
		}
		sum += days;
		m++;
	}

	sum += day - 1;
	return (sum + 1) % 7; // 1900-01-01 was a Monday
}

static uint8_t srtc_read( const address_space *space, uint16_t addr )
{
	addr &= 0xffff;

	if (addr == 0x2800)
	{
		if (rtc_state.mode != RTCM_Read)
		{
			return 0x00;
		}

		if (rtc_state.index < 0)
		{
			srtc_update_time(space->machine);
			rtc_state.index++;
			return 0x0f;
		}
		else if (rtc_state.index > 12)
		{
			rtc_state.index = -1;
			return 0x0f;
		}
		else
		{
			return rtc_state.ram[rtc_state.index++];
		}
	}

	return snes_open_bus_r(space, 0);
}

static void srtc_write( running_machine *machine, uint16_t addr, uint8_t data )
{
	addr &= 0xffff;

	if (addr == 0x2801)
	{
		data &= 0x0f;	// Only the low four bits are used

		if (data == 0x0d)
		{
			rtc_state.mode = RTCM_Read;
			rtc_state.index = -1;
			return;
		}

		if (data == 0x0e)
		{
			rtc_state.mode = RTCM_Command;
			return;
		}

		if (data == 0x0f)
		{
			return;	// Unknown behaviour
		}

		if (rtc_state.mode == RTCM_Write)
		{
			if (rtc_state.index >= 0 && rtc_state.index < 12)
			{
				rtc_state.ram[rtc_state.index++] = data;

				if (rtc_state.index == 12)
				{
					// Day of week is automatically calculated and written
					uint32_t day   = rtc_state.ram[6] + rtc_state.ram[7] * 10;
					uint32_t month = rtc_state.ram[8];
					uint32_t year  = rtc_state.ram[9] + rtc_state.ram[10] * 10 + rtc_state.ram[11] * 100;
					year += 1000;

					rtc_state.ram[rtc_state.index++] = srtc_weekday(year, month, day);
				}
			}
		}
		else if (rtc_state.mode == RTCM_Command)
		{
			if (data == 0)
			{
				rtc_state.mode = RTCM_Write;
				rtc_state.index = 0;
			}
			else if (data == 4)
			{
				uint8_t i;
				rtc_state.mode = RTCM_Ready;
				rtc_state.index = -1;
				for(i = 0; i < 13; i++)
				{
					rtc_state.ram[i] = 0;
				}
			}
			else
			{
				// Unknown behaviour
				rtc_state.mode = RTCM_Ready;
			}
		}
	}
}

static void srtc_init( running_machine *machine )
{
	rtc_state.mode = RTCM_Read;
	rtc_state.index = -1;
	srtc_update_time(machine);

	state_save_register_global_array(machine, rtc_state.ram);
	state_save_register_global(machine, rtc_state.mode);
	state_save_register_global(machine, rtc_state.index);
}
