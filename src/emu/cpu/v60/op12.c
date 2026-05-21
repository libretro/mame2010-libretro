/*
 * MUL* and MULU* do not set OV correctly
 * DIVX: the second operand should be treated as dword instead of word
 * GETATE, GETPTE and GETRA should not be used
 * UPDPSW: cpustate->_CY and cpustate->_OV must be cleared or unchanged? I suppose
 *   cleared, like TEST being done on the mask operand.
 * MOVT: I cannot understand exactly what happens to the result
 *   when an overflow occurs
 *
 * Unimplemented opcodes:
 * ROTC, UPDATE, UPDPTE
 */


/*
 *  Macro to access data in operands decoded with ReadAMAddress(cpustate)
 */

#define F12LOADOPBYTE(cs, num)							\
	if ((cs)->flag##num)								\
		appb = (uint8_t)(cs)->reg[(cs)->op##num];			\
	else												\
		appb = MemRead8((cs)->program, (cs)->op##num);

#define F12LOADOPHALF(cs, num)							\
	if ((cs)->flag##num)								\
		apph = (uint16_t)(cs)->reg[(cs)->op##num];		\
	else												\
		apph = MemRead16((cs)->program, (cs)->op##num);

#define F12LOADOPWORD(cs, num)							\
	if ((cs)->flag##num)								\
		appw = (cs)->reg[(cs)->op##num];				\
	else												\
		appw = MemRead32((cs)->program,(cs)->op##num);

#define F12STOREOPBYTE(cs, num)							\
	if ((cs)->flag##num)								\
		SETREG8((cs)->reg[(cs)->op##num], appb);		\
	else												\
		MemWrite8((cs)->program, (cs)->op##num, appb);

#define F12STOREOPHALF(cs, num)							\
	if ((cs)->flag##num)								\
		SETREG16((cs)->reg[(cs)->op##num], apph);		\
	else												\
		MemWrite16((cs)->program, (cs)->op##num, apph);

#define F12STOREOPWORD(cs, num)							\
	if ((cs)->flag##num)								\
		(cs)->reg[(cs)->op##num] = appw;				\
	else												\
		MemWrite32((cs)->program, (cs)->op##num, appw);

#define F12LOADOP1BYTE(cs)  F12LOADOPBYTE(cs, 1)
#define F12LOADOP1HALF(cs)  F12LOADOPHALF(cs, 1)
#define F12LOADOP1WORD(cs)  F12LOADOPWORD(cs, 1)

#define F12LOADOP2BYTE(cs)  F12LOADOPBYTE(cs, 2)
#define F12LOADOP2HALF(cs)  F12LOADOPHALF(cs, 2)
#define F12LOADOP2WORD(cs)  F12LOADOPWORD(cs, 2)

#define F12STOREOP1BYTE(cs)  F12STOREOPBYTE(cs, 1)
#define F12STOREOP1HALF(cs)  F12STOREOPHALF(cs, 1)
#define F12STOREOP1WORD(cs)  F12STOREOPWORD(cs, 1)

#define F12STOREOP2BYTE(cs)  F12STOREOPBYTE(cs, 2)
#define F12STOREOP2HALF(cs)  F12STOREOPHALF(cs, 2)
#define F12STOREOP2WORD(cs)  F12STOREOPWORD(cs, 2)

#define F12END(cs)									\
	return (cs)->amlength1 + (cs)->amlength2 + 2;


// Decode the first operand of the instruction and prepare
// writing to the second operand.
static void F12DecodeFirstOperand(v60_state *cpustate, uint32_t (*DecodeOp1)(v60_state *), uint8_t dim1)
{
	cpustate->instflags = OpRead8(cpustate->program, cpustate->PC + 1);

	// Check if F1 or F2
	if (cpustate->instflags & 0x80)
	{
		cpustate->moddim = dim1;
		cpustate->modm = cpustate->instflags & 0x40;
		cpustate->modadd = cpustate->PC + 2;
		cpustate->amlength1 = DecodeOp1(cpustate);
		cpustate->op1 = cpustate->amout;
		cpustate->flag1 = cpustate->amflag;
	}
	else
	{
		// Check D flag
		if (cpustate->instflags & 0x20)
		{
			cpustate->moddim = dim1;
			cpustate->modm = cpustate->instflags & 0x40;
			cpustate->modadd = cpustate->PC + 2;
			cpustate->amlength1 = DecodeOp1(cpustate);
			cpustate->op1 = cpustate->amout;
			cpustate->flag1 = cpustate->amflag;
		}
		else
		{
			if (DecodeOp1 == ReadAM)
			{
				switch (dim1)
				{
				case 0:
					cpustate->op1 = (uint8_t)cpustate->reg[cpustate->instflags & 0x1F];
					break;
				case 1:
					cpustate->op1 = (uint16_t)cpustate->reg[cpustate->instflags & 0x1F];
					break;
				case 2:
					cpustate->op1 = cpustate->reg[cpustate->instflags & 0x1F];
					break;
				}

				cpustate->flag1 = 0;
			}
			else
			{
				cpustate->flag1 = 1;
				cpustate->op1 = cpustate->instflags & 0x1F;
			}

			cpustate->amlength1 = 0;
		}
	}
}

static void F12WriteSecondOperand(v60_state *cpustate, uint8_t dim2)
{
	cpustate->moddim = dim2;

	// Check if F1 or F2
	if (cpustate->instflags & 0x80)
	{
		cpustate->modm = cpustate->instflags & 0x20;
		cpustate->modadd = cpustate->PC + 2 + cpustate->amlength1;
		cpustate->moddim = dim2;
		cpustate->amlength2 = WriteAM(cpustate);
	}
	else
	{
		// Check D flag
		if (cpustate->instflags & 0x20)
		{
			switch (dim2)
			{
			case 0:
				SETREG8(cpustate->reg[cpustate->instflags & 0x1F], cpustate->modwritevalb);
				break;
			case 1:
				SETREG16(cpustate->reg[cpustate->instflags & 0x1F], cpustate->modwritevalh);
				break;
			case 2:
				cpustate->reg[cpustate->instflags & 0x1F] = cpustate->modwritevalw;
				break;
			}

			cpustate->amlength2 = 0;
		}
		else
		{
			cpustate->modm = cpustate->instflags & 0x40;
			cpustate->modadd = cpustate->PC + 2;
			cpustate->moddim = dim2;
			cpustate->amlength2 = WriteAM(cpustate);
		}
	}
}



// Decode both format 1 / 2 operands
static void F12DecodeOperands(v60_state *cpustate, uint32_t (*DecodeOp1)(v60_state *), uint8_t dim1, uint32_t (*DecodeOp2)(v60_state *), uint8_t dim2)
{
	uint8_t _if12 = OpRead8(cpustate->program, cpustate->PC + 1);

	// Check if F1 or F2
	if (_if12 & 0x80)
	{
		cpustate->moddim = dim1;
		cpustate->modm = _if12 & 0x40;
		cpustate->modadd = cpustate->PC + 2;
		cpustate->amlength1 = DecodeOp1(cpustate);
		cpustate->op1 = cpustate->amout;
		cpustate->flag1 = cpustate->amflag;

		cpustate->moddim = dim2;
		cpustate->modm = _if12 & 0x20;
		cpustate->modadd = cpustate->PC + 2 + cpustate->amlength1;
		cpustate->amlength2 = DecodeOp2(cpustate);
		cpustate->op2 = cpustate->amout;
		cpustate->flag2 = cpustate->amflag;
	}
	else
	{
		// Check D flag
		if (_if12 & 0x20)
		{
			if (DecodeOp2 == ReadAMAddress)
			{
				cpustate->op2 = _if12 & 0x1F;
				cpustate->flag2 = 1;
			}
			else
			{
				switch (dim2)
				{
				case 0:
					cpustate->op2 = (uint8_t)cpustate->reg[_if12 & 0x1F];
					break;
				case 1:
					cpustate->op2 = (uint16_t)cpustate->reg[_if12 & 0x1F];
					break;
				case 2:
					cpustate->op2 = cpustate->reg[_if12 & 0x1F];
					break;
				}
			}

			cpustate->amlength2 = 0;

			cpustate->moddim = dim1;
			cpustate->modm = _if12 & 0x40;
			cpustate->modadd = cpustate->PC + 2;
			cpustate->amlength1 = DecodeOp1(cpustate);
			cpustate->op1 = cpustate->amout;
			cpustate->flag1 = cpustate->amflag;
		}
		else
		{
			if (DecodeOp1 == ReadAMAddress)
			{
				cpustate->op1 = _if12 & 0x1F;
				cpustate->flag1 = 1;
			}
			else
			{
				switch (dim1)
				{
				case 0:
					cpustate->op1 = (uint8_t)cpustate->reg[_if12 & 0x1F];
					break;
				case 1:
					cpustate->op1 = (uint16_t)cpustate->reg[_if12 & 0x1F];
					break;
				case 2:
					cpustate->op1 = cpustate->reg[_if12 & 0x1F];
					break;
				}
			}
			cpustate->amlength1 = 0;

			cpustate->moddim = dim2;
			cpustate->modm = _if12 & 0x40;
			cpustate->modadd = cpustate->PC + 2 + cpustate->amlength1;
			cpustate->amlength2 = DecodeOp2(cpustate);
			cpustate->op2 = cpustate->amout;
			cpustate->flag2 = cpustate->amflag;
		}
	}
}

static uint32_t opADDB(v60_state *cpustate) /* TRUSTED (C too!)*/
{
	uint8_t appb;
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	ADDB(appb, (uint8_t)cpustate->op1);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opADDH(v60_state *cpustate) /* TRUSTED (C too!)*/
{
	uint16_t apph;
	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	ADDW(apph, (uint16_t)cpustate->op1);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opADDW(v60_state *cpustate) /* TRUSTED (C too!) */
{
	uint32_t appw;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	ADDL(appw, (uint32_t)cpustate->op1);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opADDCB(v60_state *cpustate)
{
	uint8_t appb, temp;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	temp = ((uint8_t)cpustate->op1 + (cpustate->_CY?1:0));
	ADDB(appb, temp);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opADDCH(v60_state *cpustate)
{
	uint16_t apph, temp;

	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	temp = ((uint16_t)cpustate->op1 + (cpustate->_CY?1:0));
	ADDW(apph, temp);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opADDCW(v60_state *cpustate)
{
	uint32_t appw, temp;

	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	temp = cpustate->op1 + (cpustate->_CY?1:0);
	ADDL(appw, temp);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opANDB(v60_state *cpustate) /* TRUSTED */
{
	uint8_t appb;
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	appb &= cpustate->op1;
	cpustate->_OV = 0;
	cpustate->_S = ((appb & 0x80) != 0);
	cpustate->_Z = (appb == 0);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opANDH(v60_state *cpustate) /* TRUSTED */
{
	uint16_t apph;
	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	apph &= cpustate->op1;
	cpustate->_OV = 0;
	cpustate->_S = ((apph & 0x8000) != 0);
	cpustate->_Z = (apph == 0);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opANDW(v60_state *cpustate) /* TRUSTED */
{
	uint32_t appw;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	appw &= cpustate->op1;
	cpustate->_OV = 0;
	cpustate->_S = ((appw & 0x80000000) != 0);
	cpustate->_Z = (appw == 0);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opCALL(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeOperands(cpustate, ReadAMAddress, 0,ReadAMAddress, 2);

	cpustate->SP -= 4;
	MemWrite32(cpustate->program, cpustate->SP, cpustate->AP);
	cpustate->AP = cpustate->op2;

	cpustate->SP -= 4;
	MemWrite32(cpustate->program, cpustate->SP, cpustate->PC + cpustate->amlength1 + cpustate->amlength2 + 2);
	cpustate->PC = cpustate->op1;

	return 0;
}

static uint32_t opCHKAR(v60_state *cpustate)
{
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAM, 0);

	// No MMU and memory permissions yet @@@
	cpustate->_Z = 1;
	cpustate->_CY = 0;
	cpustate->_S = 0;

	F12END(cpustate);
}

static uint32_t opCHKAW(v60_state *cpustate)
{
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAM, 0);

	// No MMU and memory permissions yet @@@
	cpustate->_Z = 1;
	cpustate->_CY = 0;
	cpustate->_S = 0;

	F12END(cpustate);
}

static uint32_t opCHKAE(v60_state *cpustate)
{
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAM, 0);

	// No MMU and memory permissions yet @@@
	cpustate->_Z = 1;
	cpustate->_CY = 0;
	cpustate->_S = 0;

	F12END(cpustate);
}

static uint32_t opCHLVL(v60_state *cpustate)
{
	uint32_t oldPSW;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAM, 0);

	if (cpustate->op1 > 3)
	{
		fatalerror("Illegal data field on opCHLVL, cpustate->PC=%x", cpustate->PC);
	}

	oldPSW = v60_update_psw_for_exception(cpustate, 0, cpustate->op1);

	cpustate->SP -= 4;
	MemWrite32(cpustate->program, cpustate->SP, cpustate->op2);

	cpustate->SP -= 4;
	MemWrite32(cpustate->program, cpustate->SP, EXCEPTION_CODE_AND_SIZE(0x1800 + cpustate->op1 * 0x100, 8));

	cpustate->SP -= 4;
	MemWrite32(cpustate->program, cpustate->SP, oldPSW);

	cpustate->SP -= 4;
	MemWrite32(cpustate->program, cpustate->SP, cpustate->PC + cpustate->amlength1 + cpustate->amlength2 + 2);

	cpustate->PC = GETINTVECT(cpustate, 24 + cpustate->op1);

	return 0;
}

static uint32_t opCLR1(v60_state *cpustate) /* TRUSTED */
{
	uint32_t appw;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	cpustate->_CY = ((appw & (1 << cpustate->op1)) != 0);
	cpustate->_Z = !(cpustate->_CY);

	appw &= ~(1 << cpustate->op1);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opCMPB(v60_state *cpustate) /* TRUSTED (C too!) */
{
	uint8_t appb;
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAM, 0);

	appb = (uint8_t)cpustate->op2;
	SUBB(appb, (uint8_t)cpustate->op1);

	F12END(cpustate);
}

static uint32_t opCMPH(v60_state *cpustate) /* TRUSTED (C too!) */
{
	uint16_t apph;
	F12DecodeOperands(cpustate, ReadAM, 1,ReadAM, 1);

	apph = (uint16_t)cpustate->op2;
	SUBW(apph, (uint16_t)cpustate->op1);

	F12END(cpustate);
}


static uint32_t opCMPW(v60_state *cpustate) /* TRUSTED (C too!)*/
{
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAM, 2);

	SUBL(cpustate->op2, (uint32_t)cpustate->op1);

	F12END(cpustate);
}

static uint32_t opDIVB(v60_state *cpustate) /* TRUSTED */
{
	uint8_t appb;
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	cpustate->_OV = ((appb == 0x80) && (cpustate->op1 == 0xFF));
	if (cpustate->op1 && !cpustate->_OV)
		appb= (int8_t)appb / (int8_t)cpustate->op1;
	cpustate->_Z = (appb == 0);
	cpustate->_S = ((appb & 0x80) != 0);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opDIVH(v60_state *cpustate) /* TRUSTED */
{
	uint16_t apph;
	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	cpustate->_OV = ((apph == 0x8000) && (cpustate->op1 == 0xFFFF));
	if (cpustate->op1 && !cpustate->_OV)
		apph = (int16_t)apph / (int16_t)cpustate->op1;
	cpustate->_Z = (apph == 0);
	cpustate->_S = ((apph & 0x8000) != 0);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opDIVW(v60_state *cpustate) /* TRUSTED */
{
	uint32_t appw;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	cpustate->_OV = ((appw == 0x80000000) && (cpustate->op1 == 0xFFFFFFFF));
	if (cpustate->op1 && !cpustate->_OV)
		appw = (int32_t)appw / (int32_t)cpustate->op1;
	cpustate->_Z = (appw == 0);
	cpustate->_S = ((appw & 0x80000000) != 0);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opDIVX(v60_state *cpustate)
{
	uint32_t a, b;
	int64_t dv;

	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 3);

	if (cpustate->flag2)
	{
		a = cpustate->reg[cpustate->op2 & 0x1F];
		b = cpustate->reg[(cpustate->op2 & 0x1F) + 1];
	}
	else
	{
		a = MemRead32(cpustate->program, cpustate->op2);
		b = MemRead32(cpustate->program, cpustate->op2 + 4);
	}

	dv = ((uint64_t)b << 32) | ((uint64_t)a);

	a = dv / (int64_t)((int32_t)cpustate->op1);
	b = dv % (int64_t)((int32_t)cpustate->op1);

	cpustate->_S = ((a & 0x80000000) != 0);
	cpustate->_Z = (a == 0);

	if (cpustate->flag2)
	{
		cpustate->reg[cpustate->op2 & 0x1F] = a;
		cpustate->reg[(cpustate->op2 & 0x1F) + 1] = b;
	}
	else
	{
		MemWrite32(cpustate->program, cpustate->op2, a);
		MemWrite32(cpustate->program, cpustate->op2 + 4, b);
	}

	F12END(cpustate);
}

static uint32_t opDIVUX(v60_state *cpustate)
{
	uint32_t a, b;
	uint64_t dv;

	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 3);

	if (cpustate->flag2)
	{
		a = cpustate->reg[cpustate->op2 & 0x1F];
		b = cpustate->reg[(cpustate->op2 & 0x1F) + 1];
	}
	else
	{
		a = MemRead32(cpustate->program, cpustate->op2);
		b = MemRead32(cpustate->program, cpustate->op2 + 4);
	}

	dv = (uint64_t)(((uint64_t)b << 32) | (uint64_t)a);
	a = (uint32_t)(dv / (uint64_t)cpustate->op1);
	b = (uint32_t)(dv % (uint64_t)cpustate->op1);

	cpustate->_S = ((a & 0x80000000) != 0);
	cpustate->_Z = (a == 0);

	if (cpustate->flag2)
	{
		cpustate->reg[cpustate->op2 & 0x1F] = a;
		cpustate->reg[(cpustate->op2 & 0x1F) + 1] = b;
	}
	else
	{
		MemWrite32(cpustate->program, cpustate->op2, a);
		MemWrite32(cpustate->program, cpustate->op2 + 4, b);
	}

	F12END(cpustate);
}


static uint32_t opDIVUB(v60_state *cpustate) /* TRUSTED */
{
	uint8_t appb;
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	cpustate->_OV = 0;
	if (cpustate->op1)	appb /= (uint8_t)cpustate->op1;
	cpustate->_Z = (appb == 0);
	cpustate->_S = ((appb & 0x80) != 0);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opDIVUH(v60_state *cpustate) /* TRUSTED */
{
	uint16_t apph;
	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	cpustate->_OV = 0;
	if (cpustate->op1)	apph /= (uint16_t)cpustate->op1;
	cpustate->_Z = (apph == 0);
	cpustate->_S = ((apph & 0x8000) != 0);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opDIVUW(v60_state *cpustate) /* TRUSTED */
{
	uint32_t appw;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	cpustate->_OV = 0;
	if (cpustate->op1)	appw /= cpustate->op1;
	cpustate->_Z = (appw == 0);
	cpustate->_S = ((appw & 0x80000000) != 0);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opINB(v60_state *cpustate)
{
	F12DecodeFirstOperand(cpustate, ReadAMAddress, 0);
	cpustate->modwritevalb = MemRead8(cpustate->io, cpustate->op1);

	if ( cpustate->stall_io )
	{
		cpustate->stall_io = 0;
		return 0;
	}

	F12WriteSecondOperand(cpustate, 0);
	F12END(cpustate);
}

static uint32_t opINH(v60_state *cpustate)
{
	F12DecodeFirstOperand(cpustate, ReadAMAddress, 1);
	cpustate->modwritevalh = MemRead16(cpustate->io, cpustate->op1);

	if ( cpustate->stall_io )
	{
		cpustate->stall_io = 0;
		return 0;
	}

	F12WriteSecondOperand(cpustate, 1);
	F12END(cpustate);
}

static uint32_t opINW(v60_state *cpustate)
{
	F12DecodeFirstOperand(cpustate, ReadAMAddress, 2);
	cpustate->modwritevalw = MemRead32(cpustate->io, cpustate->op1);

	if ( cpustate->stall_io )
	{
		cpustate->stall_io = 0;
		return 0;
	}

	F12WriteSecondOperand(cpustate, 2);
	F12END(cpustate);
}

static uint32_t opLDPR(v60_state *cpustate)
{
	F12DecodeOperands(cpustate, ReadAMAddress, 2,ReadAM, 2);
	if (cpustate->op2 >= 0 && cpustate->op2 <= 28)
	{
	  if (cpustate->flag1 &&(!(OpRead8(cpustate->program, cpustate->PC + 1)&0x80 && OpRead8(cpustate->program, cpustate->PC + 2) == 0xf4 ) ))
			cpustate->reg[cpustate->op2 + 36] = cpustate->reg[cpustate->op1];
		else
			cpustate->reg[cpustate->op2 + 36] = cpustate->op1;
	}
	else
	{
		fatalerror("Invalid operand on LDPR cpustate->PC=%x", cpustate->PC);
	}
	F12END(cpustate);
}

static uint32_t opLDTASK(v60_state *cpustate)
{
	int i;
	F12DecodeOperands(cpustate, ReadAMAddress, 2,ReadAM, 2);

	v60WritePSW(cpustate, v60ReadPSW(cpustate) & 0xefffffff);

	cpustate->TR = cpustate->op2;

	cpustate->TKCW = MemRead32(cpustate->program, cpustate->op2);
	cpustate->op2 += 4;
	if(cpustate->SYCW & 0x100) {
		cpustate->L0SP = MemRead32(cpustate->program, cpustate->op2);
		cpustate->op2 += 4;
	}
	if(cpustate->SYCW & 0x200) {
		cpustate->L1SP = MemRead32(cpustate->program, cpustate->op2);
		cpustate->op2 += 4;
	}
	if(cpustate->SYCW & 0x400) {
		cpustate->L2SP = MemRead32(cpustate->program, cpustate->op2);
		cpustate->op2 += 4;
	}
	if(cpustate->SYCW & 0x800) {
		cpustate->L3SP = MemRead32(cpustate->program, cpustate->op2);
		cpustate->op2 += 4;
	}

	v60ReloadStack(cpustate);

	// 31 registers supported, _not_ 32
	for(i = 0; i < 31; i++)
		if(cpustate->op1 & (1 << i)) {
			cpustate->reg[i] = MemRead32(cpustate->program, cpustate->op2);
			cpustate->op2 += 4;
		}

	// #### Ignore the virtual addressing crap.

	F12END(cpustate);
}

static uint32_t opMOVD(v60_state *cpustate) /* TRUSTED */
{
	uint32_t a, b;

	F12DecodeOperands(cpustate, ReadAMAddress, 3,ReadAMAddress, 3);

	if (cpustate->flag1)
	{
		a = cpustate->reg[cpustate->op1 & 0x1F];
		b = cpustate->reg[(cpustate->op1 & 0x1F) + 1];
	}
	else
	{
		a = MemRead32(cpustate->program, cpustate->op1);
		b = MemRead32(cpustate->program, cpustate->op1 + 4);
	}

	if (cpustate->flag2)
	{
		cpustate->reg[cpustate->op2 & 0x1F] = a;
		cpustate->reg[(cpustate->op2 & 0x1F) + 1] = b;
	}
	else
	{
		MemWrite32(cpustate->program, cpustate->op2, a);
		MemWrite32(cpustate->program, cpustate->op2 + 4, b);
	}

	F12END(cpustate);
}

static uint32_t opMOVB(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAM, 0);
	cpustate->modwritevalb = (uint8_t)cpustate->op1;
	F12WriteSecondOperand(cpustate, 0);
	F12END(cpustate);
}

static uint32_t opMOVH(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAM, 1);
	cpustate->modwritevalh = (uint16_t)cpustate->op1;
	F12WriteSecondOperand(cpustate, 1);
	F12END(cpustate);
}

static uint32_t opMOVW(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAM, 2);
	cpustate->modwritevalw = cpustate->op1;
	F12WriteSecondOperand(cpustate, 2);
	F12END(cpustate);
}

static uint32_t opMOVEAB(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAMAddress, 0);
	cpustate->modwritevalw = cpustate->op1;
	F12WriteSecondOperand(cpustate, 2);
	F12END(cpustate);
}

static uint32_t opMOVEAH(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAMAddress, 1);
	cpustate->modwritevalw = cpustate->op1;
	F12WriteSecondOperand(cpustate, 2);
	F12END(cpustate);
}

static uint32_t opMOVEAW(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAMAddress, 2);
	cpustate->modwritevalw = cpustate->op1;
	F12WriteSecondOperand(cpustate, 2);
	F12END(cpustate);
}

static uint32_t opMOVSBH(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAM, 0);
	cpustate->modwritevalh = (int8_t)(cpustate->op1 & 0xFF);
	F12WriteSecondOperand(cpustate, 1);
	F12END(cpustate);
}

static uint32_t opMOVSBW(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAM, 0);
	cpustate->modwritevalw = (int8_t)(cpustate->op1 & 0xFF);
	F12WriteSecondOperand(cpustate, 2);
	F12END(cpustate);
}

static uint32_t opMOVSHW(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAM, 1);
	cpustate->modwritevalw = (int16_t)(cpustate->op1 & 0xFFFF);
	F12WriteSecondOperand(cpustate, 2);
	F12END(cpustate);
}

static uint32_t opMOVTHB(v60_state *cpustate)
{
	F12DecodeFirstOperand(cpustate, ReadAM, 1);
	cpustate->modwritevalb = (uint8_t)(cpustate->op1 & 0xFF);

	// Check for overflow: the truncated bits must match the sign
	//  of the result, otherwise overflow
	if (((cpustate->modwritevalb & 0x80) == 0x80 && ((cpustate->op1 & 0xFF00) == 0xFF00)) ||
		  ((cpustate->modwritevalb & 0x80) == 0 && ((cpustate->op1 & 0xFF00) == 0x0000)))
		cpustate->_OV = 0;
	else
		cpustate->_OV = 1;

	F12WriteSecondOperand(cpustate, 0);
	F12END(cpustate);
}

static uint32_t opMOVTWB(v60_state *cpustate)
{
	F12DecodeFirstOperand(cpustate, ReadAM, 2);
	cpustate->modwritevalb = (uint8_t)(cpustate->op1 & 0xFF);

	// Check for overflow: the truncated bits must match the sign
	//  of the result, otherwise overflow
	if (((cpustate->modwritevalb & 0x80) == 0x80 && ((cpustate->op1 & 0xFFFFFF00) == 0xFFFFFF00)) ||
		  ((cpustate->modwritevalb & 0x80) == 0 && ((cpustate->op1 & 0xFFFFFF00) == 0x00000000)))
		cpustate->_OV = 0;
	else
		cpustate->_OV = 1;

	F12WriteSecondOperand(cpustate, 0);
	F12END(cpustate);
}

static uint32_t opMOVTWH(v60_state *cpustate)
{
	F12DecodeFirstOperand(cpustate, ReadAM, 2);
	cpustate->modwritevalh = (uint16_t)(cpustate->op1 & 0xFFFF);

	// Check for overflow: the truncated bits must match the sign
	//  of the result, otherwise overflow
	if (((cpustate->modwritevalh & 0x8000) == 0x8000 && ((cpustate->op1 & 0xFFFF0000) == 0xFFFF0000)) ||
		  ((cpustate->modwritevalh & 0x8000) == 0 && ((cpustate->op1 & 0xFFFF0000) == 0x00000000)))
		cpustate->_OV = 0;
	else
		cpustate->_OV = 1;

	F12WriteSecondOperand(cpustate, 1);
	F12END(cpustate);
}


static uint32_t opMOVZBH(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAM, 0);
	cpustate->modwritevalh = (uint16_t)cpustate->op1;
	F12WriteSecondOperand(cpustate, 1);
	F12END(cpustate);
}

static uint32_t opMOVZBW(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAM, 0);
	cpustate->modwritevalw = cpustate->op1;
	F12WriteSecondOperand(cpustate, 2);
	F12END(cpustate);
}

static uint32_t opMOVZHW(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAM, 1);
	cpustate->modwritevalw = cpustate->op1;
	F12WriteSecondOperand(cpustate, 2);
	F12END(cpustate);
}

static uint32_t opMULB(v60_state *cpustate)
{
	uint8_t appb;
	uint32_t tmp;
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	// @@@ OV not set!!
	tmp = (int8_t)appb * (int32_t)(int8_t)cpustate->op1;
	appb = tmp;
	cpustate->_Z = (appb == 0);
	cpustate->_S = ((appb & 0x80) != 0);
	cpustate->_OV = ((tmp >> 8) != 0);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opMULH(v60_state *cpustate)
{
	uint16_t apph;
	uint32_t tmp;
	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	// @@@ OV not set!!
	tmp = (int16_t)apph * (int32_t)(int16_t)cpustate->op1;
	apph = tmp;
	cpustate->_Z = (apph == 0);
	cpustate->_S = ((apph & 0x8000) != 0);
	cpustate->_OV = ((tmp >> 16) != 0);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opMULW(v60_state *cpustate)
{
	uint32_t appw;
	uint64_t tmp;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	// @@@ OV not set!!
	tmp = (int32_t)appw * (int64_t)(int32_t)cpustate->op1;
	appw = tmp;
	cpustate->_Z = (appw == 0);
	cpustate->_S = ((appw & 0x80000000) != 0);
	cpustate->_OV = ((tmp >> 32) != 0);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opMULUB(v60_state *cpustate)
{
	uint8_t appb;
	uint32_t tmp;
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	// @@@ OV not set!!
	tmp = appb * (uint8_t)cpustate->op1;
	appb = tmp;
	cpustate->_Z = (appb == 0);
	cpustate->_S = ((appb & 0x80) != 0);
	cpustate->_OV = ((tmp >> 8) != 0);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opMULUH(v60_state *cpustate)
{
	uint16_t apph;
	uint32_t tmp;
	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	// @@@ OV not set!!
	tmp = apph * (uint16_t)cpustate->op1;
	apph = tmp;
	cpustate->_Z = (apph == 0);
	cpustate->_S = ((apph & 0x8000) != 0);
	cpustate->_OV = ((tmp >> 16) != 0);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opMULUW(v60_state *cpustate)
{
	uint32_t appw;
	uint64_t tmp;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	// @@@ OV not set!!
	tmp = (uint64_t)appw * (uint64_t)cpustate->op1;
	appw = tmp;
	cpustate->_Z = (appw == 0);
	cpustate->_S = ((appw & 0x80000000) != 0);
	cpustate->_OV = ((tmp >> 32) != 0);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opNEGB(v60_state *cpustate) /* TRUSTED  (C too!)*/
{
	F12DecodeFirstOperand(cpustate, ReadAM, 0);

	cpustate->modwritevalb = 0;
	SUBB(cpustate->modwritevalb, (int8_t)cpustate->op1);

	F12WriteSecondOperand(cpustate, 0);
	F12END(cpustate);
}

static uint32_t opNEGH(v60_state *cpustate) /* TRUSTED  (C too!)*/
{
	F12DecodeFirstOperand(cpustate, ReadAM, 1);

	cpustate->modwritevalh = 0;
	SUBW(cpustate->modwritevalh, (int16_t)cpustate->op1);

	F12WriteSecondOperand(cpustate, 1);
	F12END(cpustate);
}

static uint32_t opNEGW(v60_state *cpustate) /* TRUSTED  (C too!)*/
{
	F12DecodeFirstOperand(cpustate, ReadAM, 2);

	cpustate->modwritevalw = 0;
	SUBL(cpustate->modwritevalw, (int32_t)cpustate->op1);

	F12WriteSecondOperand(cpustate, 2);
	F12END(cpustate);
}

static uint32_t opNOTB(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAM, 0);
	cpustate->modwritevalb=~cpustate->op1;

	cpustate->_OV = 0;
	cpustate->_S = ((cpustate->modwritevalb & 0x80) != 0);
	cpustate->_Z = (cpustate->modwritevalb == 0);

	F12WriteSecondOperand(cpustate, 0);
	F12END(cpustate);
}

static uint32_t opNOTH(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAM, 1);
	cpustate->modwritevalh=~cpustate->op1;

	cpustate->_OV = 0;
	cpustate->_S = ((cpustate->modwritevalh & 0x8000) != 0);
	cpustate->_Z = (cpustate->modwritevalh == 0);

	F12WriteSecondOperand(cpustate, 1);
	F12END(cpustate);
}

static uint32_t opNOTW(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAM, 2);
	cpustate->modwritevalw=~cpustate->op1;

	cpustate->_OV = 0;
	cpustate->_S = ((cpustate->modwritevalw & 0x80000000) != 0);
	cpustate->_Z = (cpustate->modwritevalw == 0);

	F12WriteSecondOperand(cpustate, 2);
	F12END(cpustate);
}

static uint32_t opNOT1(v60_state *cpustate) /* TRUSTED */
{
	uint32_t appw;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	cpustate->_CY = ((appw & (1 << cpustate->op1)) != 0);
	cpustate->_Z = !(cpustate->_CY);

	if (cpustate->_CY)
		appw &= ~(1 << cpustate->op1);
	else
		appw |= (1 << cpustate->op1);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opORB(v60_state *cpustate) /* TRUSTED  (C too!)*/
{
	uint8_t appb;
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	ORB(appb, (uint8_t)cpustate->op1);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opORH(v60_state *cpustate) /* TRUSTED (C too!)*/
{
	uint16_t apph;
	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	ORW(apph, (uint16_t)cpustate->op1);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opORW(v60_state *cpustate) /* TRUSTED (C too!) */
{
	uint32_t appw;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	ORL(appw, (uint32_t)cpustate->op1);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opOUTB(v60_state *cpustate)
{
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 2);
	MemWrite8(cpustate->io, cpustate->op2,(uint8_t)cpustate->op1);
	F12END(cpustate);
}

static uint32_t opOUTH(v60_state *cpustate)
{
	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 2);
	MemWrite16(cpustate->io, cpustate->op2,(uint16_t)cpustate->op1);
	F12END(cpustate);
}

static uint32_t opOUTW(v60_state *cpustate)
{
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);
	MemWrite32(cpustate->io, cpustate->op2, cpustate->op1);
	F12END(cpustate);
}

static uint32_t opREMB(v60_state *cpustate)
{
	uint8_t appb;
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	cpustate->_OV = 0;
	if (cpustate->op1)
		appb= (int8_t)appb % (int8_t)cpustate->op1;
	cpustate->_Z = (appb == 0);
	cpustate->_S = ((appb & 0x80) != 0);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opREMH(v60_state *cpustate)
{
	uint16_t apph;
	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	cpustate->_OV = 0;
	if (cpustate->op1)
		apph = (int16_t)apph % (int16_t)cpustate->op1;
	cpustate->_Z = (apph == 0);
	cpustate->_S = ((apph & 0x8000) != 0);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opREMW(v60_state *cpustate)
{
	uint32_t appw;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	cpustate->_OV = 0;
	if (cpustate->op1)
		appw = (int32_t)appw % (int32_t)cpustate->op1;
	cpustate->_Z = (appw == 0);
	cpustate->_S = ((appw & 0x80000000) != 0);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opREMUB(v60_state *cpustate)
{
	uint8_t appb;
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	cpustate->_OV = 0;
	if (cpustate->op1)
		appb %= (uint8_t)cpustate->op1;
	cpustate->_Z = (appb == 0);
	cpustate->_S = ((appb & 0x80) != 0);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opREMUH(v60_state *cpustate)
{
	uint16_t apph;
	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	cpustate->_OV = 0;
	if (cpustate->op1)
		apph %= (uint16_t)cpustate->op1;
	cpustate->_Z = (apph == 0);
	cpustate->_S = ((apph & 0x8000) != 0);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opREMUW(v60_state *cpustate)
{
	uint32_t appw;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	cpustate->_OV = 0;
	if (cpustate->op1)
		appw %= cpustate->op1;
	cpustate->_Z = (appw == 0);
	cpustate->_S = ((appw & 0x80000000) != 0);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opROTB(v60_state *cpustate) /* TRUSTED */
{
	uint8_t appb;
	int8_t i, count;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	count = (int8_t)(cpustate->op1 & 0xFF);
	if (count > 0)
	{
		for (i = 0;i < count;i++)
			appb = (appb << 1) | ((appb & 0x80) >> 7);

		cpustate->_CY = (appb & 0x1) != 0;
	}
	else if (count < 0)
	{
		count=-count;
		for (i = 0;i < count;i++)
			appb = (appb >> 1) | ((appb & 0x1) << 7);

		cpustate->_CY = (appb & 0x80) != 0;
	}
	else
		cpustate->_CY = 0;

	cpustate->_OV = 0;
	cpustate->_S = (appb & 0x80) != 0;
	cpustate->_Z = (appb == 0);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opROTH(v60_state *cpustate) /* TRUSTED */
{
	uint16_t apph;
	int8_t i, count;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	count = (int8_t)(cpustate->op1 & 0xFF);
	if (count > 0)
	{
		for (i = 0;i < count;i++)
			apph = (apph << 1) | ((apph & 0x8000) >> 15);

		cpustate->_CY = (apph & 0x1) != 0;
	}
	else if (count < 0)
	{
		count=-count;
		for (i = 0;i < count;i++)
			apph = (apph >> 1) | ((apph & 0x1) << 15);

		cpustate->_CY = (apph & 0x8000) != 0;
	}
	else
		cpustate->_CY = 0;

	cpustate->_OV = 0;
	cpustate->_S = (apph & 0x8000) != 0;
	cpustate->_Z = (apph == 0);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opROTW(v60_state *cpustate) /* TRUSTED */
{
	uint32_t appw;
	int8_t i, count;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	count = (int8_t)(cpustate->op1 & 0xFF);
	if (count > 0)
	{
		for (i = 0;i < count;i++)
			appw = (appw << 1) | ((appw & 0x80000000) >> 31);

		cpustate->_CY = (appw & 0x1) != 0;
	}
	else if (count < 0)
	{
		count=-count;
		for (i = 0;i < count;i++)
			appw = (appw >> 1) | ((appw & 0x1) << 31);

		cpustate->_CY = (appw & 0x80000000) != 0;
	}
	else
		cpustate->_CY = 0;

	cpustate->_OV = 0;
	cpustate->_S = (appw & 0x80000000) != 0;
	cpustate->_Z = (appw == 0);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opROTCB(v60_state *cpustate) /* TRUSTED */
{
	uint8_t appb;
	int8_t i, cy, count;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);
	NORMALIZEFLAGS(cpustate);

	count = (int8_t)(cpustate->op1 & 0xFF);
	if (count > 0)
	{
		for (i = 0;i < count;i++)
		{
			cy = cpustate->_CY;
			cpustate->_CY = (uint8_t)((appb & 0x80) >> 7);
			appb = (appb << 1) | cy;
		}
	}
	else if (count < 0)
	{
		count=-count;
		for (i = 0;i < count;i++)
		{
			cy = cpustate->_CY;
			cpustate->_CY = (appb & 1);
			appb = (appb >> 1) | (cy << 7);
		}
	}
	else
		cpustate->_CY = 0;

	cpustate->_OV = 0;
	cpustate->_S = (appb & 0x80) != 0;
	cpustate->_Z = (appb == 0);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opROTCH(v60_state *cpustate) /* TRUSTED */
{
	uint16_t apph;
	int8_t i, cy, count;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);
	NORMALIZEFLAGS(cpustate);

	count = (int8_t)(cpustate->op1 & 0xFF);
	if (count > 0)
	{
		for (i = 0;i < count;i++)
		{
			cy = cpustate->_CY;
			cpustate->_CY = (uint8_t)((apph & 0x8000) >> 15);
			apph = (apph << 1) | cy;
		}
	}
	else if (count < 0)
	{
		count=-count;
		for (i = 0;i < count;i++)
		{
			cy = cpustate->_CY;
			cpustate->_CY = (uint8_t)(apph & 1);
			apph = (apph >> 1) | ((uint16_t)cy << 15);
		}
	}
	else
		cpustate->_CY = 0;

	cpustate->_OV = 0;
	cpustate->_S = (apph & 0x8000) != 0;
	cpustate->_Z = (apph == 0);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opROTCW(v60_state *cpustate) /* TRUSTED */
{
	uint32_t appw;
	int8_t i, cy, count;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);
	NORMALIZEFLAGS(cpustate);

	count = (int8_t)(cpustate->op1 & 0xFF);
	if (count > 0)
	{
		for (i = 0;i < count;i++)
		{
			cy = cpustate->_CY;
			cpustate->_CY = (uint8_t)((appw & 0x80000000) >> 31);
			appw = (appw << 1) | cy;
		}
	}
	else if (count < 0)
	{
		count=-count;
		for (i = 0;i < count;i++)
		{
			cy = cpustate->_CY;
			cpustate->_CY = (uint8_t)(appw & 1);
			appw = (appw >> 1) | ((uint32_t)cy << 31);
		}
	}
	else
		cpustate->_CY = 0;

	cpustate->_OV = 0;
	cpustate->_S = (appw & 0x80000000) != 0;
	cpustate->_Z = (appw == 0);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opRVBIT(v60_state *cpustate)
{
	F12DecodeFirstOperand(cpustate, ReadAM, 0);

	cpustate->modwritevalb =(uint8_t)
								(((cpustate->op1 & (1 << 0)) << 7) |
								 ((cpustate->op1 & (1 << 1)) << 5) |
								 ((cpustate->op1 & (1 << 2)) << 3) |
								 ((cpustate->op1 & (1 << 3)) << 1) |
								 ((cpustate->op1 & (1 << 4)) >> 1) |
								 ((cpustate->op1 & (1 << 5)) >> 3) |
								 ((cpustate->op1 & (1 << 6)) >> 5) |
								 ((cpustate->op1 & (1 << 7)) >> 7));

	F12WriteSecondOperand(cpustate, 0);
	F12END(cpustate);
}

static uint32_t opRVBYT(v60_state *cpustate) /* TRUSTED */
{
	F12DecodeFirstOperand(cpustate, ReadAM, 2);

	cpustate->modwritevalw = ((cpustate->op1 & 0x000000FF) << 24) |
								 ((cpustate->op1 & 0x0000FF00) << 8)  |
								 ((cpustate->op1 & 0x00FF0000) >> 8)  |
								 ((cpustate->op1 & 0xFF000000) >> 24);

	F12WriteSecondOperand(cpustate, 2);
	F12END(cpustate);
}

static uint32_t opSET1(v60_state *cpustate) /* TRUSTED */
{
	uint32_t appw;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	cpustate->_CY = ((appw & (1 << cpustate->op1)) != 0);
	cpustate->_Z = !(cpustate->_CY);

	appw |= (1 << cpustate->op1);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}


static uint32_t opSETF(v60_state *cpustate)
{
	F12DecodeFirstOperand(cpustate, ReadAM, 0);

	// Normalize the flags
	NORMALIZEFLAGS(cpustate);

	switch (cpustate->op1 & 0xF)
	{
	case 0:
		if (!cpustate->_OV) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	case 1:
		if (cpustate->_OV) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	case 2:
		if (!cpustate->_CY) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	case 3:
		if (cpustate->_CY) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	case 4:
		if (!cpustate->_Z) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	case 5:
		if (cpustate->_Z) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	case 6:
		if (!(cpustate->_CY | cpustate->_Z)) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	case 7:
		if ((cpustate->_CY | cpustate->_Z)) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	case 8:
		if (!cpustate->_S) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	case 9:
		if (cpustate->_S) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	case 10:
		cpustate->modwritevalb = 1;
		break;
	case 11:
		cpustate->modwritevalb = 0;
		break;
	case 12:
		if (!(cpustate->_S^cpustate->_OV)) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	case 13:
		if ((cpustate->_S^cpustate->_OV)) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	case 14:
		if (!((cpustate->_S^cpustate->_OV)|cpustate->_Z)) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	case 15:
		if (((cpustate->_S^cpustate->_OV)|cpustate->_Z)) cpustate->modwritevalb = 0;
		else cpustate->modwritevalb = 1;
		break;
	}

	F12WriteSecondOperand(cpustate, 0);

	F12END(cpustate);
}

/*
#define SHIFTLEFT_OY(val, count, bitsize) \
{\
    uint32_t tmp = ((val) >> (bitsize - 1)) & 1; \
    tmp <<= count; \
    tmp -= 1; \
    tmp <<= (bitsize - (count)); \
    cpustate->_OV = (((val) & tmp) != tmp); \
    cpustate->_CY = (((val) & (1 << (count - 1))) != 0); \
}
*/

// During the shift, the overflow is set if the sign bit changes at any point during the shift
#define SHIFTLEFT_OV(val, count, bitsize) \
{\
	uint32_t tmp; \
	if (count == 32) \
		tmp = 0xFFFFFFFF; \
	else \
		tmp = ((1 << (count)) - 1); \
	tmp <<= (bitsize - (count)); \
	if (((val) >> (bitsize - 1)) & 1) \
		cpustate->_OV = (((val) & tmp) != tmp); \
	else \
		cpustate->_OV = (((val) & tmp) != 0); \
}

#define SHIFTLEFT_CY(val, count, bitsize) \
	cpustate->_CY = (uint8_t)(((val) >> (bitsize - count)) & 1);



#define SHIFTARITHMETICRIGHT_OV(val, count, bitsize) \
	cpustate->_OV = 0;

#define SHIFTARITHMETICRIGHT_CY(val, count, bitsize) \
	cpustate->_CY = (uint8_t)(((val) >> (count - 1)) & 1);



static uint32_t opSHAB(v60_state *cpustate)
{
	uint8_t appb;
	int8_t count;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	count = (int8_t)(cpustate->op1 & 0xFF);

	// Special case: destination unchanged, flags set
	if (count == 0)
	{
		cpustate->_CY = cpustate->_OV = 0;
		SetSZPF_Byte(appb);
	}
	else if (count > 0)
	{
		SHIFTLEFT_OV(appb, count, 8);

		// @@@ Undefined what happens to CY when count >= bitsize
		SHIFTLEFT_CY(appb, count, 8);

		// do the actual shift...
		if (count >= 8)
			appb = 0;
		else
			appb <<= count;

		// and set zero and sign
		SetSZPF_Byte(appb);
	}
	else
	{
		count = -count;

		SHIFTARITHMETICRIGHT_OV(appb, count, 8);
		SHIFTARITHMETICRIGHT_CY(appb, count, 8);

		if (count >= 8)
			appb = (appb & 0x80) ? 0xFF : 0;
		else
			appb = ((int8_t)appb) >> count;

		SetSZPF_Byte(appb);
	}

//  mame_printf_debug("SHAB: %x cpustate->_CY: %d cpustate->_Z: %d cpustate->_OV: %d cpustate->_S: %d\n", appb, cpustate->_CY, cpustate->_Z, cpustate->_OV, cpustate->_S);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opSHAH(v60_state *cpustate)
{
	uint16_t apph;
	int8_t count;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	count = (int8_t)(cpustate->op1 & 0xFF);

	// Special case: destination unchanged, flags set
	if (count == 0)
	{
		cpustate->_CY = cpustate->_OV = 0;
		SetSZPF_Word(apph);
	}
	else if (count > 0)
	{
		SHIFTLEFT_OV(apph, count, 16);

		// @@@ Undefined what happens to CY when count >= bitsize
		SHIFTLEFT_CY(apph, count, 16);

		// do the actual shift...
		if (count >= 16)
			apph = 0;
		else
			apph <<= count;

		// and set zero and sign
		SetSZPF_Word(apph);
	}
	else
	{
		count = -count;

		SHIFTARITHMETICRIGHT_OV(apph, count, 16);
		SHIFTARITHMETICRIGHT_CY(apph, count, 16);

		if (count >= 16)
			apph = (apph & 0x8000) ? 0xFFFF : 0;
		else
			apph = ((int16_t)apph) >> count;

		SetSZPF_Word(apph);
	}

//  mame_printf_debug("SHAH: %x >> %d = %x cpustate->_CY: %d cpustate->_Z: %d cpustate->_OV: %d cpustate->_S: %d\n", oldval, count, apph, cpustate->_CY, cpustate->_Z, cpustate->_OV, cpustate->_S);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opSHAW(v60_state *cpustate)
{
	uint32_t appw;
	int8_t count;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	count = (int8_t)(cpustate->op1 & 0xFF);

	// Special case: destination unchanged, flags set
	if (count == 0)
	{
		cpustate->_CY = cpustate->_OV = 0;
		SetSZPF_Long(appw);
	}
	else if (count > 0)
	{
		SHIFTLEFT_OV(appw, count, 32);

		// @@@ Undefined what happens to CY when count >= bitsize
		SHIFTLEFT_CY(appw, count, 32);

		// do the actual shift...
		if (count >= 32)
			appw = 0;
		else
			appw <<= count;

		// and set zero and sign
		SetSZPF_Long(appw);
	}
	else
	{
		count = -count;

		SHIFTARITHMETICRIGHT_OV(appw, count, 32);
		SHIFTARITHMETICRIGHT_CY(appw, count, 32);

		if (count >= 32)
			appw = (appw & 0x80000000) ? 0xFFFFFFFF : 0;
		else
			appw = ((int32_t)appw) >> count;

		SetSZPF_Long(appw);
	}

//  mame_printf_debug("SHAW: %x >> %d = %x cpustate->_CY: %d cpustate->_Z: %d cpustate->_OV: %d cpustate->_S: %d\n", oldval, count, appw, cpustate->_CY, cpustate->_Z, cpustate->_OV, cpustate->_S);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}


static uint32_t opSHLB(v60_state *cpustate) /* TRUSTED */
{
	uint8_t appb;
	int8_t count;
	uint32_t tmp;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	count = (int8_t)(cpustate->op1 & 0xFF);
	if (count > 0)
	{
		// left shift flags:
		// carry gets the last bit shifted out,
		// overflow is always CLEARed

		cpustate->_OV = 0;	// default to no overflow

		// now handle carry
		tmp = appb & 0xff;
		tmp <<= count;
		SetCFB(tmp);	// set carry properly

		// do the actual shift...
		appb <<= count;

		// and set zero and sign
		SetSZPF_Byte(appb);
	}
	else
	{
		if (count == 0)
		{
			// special case: clear carry and overflow, do nothing else
			cpustate->_CY = cpustate->_OV = 0;
			SetSZPF_Byte(appb);	// doc. is unclear if this is true...
		}
		else
		{
			// right shift flags:
			// carry = last bit shifted out
			// overflow always cleared
			tmp = appb & 0xff;
			tmp >>= ((-count) - 1);
			cpustate->_CY = (uint8_t)(tmp & 0x1);
			cpustate->_OV = 0;

			appb >>= -count;
			SetSZPF_Byte(appb);
		}
	}

//  mame_printf_debug("SHLB: %x cpustate->_CY: %d cpustate->_Z: %d cpustate->_OV: %d cpustate->_S: %d\n", appb, cpustate->_CY, cpustate->_Z, cpustate->_OV, cpustate->_S);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opSHLH(v60_state *cpustate) /* TRUSTED */
{
	uint16_t apph;
	int8_t count;
	uint32_t tmp;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	count = (int8_t)(cpustate->op1 & 0xFF);
//  mame_printf_debug("apph: %x count: %d  ", apph, count);
	if (count > 0)
	{
		// left shift flags:
		// carry gets the last bit shifted out,
		// overflow is always CLEARed

		cpustate->_OV = 0;

		// now handle carry
		tmp = apph & 0xffff;
		tmp <<= count;
		SetCFW(tmp);	// set carry properly

		// do the actual shift...
		apph <<= count;

		// and set zero and sign
		SetSZPF_Word(apph);
	}
	else
	{
		if (count == 0)
		{
			// special case: clear carry and overflow, do nothing else
			cpustate->_CY = cpustate->_OV = 0;
			SetSZPF_Word(apph);	// doc. is unclear if this is true...
		}
		else
		{
			// right shift flags:
			// carry = last bit shifted out
			// overflow always cleared
			tmp = apph & 0xffff;
			tmp >>= ((-count) - 1);
			cpustate->_CY = (uint8_t)(tmp & 0x1);
			cpustate->_OV = 0;

			apph >>= -count;
			SetSZPF_Word(apph);
		}
	}

//  mame_printf_debug("SHLH: %x cpustate->_CY: %d cpustate->_Z: %d cpustate->_OV: %d cpustate->_S: %d\n", apph, cpustate->_CY, cpustate->_Z, cpustate->_OV, cpustate->_S);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opSHLW(v60_state *cpustate) /* TRUSTED */
{
	uint32_t appw;
	int8_t count;
	uint64_t tmp;

	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	count = (int8_t)(cpustate->op1 & 0xFF);
	if (count > 0)
	{
		// left shift flags:
		// carry gets the last bit shifted out,
		// overflow is always CLEARed

		cpustate->_OV = 0;

		// now handle carry
		tmp = appw & 0xffffffff;
		tmp <<= count;
		SetCFL(tmp);	// set carry properly

		// do the actual shift...
		appw <<= count;

		// and set zero and sign
		SetSZPF_Long(appw);
	}
	else
	{
		if (count == 0)
		{
			// special case: clear carry and overflow, do nothing else
			cpustate->_CY = cpustate->_OV = 0;
			SetSZPF_Long(appw);	// doc. is unclear if this is true...
		}
		else
		{
			// right shift flags:
			// carry = last bit shifted out
			// overflow always cleared
			tmp = (uint64_t)(appw & 0xffffffff);
			tmp >>= ((-count) - 1);
			cpustate->_CY = (uint8_t)(tmp & 0x1);
			cpustate->_OV = 0;

			appw >>= -count;
			SetSZPF_Long(appw);
		}
	}

//  mame_printf_debug("SHLW: %x cpustate->_CY: %d cpustate->_Z: %d cpustate->_OV: %d cpustate->_S: %d\n", appw, cpustate->_CY, cpustate->_Z, cpustate->_OV, cpustate->_S);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opSTPR(v60_state *cpustate)
{
	F12DecodeFirstOperand(cpustate, ReadAM, 2);
	if (cpustate->op1 >= 0 && cpustate->op1 <= 28)
		cpustate->modwritevalw = cpustate->reg[cpustate->op1 + 36];
	else
	{
		fatalerror("Invalid operand on STPR cpustate->PC=%x", cpustate->PC);
	}
	F12WriteSecondOperand(cpustate, 2);
	F12END(cpustate);
}


static uint32_t opSUBB(v60_state *cpustate) /* TRUSTED (C too!) */
{
	uint8_t appb;
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	SUBB(appb, (uint8_t)cpustate->op1);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opSUBH(v60_state *cpustate) /* TRUSTED (C too!) */
{
	uint16_t apph;
	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	SUBW(apph, (uint16_t)cpustate->op1);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opSUBW(v60_state *cpustate) /* TRUSTED (C too!) */
{
	uint32_t appw;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	SUBL(appw, (uint32_t)cpustate->op1);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}


static uint32_t opSUBCB(v60_state *cpustate)
{
	uint8_t appb;
	uint8_t src;
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	src = (uint8_t)cpustate->op1 + (cpustate->_CY?1:0);
	SUBB(appb, src);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opSUBCH(v60_state *cpustate)
{
	uint16_t apph;
	uint16_t src;

	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	src = (uint16_t)cpustate->op1 + (cpustate->_CY?1:0);
	SUBW(apph, src);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opSUBCW(v60_state *cpustate)
{
	uint32_t appw;
	uint32_t src;

	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	src = (uint32_t)cpustate->op1 + (cpustate->_CY?1:0);
	SUBL(appw, src);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opTEST1(v60_state *cpustate)
{
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAM, 2);

	cpustate->_CY = ((cpustate->op2 & (1 << cpustate->op1)) != 0);
	cpustate->_Z = !(cpustate->_CY);

	F12END(cpustate);
}

static uint32_t opUPDPSWW(v60_state *cpustate)
{
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAM, 2);

	/* can only modify condition code and control fields */
	cpustate->op2 &= 0xFFFFFF;
	cpustate->op1 &= 0xFFFFFF;
	v60WritePSW(cpustate, (v60ReadPSW(cpustate) & (~cpustate->op2)) | (cpustate->op1 & cpustate->op2));

	F12END(cpustate);
}

static uint32_t opUPDPSWH(v60_state *cpustate)
{
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAM, 2);

	/* can only modify condition code fields */
	cpustate->op2 &= 0xFFFF;
	cpustate->op1 &= 0xFFFF;
	v60WritePSW(cpustate, (v60ReadPSW(cpustate) & (~cpustate->op2)) | (cpustate->op1 & cpustate->op2));

	F12END(cpustate);
}

static uint32_t opXCHB(v60_state *cpustate) /* TRUSTED */
{
	uint8_t appb, temp;

	F12DecodeOperands(cpustate, ReadAMAddress, 0,ReadAMAddress, 0);

	F12LOADOP1BYTE(cpustate);
	temp = appb;
	F12LOADOP2BYTE(cpustate);
	F12STOREOP1BYTE(cpustate);
	appb = temp;
	F12STOREOP2BYTE(cpustate);

	F12END(cpustate)
}

static uint32_t opXCHH(v60_state *cpustate) /* TRUSTED */
{
	uint16_t apph, temp;

	F12DecodeOperands(cpustate, ReadAMAddress, 1,ReadAMAddress, 1);

	F12LOADOP1HALF(cpustate);
	temp = apph;
	F12LOADOP2HALF(cpustate);
	F12STOREOP1HALF(cpustate);
	apph = temp;
	F12STOREOP2HALF(cpustate);

	F12END(cpustate)
}

static uint32_t opXCHW(v60_state *cpustate) /* TRUSTED */
{
	uint32_t appw, temp;

	F12DecodeOperands(cpustate, ReadAMAddress, 2,ReadAMAddress, 2);

	F12LOADOP1WORD(cpustate);
	temp = appw;
	F12LOADOP2WORD(cpustate);
	F12STOREOP1WORD(cpustate);
	appw = temp;
	F12STOREOP2WORD(cpustate);

	F12END(cpustate)
}

static uint32_t opXORB(v60_state *cpustate) /* TRUSTED (C too!) */
{
	uint8_t appb;
	F12DecodeOperands(cpustate, ReadAM, 0,ReadAMAddress, 0);

	F12LOADOP2BYTE(cpustate);

	XORB(appb, (uint8_t)cpustate->op1);

	F12STOREOP2BYTE(cpustate);
	F12END(cpustate);
}

static uint32_t opXORH(v60_state *cpustate) /* TRUSTED (C too!) */
{
	uint16_t apph;
	F12DecodeOperands(cpustate, ReadAM, 1,ReadAMAddress, 1);

	F12LOADOP2HALF(cpustate);

	XORW(apph, (uint16_t)cpustate->op1);

	F12STOREOP2HALF(cpustate);
	F12END(cpustate);
}

static uint32_t opXORW(v60_state *cpustate) /* TRUSTED (C too!) */
{
	uint32_t appw;
	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 2);

	F12LOADOP2WORD(cpustate);

	XORL(appw, (uint32_t)cpustate->op1);

	F12STOREOP2WORD(cpustate);
	F12END(cpustate);
}

static uint32_t opMULX(v60_state *cpustate)
{
	int32_t a, b;
	int64_t res;

	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 3);

	if (cpustate->flag2)
	{
		a = cpustate->reg[cpustate->op2 & 0x1F];
	}
	else
	{
		a = MemRead32(cpustate->program, cpustate->op2);
	}

	res = (int64_t)a * (int64_t)(int32_t)cpustate->op1;

	b = (int32_t)((res >> 32)&0xffffffff);
	a = (int32_t)(res & 0xffffffff);

	cpustate->_S = ((b & 0x80000000) != 0);
	cpustate->_Z = (a == 0 && b == 0);

	if (cpustate->flag2)
	{
		cpustate->reg[cpustate->op2 & 0x1F] = a;
		cpustate->reg[(cpustate->op2 & 0x1F) + 1] = b;
	}
	else
	{
		MemWrite32(cpustate->program, cpustate->op2, a);
		MemWrite32(cpustate->program, cpustate->op2 + 4, b);
	}

	F12END(cpustate);
}

static uint32_t opMULUX(v60_state *cpustate)
{
	int32_t a, b;
	uint64_t res;

	F12DecodeOperands(cpustate, ReadAM, 2,ReadAMAddress, 3);

	if (cpustate->flag2)
	{
		a = cpustate->reg[cpustate->op2 & 0x1F];
	}
	else
	{
		a = MemRead32(cpustate->program, cpustate->op2);
	}

	res = (uint64_t)a * (uint64_t)cpustate->op1;
	b = (int32_t)((res >> 32)&0xffffffff);
	a = (int32_t)(res & 0xffffffff);

	cpustate->_S = ((b & 0x80000000) != 0);
	cpustate->_Z = (a == 0 && b == 0);

	if (cpustate->flag2)
	{
		cpustate->reg[cpustate->op2 & 0x1F] = a;
		cpustate->reg[(cpustate->op2 & 0x1F) + 1] = b;
	}
	else
	{
		MemWrite32(cpustate->program, cpustate->op2, a);
		MemWrite32(cpustate->program, cpustate->op2 + 4, b);
	}

	F12END(cpustate);
}
