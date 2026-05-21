/*****************************************************************************
    8-bit memory accessors
 *****************************************************************************/

#ifdef I8086
static void configure_memory_8bit(i8086_state *cpustate)
{
	cpustate->mem.fetch_xor = 0;

	cpustate->mem.rbyte = memory_read_byte_8le;
	cpustate->mem.rword = memory_read_word_8le;
	cpustate->mem.wbyte = memory_write_byte_8le;
	cpustate->mem.wword = memory_write_word_8le;
}
#endif


/*****************************************************************************
    16-bit memory accessors
 *****************************************************************************/

static uint16_t read_word_16le(const address_space *space, offs_t addr)
{
	if (!(addr & 1))
		return memory_read_word_16le(space, addr);
	else
	{
		uint16_t result = memory_read_byte_16le(space, addr);
		return result | (memory_read_byte_16le(space, addr + 1) << 8);
	}
}

static void write_word_16le(const address_space *space, offs_t addr, uint16_t data)
{
	if (!(addr & 1))
		memory_write_word_16le(space, addr, data);
	else
	{
		memory_write_byte_16le(space, addr, data);
		memory_write_byte_16le(space, addr + 1, data >> 8);
	}
}

static void configure_memory_16bit(i8086_state *cpustate)
{
	cpustate->mem.fetch_xor = BYTE_XOR_LE(0);

	cpustate->mem.rbyte = memory_read_byte_16le;
	cpustate->mem.rword = read_word_16le;
	cpustate->mem.wbyte = memory_write_byte_16le;
	cpustate->mem.wword = write_word_16le;
}
