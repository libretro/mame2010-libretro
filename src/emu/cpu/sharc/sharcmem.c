/* SHARC memory operations */

static uint32_t pm_read32(SHARC_REGS *cpustate, uint32_t address)
{
	if (address >= 0x20000 && address < 0x28000)
	{
		uint32_t addr = (address & 0x7fff) * 3;

		return (uint32_t)(cpustate->internal_ram_block0[addr + 0] << 16) |
					   (cpustate->internal_ram_block0[addr + 1]);
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		uint32_t addr = (address & 0x7fff) * 3;

		return (uint32_t)(cpustate->internal_ram_block1[addr + 0] << 16) |
					   (cpustate->internal_ram_block1[addr + 1]);
	}
	else {
		fatalerror("SHARC: PM Bus Read %08X at %08X", address, cpustate->pc);
	}
}

static void pm_write32(SHARC_REGS *cpustate, uint32_t address, uint32_t data)
{
	if (address >= 0x20000 && address < 0x28000)
	{
		uint32_t addr = (address & 0x7fff) * 3;

		cpustate->internal_ram_block0[addr + 0] = (uint16_t)(data >> 16);
		cpustate->internal_ram_block0[addr + 1] = (uint16_t)(data);
		return;
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		uint32_t addr = (address & 0x7fff) * 3;

		cpustate->internal_ram_block1[addr + 0] = (uint16_t)(data >> 16);
		cpustate->internal_ram_block1[addr + 1] = (uint16_t)(data);
		return;
	}
	else {
		fatalerror("SHARC: PM Bus Write %08X, %08X at %08X", address, data, cpustate->pc);
	}
}

static uint64_t pm_read48(SHARC_REGS *cpustate, uint32_t address)
{
	if (address >= 0x20000 && address < 0x28000)
	{
		uint32_t addr = (address & 0x7fff) * 3;

		return ((uint64_t)(cpustate->internal_ram_block0[addr + 0]) << 32) |
			   ((uint64_t)(cpustate->internal_ram_block0[addr + 1]) << 16) |
			   ((uint64_t)(cpustate->internal_ram_block0[addr + 2]) << 0);
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		uint32_t addr = (address & 0x7fff) * 3;

		return ((uint64_t)(cpustate->internal_ram_block1[addr + 0]) << 32) |
			   ((uint64_t)(cpustate->internal_ram_block1[addr + 1]) << 16) |
			   ((uint64_t)(cpustate->internal_ram_block1[addr + 2]) << 0);
	}
	else {
		fatalerror("SHARC: PM Bus Read %08X at %08X", address, cpustate->pc);
	}

	return 0;
}

static void pm_write48(SHARC_REGS *cpustate, uint32_t address, uint64_t data)
{
	if (address >= 0x20000 && address < 0x28000)
	{
		uint32_t addr = (address & 0x7fff) * 3;

		cpustate->internal_ram_block0[addr + 0] = (uint16_t)(data >> 32);
		cpustate->internal_ram_block0[addr + 1] = (uint16_t)(data >> 16);
		cpustate->internal_ram_block0[addr + 2] = (uint16_t)(data);
		return;
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		uint32_t addr = (address & 0x7fff) * 3;

		cpustate->internal_ram_block1[addr + 0] = (uint16_t)(data >> 32);
		cpustate->internal_ram_block1[addr + 1] = (uint16_t)(data >> 16);
		cpustate->internal_ram_block1[addr + 2] = (uint16_t)(data);
		return;
	}
	else {
		fatalerror("SHARC: PM Bus Write %08X, %04X%08X at %08X", address, (uint16_t)(data >> 32),(uint32_t)data, cpustate->pc);
	}
}

static uint32_t dm_read32(SHARC_REGS *cpustate, uint32_t address)
{
	if (address < 0x100)
	{
		return sharc_iop_r(cpustate, address);
	}
	else if (address >= 0x20000 && address < 0x28000)
	{
		uint32_t addr = (address & 0x7fff) * 2;

		return (uint32_t)(cpustate->internal_ram_block0[addr + 0] << 16) |
					   (cpustate->internal_ram_block0[addr + 1]);
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		uint32_t addr = (address & 0x7fff) * 2;

		return (uint32_t)(cpustate->internal_ram_block1[addr + 0] << 16) |
					   (cpustate->internal_ram_block1[addr + 1]);
	}

	// short word addressing
	else if (address >= 0x40000 && address < 0x50000)
	{
		uint32_t addr = address & 0xffff;

		uint16_t r = cpustate->internal_ram_block0[addr ^ 1];
		if (cpustate->mode1 & 0x4000)
		{
			// sign-extend
			return (int32_t)(int16_t)(r);
		}
		else
		{
			return (uint32_t)(r);
		}
	}
	else if (address >= 0x50000 && address < 0x80000)
	{
		// block 1 is mirrored in 0x50000...5ffff, 0x60000...0x6ffff and 0x70000...7ffff
		uint32_t addr = address & 0xffff;

		uint16_t r = cpustate->internal_ram_block1[addr ^ 1];
		if (cpustate->mode1 & 0x4000)
		{
			// sign-extend
			return (int32_t)(int16_t)(r);
		}
		else
		{
			return (uint32_t)(r);
		}
	}

	return memory_read_dword_32le(cpustate->data, address << 2);
}

static void dm_write32(SHARC_REGS *cpustate, uint32_t address, uint32_t data)
{
	if (address < 0x100)
	{
		sharc_iop_w(cpustate, address, data);
		return;
	}
	else if (address >= 0x20000 && address < 0x28000)
	{
		uint32_t addr = (address & 0x7fff) * 2;

		cpustate->internal_ram_block0[addr + 0] = (uint16_t)(data >> 16);
		cpustate->internal_ram_block0[addr + 1] = (uint16_t)(data);
		return;
	}
	else if (address >= 0x28000 && address < 0x40000)
	{
		// block 1 is mirrored in 0x28000...2ffff, 0x30000...0x37fff and 0x38000...3ffff
		uint32_t addr = (address & 0x7fff) * 2;

		cpustate->internal_ram_block1[addr + 0] = (uint16_t)(data >> 16);
		cpustate->internal_ram_block1[addr + 1] = (uint16_t)(data);
		return;
	}

	// short word addressing
	else if (address >= 0x40000 && address < 0x50000)
	{
		uint32_t addr = address & 0xffff;

		cpustate->internal_ram_block0[addr ^ 1] = data;
		return;
	}
	else if (address >= 0x50000 && address < 0x80000)
	{
		// block 1 is mirrored in 0x50000...5ffff, 0x60000...0x6ffff and 0x70000...7ffff
		uint32_t addr = address & 0xffff;

		cpustate->internal_ram_block1[addr ^ 1] = data;
		return;
	}

	memory_write_dword_32le(cpustate->data, address << 2, data);
}
