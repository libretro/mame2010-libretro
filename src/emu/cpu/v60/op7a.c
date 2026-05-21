
/*
 * CMPC: What happens to cpustate->_S flag if the strings are identical?
 *   I suppose that it will be cleared. And is it set or cleared
 *   when the first one is a substring of the second? I suppose
 *   cleared (since cpustate->_S should be (src > dst))
 * MOVC: Why MOVCS does not exist in downward version?
 * SHCHDB / SHCHDH: cpustate->R27 is filled with the offset from the start or from the end?
 *
 * Strange stuff:
 *   SCHC opcodes does *not* modify cpustate->_Z flag as stated in V60 manual:
 *   they do the opposite (set if not found, reset if found)
 */

#define F7AEND(cs)	\
	return (cs)->amlength1 + (cs)->amlength2 + 4;

#define F7BEND(cs)	\
	return (cs)->amlength1 + (cs)->amlength2 + 3;

#define F7CEND(cs)	\
	return (cs)->amlength1 + (cs)->amlength2 + 3;

#define F7BCREATEBITMASK(x)	\
	x = ((1 << (x)) - 1)

#define F7CCREATEBITMASK(x)	\
	x = ((1 << (x)) - 1)

static void F7aDecodeOperands(v60_state *cpustate, uint32_t (*DecodeOp1)(v60_state *), uint8_t dim1, uint32_t (*DecodeOp2)(v60_state *), uint8_t dim2)
{
	uint8_t appb;
	// Decode first operand
	cpustate->moddim = dim1;
	cpustate->modm = cpustate->subop & 0x40;
	cpustate->modadd = cpustate->PC + 2;
	cpustate->amlength1 = DecodeOp1(cpustate);
	cpustate->flag1 = cpustate->amflag;
	cpustate->op1 = cpustate->amout;

	// Decode length
	appb = OpRead8(cpustate->program, cpustate->PC + 2 + cpustate->amlength1);
	if (appb & 0x80)
		cpustate->lenop1 = cpustate->reg[appb & 0x1F];
	else
		cpustate->lenop1 = appb;

	// Decode second operand
	cpustate->moddim = dim2;
	cpustate->modm = cpustate->subop & 0x20;
	cpustate->modadd = cpustate->PC + 3 + cpustate->amlength1;
	cpustate->amlength2 = DecodeOp2(cpustate);
	cpustate->flag2 = cpustate->amflag;
	cpustate->op2 = cpustate->amout;

	// Decode length
	appb = OpRead8(cpustate->program, cpustate->PC + 3 + cpustate->amlength1 + cpustate->amlength2);
	if (appb & 0x80)
		cpustate->lenop2 = cpustate->reg[appb & 0x1F];
	else
		cpustate->lenop2 = appb;
}

static void F7bDecodeFirstOperand(v60_state *cpustate, uint32_t (*DecodeOp1)(v60_state *), uint8_t dim1)
{
	uint8_t appb;
	// Decode first operand
	cpustate->moddim = dim1;
	cpustate->modm = cpustate->subop & 0x40;
	cpustate->modadd = cpustate->PC + 2;
	cpustate->amlength1 = DecodeOp1(cpustate);
	cpustate->flag1 = cpustate->amflag;
	cpustate->op1 = cpustate->amout;

	// Decode ext
	appb = OpRead8(cpustate->program, cpustate->PC + 2 + cpustate->amlength1);
	if (appb & 0x80)
		cpustate->lenop1 = cpustate->reg[appb & 0x1F];
	else
		cpustate->lenop1 = appb;
}


static void F7bWriteSecondOperand(v60_state *cpustate, uint8_t dim2)
{
	cpustate->moddim = dim2;
	cpustate->modm = cpustate->subop & 0x20;
	cpustate->modadd = cpustate->PC + 3 + cpustate->amlength1;
	cpustate->amlength2 = WriteAM(cpustate);
}


static void F7bDecodeOperands(v60_state *cpustate, uint32_t (*DecodeOp1)(v60_state *), uint8_t dim1, uint32_t (*DecodeOp2)(v60_state *), uint8_t dim2)
{
	// Decode first operand
	F7bDecodeFirstOperand(cpustate, DecodeOp1, dim1);
	cpustate->bamoffset1 = cpustate->bamoffset;

	// Decode second operand
	cpustate->moddim = dim2;
	cpustate->modm = cpustate->subop & 0x20;
	cpustate->modadd = cpustate->PC + 3 + cpustate->amlength1;
	cpustate->amlength2 = DecodeOp2(cpustate);
	cpustate->flag2 = cpustate->amflag;
	cpustate->op2 = cpustate->amout;
	cpustate->bamoffset2 = cpustate->bamoffset;
}

static void F7cDecodeOperands(v60_state *cpustate, uint32_t (*DecodeOp1)(v60_state *), uint8_t dim1, uint32_t (*DecodeOp2)(v60_state *), uint8_t dim2)
{
	uint8_t appb;
	// Decode first operand
	cpustate->moddim = dim1;
	cpustate->modm = cpustate->subop & 0x40;
	cpustate->modadd = cpustate->PC + 2;
	cpustate->amlength1 = DecodeOp1(cpustate);
	cpustate->flag1 = cpustate->amflag;
	cpustate->op1 = cpustate->amout;

	// Decode second operand
	cpustate->moddim = dim2;
	cpustate->modm = cpustate->subop & 0x20;
	cpustate->modadd = cpustate->PC + 2 + cpustate->amlength1;
	cpustate->amlength2 = DecodeOp2(cpustate);
	cpustate->flag2 = cpustate->amflag;
	cpustate->op2 = cpustate->amout;

	// Decode ext
	appb = OpRead8(cpustate->program, cpustate->PC + 2 + cpustate->amlength1 + cpustate->amlength2);
	if (appb & 0x80)
		cpustate->lenop1 = cpustate->reg[appb & 0x1F];
	else
		cpustate->lenop1 = appb;
}

#define F7CLOADOP1BYTE(cs,appb) \
	if ((cs)->flag1) \
		appb = (uint8_t)((cs)->reg[(cs)->op1]&0xFF); \
	else \
		appb = MemRead8((cs)->program, (cs)->op1);

#define F7CLOADOP2BYTE(cs,appb) \
	if ((cs)->flag2) \
		appb = (uint8_t)((cs)->reg[(cs)->op2]&0xFF); \
	else \
		appb = MemRead8((cs)->program, (cs)->op2);


#define F7CSTOREOP2BYTE(cs) \
	if ((cs)->flag2) \
		SETREG8((cs)->reg[(cs)->op2], appb); \
	else \
		MemWrite8((cs)->program, (cs)->op2, appb);

#define F7CSTOREOP2HALF(cs) \
	if ((cs)->flag2) \
		SETREG16((cs)->reg[(cs)->op2], apph); \
	else \
		MemWrite16((cs)->program, (cs)->op2, apph);

static uint32_t opCMPSTRB(v60_state *cpustate, uint8_t bFill, uint8_t bStop)
{
	uint32_t i, dest;
	uint8_t c1, c2;

	F7aDecodeOperands(cpustate, ReadAMAddress, 0,ReadAMAddress, 0);

	// Filling
	if (bFill)
	{
		if (cpustate->lenop1 < cpustate->lenop2)
		{
			for (i = cpustate->lenop1; i < cpustate->lenop2; i++)
				MemWrite8(cpustate->program, cpustate->op1 + i,(uint8_t)cpustate->R26);
		}
		else if (cpustate->lenop2 < cpustate->lenop1)
		{
			for (i = cpustate->lenop2; i < cpustate->lenop1; i++)
				MemWrite8(cpustate->program, cpustate->op2 + i,(uint8_t)cpustate->R26);
		}
	}

	dest = (cpustate->lenop1 < cpustate->lenop2 ? cpustate->lenop1 : cpustate->lenop2);

	cpustate->_Z = 0;
	cpustate->_S = 0;
	if (bStop) cpustate->_CY = 1;

	for (i = 0; i < dest; i++)
	{
		c1 = MemRead8(cpustate->program, cpustate->op1 + i);
		c2 = MemRead8(cpustate->program, cpustate->op2 + i);

		if (c1 > c2)
		{
			cpustate->_S = 1;	break;
		}
		else if (c2 > c1)
		{
			cpustate->_S = 0;	break;
		}

		if (bStop)
			if (c1 == (uint8_t)cpustate->R26 || c2 == (uint8_t)cpustate->R26)
			{
				cpustate->_CY = 0;
				break;
			}
	}

	cpustate->R28 = cpustate->lenop1 + i;
	cpustate->R27 = cpustate->lenop2 + i;

	if (i == dest)
	{
		if (cpustate->lenop1 > cpustate->lenop2)
			cpustate->_S = 1;
		else if (cpustate->lenop2 > cpustate->lenop1)
			cpustate->_S = 0;
		else
			cpustate->_Z = 1;
	}

	F7AEND(cpustate);
}

static uint32_t opCMPSTRH(v60_state *cpustate, uint8_t bFill, uint8_t bStop)
{
	uint32_t i, dest;
	uint16_t c1, c2;

	F7aDecodeOperands(cpustate, ReadAMAddress, 0,ReadAMAddress, 0);

	// Filling
	if (bFill)
	{
		if (cpustate->lenop1 < cpustate->lenop2)
		{
			for (i = cpustate->lenop1; i < cpustate->lenop2; i++)
				MemWrite16(cpustate->program, cpustate->op1 + i * 2,(uint16_t)cpustate->R26);
		}
		else if (cpustate->lenop2 < cpustate->lenop1)
		{
			for (i = cpustate->lenop2; i < cpustate->lenop1; i++)
				MemWrite16(cpustate->program, cpustate->op2 + i * 2,(uint16_t)cpustate->R26);
		}
	}

	dest = (cpustate->lenop1 < cpustate->lenop2 ? cpustate->lenop1 : cpustate->lenop2);

	cpustate->_Z = 0;
	cpustate->_S = 0;
	if (bStop) cpustate->_CY = 1;

	for (i = 0; i < dest; i++)
	{
		c1 = MemRead16(cpustate->program, cpustate->op1 + i * 2);
		c2 = MemRead16(cpustate->program, cpustate->op2 + i * 2);

		if (c1 > c2)
		{
			cpustate->_S = 1;	break;
		}
		else if (c2 > c1)
		{
			cpustate->_S = 0;	break;
		}

		if (bStop)
			if (c1 == (uint16_t)cpustate->R26 || c2 == (uint16_t)cpustate->R26)
			{
				cpustate->_CY = 0;
				break;
			}
	}

	cpustate->R28 = cpustate->lenop1 + i * 2;
	cpustate->R27 = cpustate->lenop2 + i * 2;

	if (i == dest)
	{
		if (cpustate->lenop1 > cpustate->lenop2)
			cpustate->_S = 1;
		else if (cpustate->lenop2 > cpustate->lenop1)
			cpustate->_S = 0;
		else
			cpustate->_Z = 1;
	}

	F7AEND(cpustate);
}



static uint32_t opMOVSTRUB(v60_state *cpustate, uint8_t bFill, uint8_t bStop) /* TRUSTED (0, 0) (1, 0) */
{
	uint32_t i, dest;
	uint8_t c1;

//  if (bStop)
//  {
//      int a = 1;
//  }

	F7aDecodeOperands(cpustate, ReadAMAddress, 0,ReadAMAddress, 0);

	dest = (cpustate->lenop1 < cpustate->lenop2 ? cpustate->lenop1 : cpustate->lenop2);

	for (i = 0; i < dest; i++)
	{
		MemWrite8(cpustate->program, cpustate->op2 + i,(c1 = MemRead8(cpustate->program, cpustate->op1 + i)));

		if (bStop && c1 == (uint8_t)cpustate->R26)
			break;
	}

	cpustate->R28 = cpustate->op1 + i;
	cpustate->R27 = cpustate->op2 + i;

	if (bFill && cpustate->lenop1 < cpustate->lenop2)
	{
		for (;i < cpustate->lenop2; i++)
			MemWrite8(cpustate->program, cpustate->op2 + i,(uint8_t)cpustate->R26);

		cpustate->R27 = cpustate->op2 + i;
	}


	F7AEND(cpustate);
}

static uint32_t opMOVSTRDB(v60_state *cpustate, uint8_t bFill, uint8_t bStop)
{
	uint32_t i, dest;
	uint8_t c1;

	F7aDecodeOperands(cpustate, ReadAMAddress, 0,ReadAMAddress, 0);

	dest = (cpustate->lenop1 < cpustate->lenop2 ? cpustate->lenop1 : cpustate->lenop2);

	for (i = 0; i < dest; i++)
	{
		MemWrite8(cpustate->program, cpustate->op2 + (dest - i - 1),(c1 = MemRead8(cpustate->program, cpustate->op1 + (dest - i - 1))));

		if (bStop && c1 == (uint8_t)cpustate->R26)
			break;
	}

	cpustate->R28 = cpustate->op1 + (cpustate->lenop1 - i - 1);
	cpustate->R27 = cpustate->op2 + (cpustate->lenop2 - i - 1);

	if (bFill && cpustate->lenop1 < cpustate->lenop2)
	{
		for (;i < cpustate->lenop2; i++)
			MemWrite8(cpustate->program, cpustate->op2 + dest + (cpustate->lenop2 - i - 1),(uint8_t)cpustate->R26);

		cpustate->R27 = cpustate->op2 + (cpustate->lenop2 - i - 1);
	}


	F7AEND(cpustate);
}


static uint32_t opMOVSTRUH(v60_state *cpustate, uint8_t bFill, uint8_t bStop) /* TRUSTED (0, 0) (1, 0) */
{
	uint32_t i, dest;
	uint16_t c1;

//  if (bStop)
//  {   int a = 1; }

	F7aDecodeOperands(cpustate, ReadAMAddress, 1,ReadAMAddress, 1);

	dest = (cpustate->lenop1 < cpustate->lenop2 ? cpustate->lenop1 : cpustate->lenop2);

	for (i = 0; i < dest; i++)
	{
		MemWrite16(cpustate->program, cpustate->op2 + i * 2,(c1 = MemRead16(cpustate->program, cpustate->op1 + i * 2)));

		if (bStop && c1 == (uint16_t)cpustate->R26)
			break;
	}

	cpustate->R28 = cpustate->op1 + i * 2;
	cpustate->R27 = cpustate->op2 + i * 2;

	if (bFill && cpustate->lenop1 < cpustate->lenop2)
	{
		for (;i < cpustate->lenop2; i++)
			MemWrite16(cpustate->program, cpustate->op2 + i * 2,(uint16_t)cpustate->R26);

		cpustate->R27 = cpustate->op2 + i * 2;
	}

	F7AEND(cpustate);
}

static uint32_t opMOVSTRDH(v60_state *cpustate, uint8_t bFill, uint8_t bStop)
{
	uint32_t i, dest;
	uint16_t c1;

//  if (bFill | bStop)
//  { int a = 1; }

	F7aDecodeOperands(cpustate, ReadAMAddress, 1,ReadAMAddress, 1);

//  if (cpustate->lenop1 != cpustate->lenop2)
//  { int a = 1; }

	dest = (cpustate->lenop1 < cpustate->lenop2 ? cpustate->lenop1 : cpustate->lenop2);

	for (i = 0; i < dest; i++)
	{
		MemWrite16(cpustate->program, cpustate->op2 + (dest - i - 1) * 2,(c1 = MemRead16(cpustate->program, cpustate->op1 + (dest - i - 1) * 2)));

		if (bStop && c1 == (uint16_t)cpustate->R26)
			break;
	}

	cpustate->R28 = cpustate->op1 + (cpustate->lenop1 - i - 1) * 2;
	cpustate->R27 = cpustate->op2 + (cpustate->lenop2 - i - 1) * 2;

	if (bFill && cpustate->lenop1 < cpustate->lenop2)
	{
		for (;i < cpustate->lenop2; i++)
			MemWrite16(cpustate->program, cpustate->op2 + (cpustate->lenop2 - i - 1) * 2,(uint16_t)cpustate->R26);

		cpustate->R27 = cpustate->op2 + (cpustate->lenop2 - i - 1) * 2;
	}

	F7AEND(cpustate);
}

static uint32_t opSEARCHUB(v60_state *cpustate, uint8_t bSearch)
{
	uint8_t appb;
	uint32_t i;

	F7bDecodeOperands(cpustate, ReadAMAddress, 0,ReadAM, 0);

	for (i = 0; i < cpustate->lenop1; i++)
	{
		appb = (MemRead8(cpustate->program, cpustate->op1 + i) == (uint8_t)cpustate->op2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	cpustate->R28 = cpustate->op1 + i;
	cpustate->R27 = i;

	// This is the opposite as stated in V60 manual...
	if (i != cpustate->lenop1)
		cpustate->_Z = 0;
	else
		cpustate->_Z = 1;

	F7BEND(cpustate);
}

static uint32_t opSEARCHUH(v60_state *cpustate, uint8_t bSearch)
{
	uint8_t appb;
	uint32_t i;

	F7bDecodeOperands(cpustate, ReadAMAddress, 1,ReadAM, 1);

	for (i = 0; i < cpustate->lenop1; i++)
	{
		appb = (MemRead16(cpustate->program, cpustate->op1 + i * 2) == (uint16_t)cpustate->op2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	cpustate->R28 = cpustate->op1 + i * 2;
	cpustate->R27 = i;

	if (i != cpustate->lenop1)
		cpustate->_Z = 0;
	else
		cpustate->_Z = 1;

	F7BEND(cpustate);
}

static uint32_t opSEARCHDB(v60_state *cpustate, uint8_t bSearch)
{
	uint8_t appb;
	int32_t i;

	F7bDecodeOperands(cpustate, ReadAMAddress, 0,ReadAM, 0);

	for (i = cpustate->lenop1; i >= 0; i--)
	{
		appb = (MemRead8(cpustate->program, cpustate->op1 + i) == (uint8_t)cpustate->op2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	cpustate->R28 = cpustate->op1 + i;
	cpustate->R27 = i;

	// This is the opposite as stated in V60 manual...
	if ((uint32_t)i != cpustate->lenop1)
		cpustate->_Z = 0;
	else
		cpustate->_Z = 1;

	F7BEND(cpustate);
}

static uint32_t opSEARCHDH(v60_state *cpustate, uint8_t bSearch)
{
	uint8_t appb;
	int32_t i;

	F7bDecodeOperands(cpustate, ReadAMAddress, 1,ReadAM, 1);

	for (i = cpustate->lenop1 - 1; i >= 0; i--)
	{
		appb = (MemRead16(cpustate->program, cpustate->op1 + i * 2) == (uint16_t)cpustate->op2);
		if ((bSearch && appb) || (!bSearch && !appb))
			break;
	}

	cpustate->R28 = cpustate->op1 + i * 2;
	cpustate->R27 = i;

	if ((uint32_t)i != cpustate->lenop1)
		cpustate->_Z = 0;
	else
		cpustate->_Z = 1;

	F7BEND(cpustate);
}


static uint32_t opSCHCUB(v60_state *cpustate) { return opSEARCHUB(cpustate, 1); }
static uint32_t opSCHCUH(v60_state *cpustate) { return opSEARCHUH(cpustate, 1); }
static uint32_t opSCHCDB(v60_state *cpustate) { return opSEARCHDB(cpustate, 1); }
static uint32_t opSCHCDH(v60_state *cpustate) { return opSEARCHDH(cpustate, 1); }
static uint32_t opSKPCUB(v60_state *cpustate) { return opSEARCHUB(cpustate, 0); }
static uint32_t opSKPCUH(v60_state *cpustate) { return opSEARCHUH(cpustate, 0); }
static uint32_t opSKPCDB(v60_state *cpustate) { return opSEARCHDB(cpustate, 0); }
static uint32_t opSKPCDH(v60_state *cpustate) { return opSEARCHDH(cpustate, 0); }

static uint32_t opCMPCB(v60_state *cpustate) { return opCMPSTRB(cpustate, 0, 0); }
static uint32_t opCMPCH(v60_state *cpustate) { return opCMPSTRH(cpustate, 0, 0); }
static uint32_t opCMPCFB(v60_state *cpustate) { return opCMPSTRB(cpustate, 1, 0); }
static uint32_t opCMPCFH(v60_state *cpustate) { return opCMPSTRH(cpustate, 1, 0); }
static uint32_t opCMPCSB(v60_state *cpustate) { return opCMPSTRB(cpustate, 0, 1); }
static uint32_t opCMPCSH(v60_state *cpustate) { return opCMPSTRH(cpustate, 0, 1); }

static uint32_t opMOVCUB(v60_state *cpustate) { return opMOVSTRUB(cpustate, 0, 0); }
static uint32_t opMOVCUH(v60_state *cpustate) { return opMOVSTRUH(cpustate, 0, 0); }
static uint32_t opMOVCFUB(v60_state *cpustate) { return opMOVSTRUB(cpustate, 1, 0); }
static uint32_t opMOVCFUH(v60_state *cpustate) { return opMOVSTRUH(cpustate, 1, 0); }
static uint32_t opMOVCSUB(v60_state *cpustate) { return opMOVSTRUB(cpustate, 0, 1); }
static uint32_t opMOVCSUH(v60_state *cpustate) { return opMOVSTRUH(cpustate, 0, 1); }

static uint32_t opMOVCDB(v60_state *cpustate) { return opMOVSTRDB(cpustate, 0, 0); }
static uint32_t opMOVCDH(v60_state *cpustate) { return opMOVSTRDH(cpustate, 0, 0); }
static uint32_t opMOVCFDB(v60_state *cpustate) { return opMOVSTRDB(cpustate, 1, 0); }
static uint32_t opMOVCFDH(v60_state *cpustate) { return opMOVSTRDH(cpustate, 1, 0); }

static uint32_t opEXTBFZ(v60_state *cpustate) /* TRUSTED */
{
	F7bDecodeFirstOperand(cpustate, BitReadAM, 11);

	F7BCREATEBITMASK(cpustate->lenop1);

	cpustate->modwritevalw = (cpustate->op1 >> cpustate->bamoffset) & cpustate->lenop1;

	F7bWriteSecondOperand(cpustate, 2);

	F7BEND(cpustate);
}

static uint32_t opEXTBFS(v60_state *cpustate) /* TRUSTED */
{
	F7bDecodeFirstOperand(cpustate, BitReadAM, 11);

	F7BCREATEBITMASK(cpustate->lenop1);

	cpustate->modwritevalw = (cpustate->op1 >> cpustate->bamoffset) & cpustate->lenop1;
	if (cpustate->modwritevalw & ((cpustate->lenop1 + 1) >> 1))
		cpustate->modwritevalw |= ~cpustate->lenop1;

	F7bWriteSecondOperand(cpustate, 2);

	F7BEND(cpustate);
}

static uint32_t opEXTBFL(v60_state *cpustate)
{
	uint32_t appw;

	F7bDecodeFirstOperand(cpustate, BitReadAM, 11);

	appw = cpustate->lenop1;
	F7BCREATEBITMASK(cpustate->lenop1);

	cpustate->modwritevalw = (cpustate->op1 >> cpustate->bamoffset) & cpustate->lenop1;
	cpustate->modwritevalw <<= 32 - appw;

	F7bWriteSecondOperand(cpustate, 2);

	F7BEND(cpustate);
}

static uint32_t opSCHBS(v60_state *cpustate, uint32_t bSearch1)
{
	uint32_t i, data;
	uint32_t offset;

	F7bDecodeFirstOperand(cpustate, BitReadAMAddress, 10);

	// Read first uint8_t
	cpustate->op1 += cpustate->bamoffset / 8;
	data = MemRead8(cpustate->program, cpustate->op1);
	offset = cpustate->bamoffset & 7;

	// Scan bitstring
	for (i = 0; i < cpustate->lenop1; i++)
	{
		// Update the work register
		cpustate->R28 = cpustate->op1;

		// There is a 0 / 1 at current offset?
		if ((bSearch1 && (data&(1 << offset))) ||
			(!bSearch1 && !(data&(1 << offset))))
			break;

		// Next bit please
		offset++;
		if (offset == 8)
		{
			// Next uint8_t please
			offset = 0;
			cpustate->op1++;
			data = MemRead8(cpustate->program, cpustate->op1);
		}
	}

	// Set zero if bit not found
	cpustate->_Z = (i == cpustate->lenop1);

	// Write to destination the final offset
	cpustate->modwritevalw = i;
	F7bWriteSecondOperand(cpustate, 2);

	F7BEND(cpustate);
}

static uint32_t opSCH0BSU(v60_state *cpustate) { return opSCHBS(cpustate, 0); }
static uint32_t opSCH1BSU(v60_state *cpustate) { return opSCHBS(cpustate, 1); }

static uint32_t opINSBFR(v60_state *cpustate)
{
	uint32_t appw;
	F7cDecodeOperands(cpustate, ReadAM, 2,BitReadAMAddress, 11);

	F7CCREATEBITMASK(cpustate->lenop1);

	cpustate->op2 += cpustate->bamoffset / 8;
	appw = MemRead32(cpustate->program, cpustate->op2);
	cpustate->bamoffset &= 7;

	appw &= ~(cpustate->lenop1 << cpustate->bamoffset);
	appw |=  (cpustate->lenop1 & cpustate->op1) << cpustate->bamoffset;

	MemWrite32(cpustate->program, cpustate->op2, appw);

	F7CEND(cpustate);
}

static uint32_t opINSBFL(v60_state *cpustate)
{
	uint32_t appw;
	F7cDecodeOperands(cpustate, ReadAM, 2,BitReadAMAddress, 11);

	cpustate->op1 >>= (32 - cpustate->lenop1);

	F7CCREATEBITMASK(cpustate->lenop1);

	cpustate->op2 += cpustate->bamoffset / 8;
	appw = MemRead32(cpustate->program, cpustate->op2);
	cpustate->bamoffset &= 7;

	appw &= ~(cpustate->lenop1 << cpustate->bamoffset);
	appw |=  (cpustate->lenop1 & cpustate->op1) << cpustate->bamoffset;

	MemWrite32(cpustate->program, cpustate->op2, appw);

	F7CEND(cpustate);
}

static uint32_t opMOVBSD(v60_state *cpustate)
{
	uint32_t i;
	uint8_t srcdata, dstdata;

	F7bDecodeOperands(cpustate, BitReadAMAddress, 10, BitReadAMAddress, 10);

//  if (cpustate->lenop1 != 1)
//  { int a = 1; }

	cpustate->bamoffset1 += cpustate->lenop1 - 1;
	cpustate->bamoffset2 += cpustate->lenop1 - 1;

	cpustate->op1 += cpustate->bamoffset1 / 8;
	cpustate->op2 += cpustate->bamoffset2 / 8;

	cpustate->bamoffset1 &= 7;
	cpustate->bamoffset2 &= 7;

	srcdata = MemRead8(cpustate->program, cpustate->op1);
	dstdata = MemRead8(cpustate->program, cpustate->op2);

	for (i = 0; i < cpustate->lenop1; i++)
	{
		// Update work registers
		cpustate->R28 = cpustate->op1;
		cpustate->R27 = cpustate->op2;

		dstdata &= ~(1 << cpustate->bamoffset2);
		dstdata |= ((srcdata >> cpustate->bamoffset1) & 1) << cpustate->bamoffset2;

		if (cpustate->bamoffset1 == 0)
		{
			cpustate->bamoffset1 = 8;
			cpustate->op1--;
			srcdata = MemRead8(cpustate->program, cpustate->op1);
		}
		if (cpustate->bamoffset2 == 0)
		{
			MemWrite8(cpustate->program, cpustate->op2, dstdata);
			cpustate->bamoffset2 = 8;
			cpustate->op2--;
			dstdata = MemRead8(cpustate->program, cpustate->op2);
		}

		cpustate->bamoffset1--;
		cpustate->bamoffset2--;
	}

	// Flush of the final data
	if (cpustate->bamoffset2 != 7)
		MemWrite8(cpustate->program, cpustate->op2, dstdata);

	F7BEND(cpustate);
}

static uint32_t opMOVBSU(v60_state *cpustate)
{
	uint32_t i;
	uint8_t srcdata, dstdata;

	F7bDecodeOperands(cpustate, BitReadAMAddress, 10, BitReadAMAddress, 10);

	cpustate->op1 += cpustate->bamoffset1 / 8;
	cpustate->op2 += cpustate->bamoffset2 / 8;

	cpustate->bamoffset1 &= 7;
	cpustate->bamoffset2 &= 7;

	srcdata = MemRead8(cpustate->program, cpustate->op1);
	dstdata = MemRead8(cpustate->program, cpustate->op2);

	for (i = 0; i < cpustate->lenop1; i++)
	{
		// Update work registers
		cpustate->R28 = cpustate->op1;
		cpustate->R27 = cpustate->op2;

		dstdata &= ~(1 << cpustate->bamoffset2);
		dstdata |= ((srcdata >> cpustate->bamoffset1) & 1) << cpustate->bamoffset2;

		cpustate->bamoffset1++;
		cpustate->bamoffset2++;
		if (cpustate->bamoffset1 == 8)
		{
			cpustate->bamoffset1 = 0;
			cpustate->op1++;
			srcdata = MemRead8(cpustate->program, cpustate->op1);
		}
		if (cpustate->bamoffset2 == 8)
		{
			MemWrite8(cpustate->program, cpustate->op2, dstdata);
			cpustate->bamoffset2 = 0;
			cpustate->op2++;
			dstdata = MemRead8(cpustate->program, cpustate->op2);
		}
	}

	// Flush of the final data
	if (cpustate->bamoffset2 != 0)
		MemWrite8(cpustate->program, cpustate->op2, dstdata);

	F7BEND(cpustate);
}

// RADM 0x20f4b8 holds the time left

static uint32_t opADDDC(v60_state *cpustate)
{
	uint8_t appb;
	uint8_t src, dst;

	F7cDecodeOperands(cpustate, ReadAM, 0, ReadAMAddress, 0);

	if (cpustate->lenop1 != 0)
	{
		logerror("ADDDC %x (pat: %x)\n", cpustate->op1, cpustate->lenop1);
	}

	F7CLOADOP2BYTE(cpustate, appb);

	src = (uint8_t)(cpustate->op1 >> 4) * 10 + (uint8_t)(cpustate->op1 & 0xF);
	dst = (appb >> 4) * 10 + (appb & 0xF);

	appb = src + dst + (cpustate->_CY?1:0);

	if (appb >= 100)
	{
		appb -= 100;
		cpustate->_CY = 1;
	}
	else
		cpustate->_CY = 0;

	// compute z flag:
	// cleared if result non-zero or carry generated
	// unchanged otherwise
	if (appb != 0 || cpustate->_CY)
		cpustate->_Z = 0;

	appb = ((appb / 10) << 4) | (appb % 10);

	F7CSTOREOP2BYTE(cpustate);
	F7CEND(cpustate);
}

static uint32_t opSUBDC(v60_state *cpustate)
{
	int8_t appb;
	uint32_t src, dst;

	F7cDecodeOperands(cpustate, ReadAM, 0, ReadAMAddress, 0);

	if (cpustate->lenop1 != 0)
	{
		logerror("SUBDC %x (pat: %x)\n", cpustate->op1, cpustate->lenop1);
	}

	F7CLOADOP2BYTE(cpustate, appb);

	src = (uint32_t)(cpustate->op1 >> 4) * 10 + (uint32_t)(cpustate->op1 & 0xF);
	dst = ((appb & 0xF0) >> 4) * 10 + (appb & 0xF);

	// Note that this APPB must be SIGNED!
	appb = (int32_t)dst - (int32_t)src - (cpustate->_CY?1:0);

	if (appb < 0)
	{
		appb += 100;
		cpustate->_CY = 1;
	}
	else
		cpustate->_CY = 0;

	// compute z flag:
	// cleared if result non-zero or carry generated
	// unchanged otherwise
	if (appb != 0 || cpustate->_CY)
		cpustate->_Z = 0;

	appb = ((appb / 10) << 4) | (appb % 10);

	F7CSTOREOP2BYTE(cpustate);
	F7CEND(cpustate);
}

static uint32_t opSUBRDC(v60_state *cpustate)
{
	int8_t appb;
	uint32_t src, dst;

	F7cDecodeOperands(cpustate, ReadAM, 0, ReadAMAddress, 0);

	if (cpustate->lenop1 != 0)
	{
		logerror("SUBRDC %x (pat: %x)\n", cpustate->op1, cpustate->lenop1);
	}

	F7CLOADOP2BYTE(cpustate, appb);

	src = (uint32_t)(cpustate->op1 >> 4) * 10 + (uint32_t)(cpustate->op1 & 0xF);
	dst = ((appb & 0xF0) >> 4) * 10 + (appb & 0xF);

	// Note that this APPB must be SIGNED!
	appb = (int32_t)src - (int32_t)dst - (cpustate->_CY?1:0);

	if (appb < 0)
	{
		appb += 100;
		cpustate->_CY = 1;
	}
	else
		cpustate->_CY = 0;

	// compute z flag:
	// cleared if result non-zero or carry generated
	// unchanged otherwise
	if (appb != 0 || cpustate->_CY)
		cpustate->_Z = 0;

	appb = ((appb / 10) << 4) | (appb % 10);

	F7CSTOREOP2BYTE(cpustate);
	F7CEND(cpustate);
}

static uint32_t opCVTDPZ(v60_state *cpustate)
{
	uint16_t apph;

	F7cDecodeOperands(cpustate, ReadAM, 0, ReadAMAddress, 1);

	apph = (uint16_t)(((cpustate->op1 >> 4) & 0xF) | ((cpustate->op1 & 0xF) << 8));
	apph |= (cpustate->lenop1);
	apph |= (cpustate->lenop1 << 8);

	// Z flag is unchanged if src is zero, cleared otherwise
	if (cpustate->op1 != 0) cpustate->_Z = 0;

	F7CSTOREOP2HALF(cpustate);
	F7CEND(cpustate);
}

static uint32_t opCVTDZP(v60_state *cpustate)
{
	uint8_t appb;
	F7cDecodeOperands(cpustate, ReadAM, 1, ReadAMAddress, 0);

	if ((cpustate->op1 & 0xF0) != (cpustate->lenop1 & 0xF0) || ((cpustate->op1 >> 8) & 0xF0) != (cpustate->lenop1 & 0xF0))
	{
		// Decimal exception
		logerror("CVTD.ZP Decimal exception #1!\n");
	}

	if ((cpustate->op1 & 0xF) > 9 || ((cpustate->op1 >> 8) & 0xF) > 9)
	{
		// Decimal exception
		logerror("CVTD.ZP Decimal exception #2!\n");
	}

	appb = (uint8_t)(((cpustate->op1 >> 8) & 0xF) | ((cpustate->op1 & 0xF) << 4));
	if (appb != 0) cpustate->_Z = 0;

	F7CSTOREOP2BYTE(cpustate);
	F7CEND(cpustate);
}

static uint32_t op58UNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 58 opcode at cpustate->PC: /%06x", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static uint32_t op5AUNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 5A opcode at cpustate->PC: /%06x", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static uint32_t op5BUNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 5B opcode at cpustate->PC: /%06x", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static uint32_t op5DUNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 5D opcode at cpustate->PC: /%06x", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static uint32_t op59UNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 59 opcode at cpustate->PC: /%06x", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static uint32_t (*const Op59Table[32])(v60_state *) =
{
	opADDDC,
	opSUBDC,
	opSUBRDC,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	opCVTDPZ,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	opCVTDZP,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED,
	op59UNHANDLED
};


static uint32_t (*const Op5BTable[32])(v60_state *) =
{
	opSCH0BSU,
	op5BUNHANDLED,
	opSCH1BSU,
    	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	opMOVBSU,
	opMOVBSD,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED,
	op5BUNHANDLED
};


static uint32_t (*const Op5DTable[32])(v60_state *) =
{
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	opEXTBFS,
	opEXTBFZ,
	opEXTBFL,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	opINSBFR,
	opINSBFL,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED,
	op5DUNHANDLED
};

static uint32_t (*const Op58Table[32])(v60_state *) =
{
	opCMPCB,
	opCMPCFB,
	opCMPCSB,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	opMOVCUB,
	opMOVCDB,
	opMOVCFUB,
	opMOVCFDB,
	opMOVCSUB,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	opSCHCUB,
	opSCHCDB,
	opSKPCUB,
	opSKPCDB,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED,
	op58UNHANDLED
};

static uint32_t (*const Op5ATable[32])(v60_state *) =
{
	opCMPCH,
	opCMPCFH,
	opCMPCSH,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	opMOVCUH,
	opMOVCDH,
	opMOVCFUH,
	opMOVCFDH,
	opMOVCSUH,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	opSCHCUH,
	opSCHCDH,
	opSKPCUH,
	opSKPCDH,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED,
	op5AUNHANDLED
};

static uint32_t op58(v60_state *cpustate)
{
	cpustate->subop = OpRead8(cpustate->program, cpustate->PC + 1);

	return Op58Table[cpustate->subop & 0x1F](cpustate);
}

static uint32_t op5A(v60_state *cpustate)
{
	cpustate->subop = OpRead8(cpustate->program, cpustate->PC + 1);

	return Op5ATable[cpustate->subop & 0x1F](cpustate);
}

static uint32_t op5B(v60_state *cpustate)
{
	cpustate->subop = OpRead8(cpustate->program, cpustate->PC + 1);

	return Op5BTable[cpustate->subop & 0x1F](cpustate);
}

static uint32_t op5D(v60_state *cpustate)
{
	cpustate->subop = OpRead8(cpustate->program, cpustate->PC + 1);

	return Op5DTable[cpustate->subop & 0x1F](cpustate);
}

static uint32_t op59(v60_state *cpustate)
{
	cpustate->subop = OpRead8(cpustate->program, cpustate->PC + 1);

	return Op59Table[cpustate->subop & 0x1F](cpustate);
}
