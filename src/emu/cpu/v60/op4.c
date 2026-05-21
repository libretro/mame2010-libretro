
/*
    FULLY TRUSTED
*/

static uint32_t opBGT8(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if (!((cpustate->_S ^ cpustate->_OV) | cpustate->_Z))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBGT16(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if (!((cpustate->_S ^ cpustate->_OV) | cpustate->_Z))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}


static uint32_t opBGE8(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if (!(cpustate->_S ^ cpustate->_OV))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBGE16(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if (!(cpustate->_S ^ cpustate->_OV))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static uint32_t opBLT8(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if ((cpustate->_S ^ cpustate->_OV))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBLT16(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if ((cpustate->_S ^ cpustate->_OV))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}


static uint32_t opBLE8(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if (((cpustate->_S ^ cpustate->_OV) | cpustate->_Z))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBLE16(v60_state *cpustate) /* TRUSTED */
{
	NORMALIZEFLAGS(cpustate);

	if (((cpustate->_S ^ cpustate->_OV) | cpustate->_Z))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static uint32_t opBH8(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_CY | cpustate->_Z))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBH16(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_CY | cpustate->_Z))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static uint32_t opBNH8(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_CY | cpustate->_Z))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBNH16(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_CY | cpustate->_Z))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static uint32_t opBNL8(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_CY))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBNL16(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_CY))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static uint32_t opBL8(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_CY))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBL16(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_CY))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static uint32_t opBNE8(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_Z))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBNE16(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_Z))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static uint32_t opBE8(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_Z))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBE16(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_Z))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static uint32_t opBNV8(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_OV))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBNV16(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_OV))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static uint32_t opBV8(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_OV))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBV16(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_OV))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static uint32_t opBP8(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_S))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBP16(v60_state *cpustate) /* TRUSTED */
{
	if (!(cpustate->_S))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static uint32_t opBN8(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_S))
	{
		cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 2;
}

static uint32_t opBN16(v60_state *cpustate) /* TRUSTED */
{
	if ((cpustate->_S))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
		return 0;
	}

	return 3;
}

static uint32_t opBR8(v60_state *cpustate) /* TRUSTED */
{
	cpustate->PC += (int8_t)OpRead8(cpustate->program, cpustate->PC + 1);
	return 0;
}

static uint32_t opBR16(v60_state *cpustate) /* TRUSTED */
{
	cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
	return 0;
}

static uint32_t opBSR(v60_state *cpustate) /* TRUSTED */
{
	// Save Next cpustate->PC onto the stack
	cpustate->SP -= 4;
	MemWrite32(cpustate->program, cpustate->SP, cpustate->PC + 3);

	// Jump to subroutine
	cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 1);
	return 0;
}

