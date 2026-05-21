/****************************************************************/
/* Structure defining all callbacks for different architectures */
/****************************************************************/

struct cpu_info {
	uint8_t  (*mr8) (const address_space *space, offs_t address);
	void   (*mw8) (const address_space *space, offs_t address, uint8_t  data);
	uint16_t (*mr16)(const address_space *space, offs_t address);
	void   (*mw16)(const address_space *space, offs_t address, uint16_t data);
	uint32_t (*mr32)(const address_space *space, offs_t address);
	void   (*mw32)(const address_space *space, offs_t address, uint32_t data);
	uint8_t  (*or8) (const address_space *space, offs_t address);
	uint16_t (*or16)(const address_space *space, offs_t address);
	uint32_t (*or32)(const address_space *space, offs_t address);
	uint32_t start_pc;
};



/*****************************************************************/
/* Memory accesses for 16-bit data bus, 24-bit address bus (V60) */
/*****************************************************************/

#define MemRead8_16		memory_read_byte_16le
#define MemWrite8_16	memory_write_byte_16le

static uint16_t MemRead16_16(const address_space *space, offs_t address)
{
	if (!(address & 1))
		return memory_read_word_16le(space, address);
	else
	{
		uint16_t result = memory_read_byte_16le(space, address);
		return result | memory_read_byte_16le(space, address + 1) << 8;
	}
}

static void MemWrite16_16(const address_space *space, offs_t address, uint16_t data)
{
	if (!(address & 1))
		memory_write_word_16le(space, address, data);
	else
	{
		memory_write_byte_16le(space, address, data);
		memory_write_byte_16le(space, address + 1, data >> 8);
	}
}

static uint32_t MemRead32_16(const address_space *space, offs_t address)
{
	if (!(address & 1))
	{
		uint32_t result = memory_read_word_16le(space, address);
		return result | (memory_read_word_16le(space, address + 2) << 16);
	}
	else
	{
		uint32_t result = memory_read_byte_16le(space, address);
		result |= memory_read_word_16le(space, address + 1) << 8;
		return result | memory_read_byte_16le(space, address + 3) << 24;
	}
}

static void MemWrite32_16(const address_space *space, offs_t address, uint32_t data)
{
	if (!(address & 1))
	{
		memory_write_word_16le(space, address, data);
		memory_write_word_16le(space, address + 2, data >> 16);
	}
	else
	{
		memory_write_byte_16le(space, address, data);
		memory_write_word_16le(space, address + 1, data >> 8);
		memory_write_byte_16le(space, address + 3, data >> 24);
	}
}


/*****************************************************************/
/* Opcode accesses for 16-bit data bus, 24-bit address bus (V60) */
/*****************************************************************/

static uint8_t OpRead8_16(const address_space *space, offs_t address)
{
	return memory_decrypted_read_byte(space, BYTE_XOR_LE(address));
}

static uint16_t OpRead16_16(const address_space *space, offs_t address)
{
	return memory_decrypted_read_byte(space, BYTE_XOR_LE(address)) | (memory_decrypted_read_byte(space, BYTE_XOR_LE(address + 1)) << 8);
}

static uint32_t OpRead32_16(const address_space *space, offs_t address)
{
	return memory_decrypted_read_byte(space, BYTE_XOR_LE(address)) | (memory_decrypted_read_byte(space, BYTE_XOR_LE(address + 1)) << 8) |
			(memory_decrypted_read_byte(space, BYTE_XOR_LE(address + 2)) << 16) | (memory_decrypted_read_byte(space, BYTE_XOR_LE(address + 3)) << 24);
}



/*****************************************************************/
/* Memory accesses for 32-bit data bus, 32-bit address bus (V70) */
/*****************************************************************/

#define MemRead8_32		memory_read_byte_32le
#define MemWrite8_32	memory_write_byte_32le

static uint16_t MemRead16_32(const address_space *space, offs_t address)
{
	if (!(address & 1))
		return memory_read_word_32le(space, address);
	else
	{
		uint16_t result = memory_read_byte_32le(space, address);
		return result | memory_read_byte_32le(space, address + 1) << 8;
	}
}

static void MemWrite16_32(const address_space *space, offs_t address, uint16_t data)
{
	if (!(address & 1))
		memory_write_word_32le(space, address, data);
	else
	{
		memory_write_byte_32le(space, address, data);
		memory_write_byte_32le(space, address + 1, data >> 8);
	}
}

static uint32_t MemRead32_32(const address_space *space, offs_t address)
{
	if (!(address & 3))
		return memory_read_dword_32le(space, address);
	else if (!(address & 1))
	{
		uint32_t result = memory_read_word_32le(space, address);
		return result | (memory_read_word_32le(space, address + 2) << 16);
	}
	else
	{
		uint32_t result = memory_read_byte_32le(space, address);
		result |= memory_read_word_32le(space, address + 1) << 8;
		return result | memory_read_byte_32le(space, address + 3) << 24;
	}
}

static void MemWrite32_32(const address_space *space, offs_t address, uint32_t data)
{
	if (!(address & 3))
		memory_write_dword_32le(space, address, data);
	else if (!(address & 1))
	{
		memory_write_word_32le(space, address, data);
		memory_write_word_32le(space, address + 2, data >> 16);
	}
	else
	{
		memory_write_byte_32le(space, address, data);
		memory_write_word_32le(space, address + 1, data >> 8);
		memory_write_byte_32le(space, address + 3, data >> 24);
	}
}



/*****************************************************************/
/* Opcode accesses for 32-bit data bus, 32-bit address bus (V60) */
/*****************************************************************/

static uint8_t OpRead8_32(const address_space *space, offs_t address)
{
	return memory_decrypted_read_byte(space, BYTE4_XOR_LE(address));
}

static uint16_t OpRead16_32(const address_space *space, offs_t address)
{
	return memory_decrypted_read_byte(space, BYTE4_XOR_LE(address)) | (memory_decrypted_read_byte(space, BYTE4_XOR_LE(address + 1)) << 8);
}

static uint32_t OpRead32_32(const address_space *space, offs_t address)
{
	return memory_decrypted_read_byte(space, BYTE4_XOR_LE(address)) | (memory_decrypted_read_byte(space, BYTE4_XOR_LE(address + 1)) << 8) |
			(memory_decrypted_read_byte(space, BYTE4_XOR_LE(address + 2)) << 16) | (memory_decrypted_read_byte(space, BYTE4_XOR_LE(address + 3)) << 24);
}



/************************************************/
/* Structures pointing to various I/O functions */
/************************************************/

static const struct cpu_info v60_i =
{
	MemRead8_16,  MemWrite8_16,  MemRead16_16,  MemWrite16_16,  MemRead32_16,  MemWrite32_16,
	OpRead8_16,                  OpRead16_16,                   OpRead32_16,
	0xfffff0
};

static const struct cpu_info v70_i =
{
	MemRead8_32,  MemWrite8_32,  MemRead16_32,  MemWrite16_32,  MemRead32_32,  MemWrite32_32,
	OpRead8_32,                  OpRead16_32,                   OpRead32_32,
	0xfffffff0
};



/**************************************/
/* Macro shorthands for I/O functions */
/**************************************/

#define MemRead8    cpustate->info.mr8
#define MemWrite8   cpustate->info.mw8
#define MemRead16   cpustate->info.mr16
#define MemWrite16  cpustate->info.mw16
#define MemRead32   cpustate->info.mr32
#define MemWrite32  cpustate->info.mw32

#if !defined(MSB_FIRST) && !defined(ALIGN_INTS)
#define OpRead8(s, a)	(memory_decrypted_read_byte(s, a))
#define OpRead16(s, a)	(memory_decrypted_read_word(s, a))
#define OpRead32(s, a)	(memory_decrypted_read_dword(s, a))
#else
#define OpRead8     cpustate->info.or8
#define OpRead16    cpustate->info.or16
#define OpRead32    cpustate->info.or32
#endif
