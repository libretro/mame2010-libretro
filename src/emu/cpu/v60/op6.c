
/*
    FULLY TRUSTED
*/

static uint32_t opTB(v60_state *cpustate, int reg) /* TRUSTED */
{
	if (cpustate->reg[reg] == 0)
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t opDBGT(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	NORMALIZEFLAGS(cpustate);
	if ((cpustate->reg[reg] != 0) && !((cpustate->_S ^ cpustate->_OV) | cpustate->_Z))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t opDBLE(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	NORMALIZEFLAGS(cpustate);
	if ((cpustate->reg[reg] != 0) && ((cpustate->_S ^ cpustate->_OV) | cpustate->_Z))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}


static uint32_t opDBGE(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	NORMALIZEFLAGS(cpustate);
	if ((cpustate->reg[reg] != 0) && !(cpustate->_S ^ cpustate->_OV))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t opDBLT(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	NORMALIZEFLAGS(cpustate);
	if ((cpustate->reg[reg] != 0) && (cpustate->_S ^ cpustate->_OV))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t opDBH(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && !(cpustate->_CY | cpustate->_Z))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t opDBNH(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && (cpustate->_CY | cpustate->_Z))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}


static uint32_t opDBL(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && (cpustate->_CY))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t opDBNL(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && !(cpustate->_CY))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t opDBE(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && (cpustate->_Z))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t opDBNE(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && !(cpustate->_Z))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t opDBV(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && (cpustate->_OV))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t opDBNV(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && !(cpustate->_OV))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t opDBN(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && (cpustate->_S))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t opDBP(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if ((cpustate->reg[reg] != 0) && !(cpustate->_S))
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t opDBR(v60_state *cpustate, int reg) /* TRUSTED */
{
	cpustate->reg[reg]--;

	if (cpustate->reg[reg] != 0)
	{
		cpustate->PC += (int16_t)OpRead16(cpustate->program, cpustate->PC + 2);
		return 0;
	}

	return 4;
}

static uint32_t (*const OpC6Table[8])(v60_state *, int reg) = /* TRUSTED */
{
	opDBV,
	opDBL,
	opDBE,
	opDBNH,
	opDBN,
	opDBR,
	opDBLT,
	opDBLE
};

static uint32_t (*const OpC7Table[8])(v60_state *, int reg) = /* TRUSTED */
{
	opDBNV,
	opDBNL,
	opDBNE,
	opDBH,
	opDBP,
	opTB,
	opDBGE,
	opDBGT
};


static uint32_t opC6(v60_state *cpustate) /* TRUSTED */
{
	uint8_t appb = OpRead8(cpustate->program, cpustate->PC + 1);
	return OpC6Table[appb >> 5](cpustate, appb & 0x1f);
}

static uint32_t opC7(v60_state *cpustate) /* TRUSTED */
{
	uint8_t appb = OpRead8(cpustate->program, cpustate->PC + 1);
	return OpC7Table[appb >> 5](cpustate, appb & 0x1f);
}

