

#define F2END(cs) \
	return 2 + (cs)->amlength1 + (cs)->amlength2;

#define F2LOADOPFLOAT(cs, num)								\
	if ((cs)->flag##num)									\
		appf = u2f((cs)->reg[(cs)->op##num]);				\
	else													\
		appf = u2f(MemRead32((cs)->program, (cs)->op##num));

#define F2STOREOPFLOAT(cs,num)								\
	if ((cs)->flag##num)									\
		(cs)->reg[(cs)->op##num] = f2u(appf);				\
	else													\
		MemWrite32((cs)->program, (cs)->op##num, f2u(appf));

static void F2DecodeFirstOperand(v60_state *cpustate, uint32_t (*DecodeOp1)(v60_state *), uint8_t dim1)
{
	cpustate->moddim = dim1;
	cpustate->modm = cpustate->instflags & 0x40;
	cpustate->modadd = cpustate->PC + 2;
	cpustate->amlength1 = DecodeOp1(cpustate);
	cpustate->op1 = cpustate->amout;
	cpustate->flag1 = cpustate->amflag;
}

static void F2DecodeSecondOperand(v60_state *cpustate, uint32_t (*DecodeOp2)(v60_state *), uint8_t dim2)
{
	cpustate->moddim = dim2;
	cpustate->modm = cpustate->instflags & 0x20;
	cpustate->modadd = cpustate->PC + 2 + cpustate->amlength1;
	cpustate->amlength2 = DecodeOp2(cpustate);
	cpustate->op2 = cpustate->amout;
	cpustate->flag2 = cpustate->amflag;
}

static void F2WriteSecondOperand(v60_state *cpustate, uint8_t dim2)
{
	cpustate->moddim = dim2;
	cpustate->modm = cpustate->instflags & 0x20;
	cpustate->modadd = cpustate->PC + 2 + cpustate->amlength1;
	cpustate->amlength2 = WriteAM(cpustate);
}

static uint32_t opCVTWS(v60_state *cpustate)
{
	float val;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);

	// Convert to float
	val = (float)(int32_t)cpustate->op1;
	cpustate->modwritevalw = f2u(val);

	cpustate->_OV = 0;
	cpustate->_CY = (val < 0.0f);
	cpustate->_S = ((cpustate->modwritevalw & 0x80000000) != 0);
	cpustate->_Z = (val == 0.0f);

	F2WriteSecondOperand(cpustate, 2);
	F2END(cpustate);
}

static uint32_t opCVTSW(v60_state *cpustate)
{
	float val;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);

	// Convert to uint32_t
	val = u2f(cpustate->op1);
	cpustate->modwritevalw = (uint32_t)val;

	cpustate->_OV = 0;
	cpustate->_CY =(val < 0.0f);
	cpustate->_S = ((cpustate->modwritevalw & 0x80000000) != 0);
	cpustate->_Z = (val == 0.0f);

	F2WriteSecondOperand(cpustate, 2);
	F2END(cpustate);
}

static uint32_t opMOVFS(v60_state *cpustate)
{
	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	cpustate->modwritevalw = cpustate->op1;
	F2WriteSecondOperand(cpustate, 2);
	F2END(cpustate);
}

static uint32_t opNEGFS(v60_state *cpustate)
{
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	appf = -u2f(cpustate->op1);

	cpustate->_OV = 0;
	cpustate->_CY = (appf < 0.0f);
	cpustate->_S = ((f2u(appf) & 0x80000000) != 0);
	cpustate->_Z = (appf == 0.0f);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static uint32_t opABSFS(v60_state *cpustate)
{
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	appf = u2f(cpustate->op1);

	if(appf < 0)
		appf = -appf;

	cpustate->_OV = 0;
	cpustate->_CY = 0;
	cpustate->_S = ((f2u(appf) & 0x80000000) != 0);
	cpustate->_Z = (appf == 0.0f);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static uint32_t opADDFS(v60_state *cpustate)
{
	uint32_t appw;
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	F2LOADOPFLOAT(cpustate, 2);

	appf += u2f(cpustate->op1);

	appw = f2u(appf);
	cpustate->_OV = cpustate->_CY = 0;
	cpustate->_S = ((appw & 0x80000000) != 0);
	cpustate->_Z = (appw == 0);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static uint32_t opSUBFS(v60_state *cpustate)
{
	uint32_t appw;
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	F2LOADOPFLOAT(cpustate, 2);

	appf -= u2f(cpustate->op1);

	appw = f2u(appf);
	cpustate->_OV = cpustate->_CY = 0;
	cpustate->_S = ((appw & 0x80000000) != 0);
	cpustate->_Z = (appw == 0);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static uint32_t opMULFS(v60_state *cpustate)
{
	uint32_t appw;
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	F2LOADOPFLOAT(cpustate, 2);

	appf *= u2f(cpustate->op1);

	appw = f2u(appf);
	cpustate->_OV = cpustate->_CY = 0;
	cpustate->_S = ((appw & 0x80000000) != 0);
	cpustate->_Z = (appw == 0);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static uint32_t opDIVFS(v60_state *cpustate)
{
	uint32_t appw;
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	F2LOADOPFLOAT(cpustate, 2);

	appf /= u2f(cpustate->op1);

	appw = f2u(appf);
	cpustate->_OV = cpustate->_CY = 0;
	cpustate->_S = ((appw & 0x80000000) != 0);
	cpustate->_Z = (appw == 0);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static uint32_t opSCLFS(v60_state *cpustate)
{
	uint32_t appw;
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 1);
	F2DecodeSecondOperand(cpustate, ReadAMAddress, 2);

	F2LOADOPFLOAT(cpustate, 2);

	if ((int16_t)cpustate->op1 < 0)
		appf /= 1 << -(int16_t)cpustate->op1;
	else
		appf *= 1 << cpustate->op1;

	appw = f2u(appf);
	cpustate->_OV = cpustate->_CY = 0;
	cpustate->_S = ((appw & 0x80000000) != 0);
	cpustate->_Z = (appw == 0);

	F2STOREOPFLOAT(cpustate, 2);
	F2END(cpustate)
}

static uint32_t opCMPF(v60_state *cpustate)
{
	float appf;

	F2DecodeFirstOperand(cpustate, ReadAM, 2);
	F2DecodeSecondOperand(cpustate, ReadAM, 2);

	appf = u2f(cpustate->op2) - u2f(cpustate->op1);

	cpustate->_Z = (appf == 0);
	cpustate->_S = (appf < 0);
	cpustate->_OV = 0;
	cpustate->_CY = 0;

	F2END(cpustate);
}

static uint32_t op5FUNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 5F opcode at %08x", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static uint32_t op5CUNHANDLED(v60_state *cpustate)
{
	fatalerror("Unhandled 5C opcode at %08x", cpustate->PC);
	return 0; /* never reached, fatalerror won't return */
}

static uint32_t (*const Op5FTable[32])(v60_state *) =
{
	opCVTWS,
	opCVTSW,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED,
	op5FUNHANDLED
};

static uint32_t (*const Op5CTable[32])(v60_state *) =
{
	opCMPF,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	opMOVFS,
	opNEGFS,
	opABSFS,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,

	opSCLFS,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	opADDFS,
	opSUBFS,
	opMULFS,
	opDIVFS,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED,
	op5CUNHANDLED
};


static uint32_t op5F(v60_state *cpustate)
{
	cpustate->instflags = OpRead8(cpustate->program, cpustate->PC + 1);
	return Op5FTable[cpustate->instflags & 0x1F](cpustate);
}


static uint32_t op5C(v60_state *cpustate)
{
	cpustate->instflags = OpRead8(cpustate->program, cpustate->PC + 1);
	return Op5CTable[cpustate->instflags & 0x1F](cpustate);
}
