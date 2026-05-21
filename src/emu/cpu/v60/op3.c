static uint32_t opINCB(v60_state *cpustate) /* TRUSTED */
{
	uint8_t appb;
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 0;

	cpustate->amlength1 = ReadAMAddress(cpustate);

	if (cpustate->amflag)
		appb = (uint8_t)cpustate->reg[cpustate->amout];
	else
		appb = MemRead8(cpustate->program, cpustate->amout);

	ADDB(appb, 1);

	if (cpustate->amflag)
		SETREG8(cpustate->reg[cpustate->amout], appb);
	else
		MemWrite8(cpustate->program, cpustate->amout, appb);

	return cpustate->amlength1 + 1;
}

static uint32_t opINCH(v60_state *cpustate) /* TRUSTED */
{
	uint16_t apph;
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 1;

	cpustate->amlength1 = ReadAMAddress(cpustate);

	if (cpustate->amflag)
		apph = (uint16_t)cpustate->reg[cpustate->amout];
	else
		apph = MemRead16(cpustate->program, cpustate->amout);

	ADDW(apph, 1);

	if (cpustate->amflag)
		SETREG16(cpustate->reg[cpustate->amout], apph);
	else
		MemWrite16(cpustate->program, cpustate->amout, apph);

	return cpustate->amlength1 + 1;
}

static uint32_t opINCW(v60_state *cpustate) /* TRUSTED */
{
	uint32_t appw;
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 2;

	cpustate->amlength1 = ReadAMAddress(cpustate);

	if (cpustate->amflag)
		appw = cpustate->reg[cpustate->amout];
	else
		appw = MemRead32(cpustate->program, cpustate->amout);

	ADDL(appw, 1);

	if (cpustate->amflag)
		cpustate->reg[cpustate->amout] = appw;
	else
		MemWrite32(cpustate->program, cpustate->amout, appw);

	return cpustate->amlength1 + 1;
}

static uint32_t opDECB(v60_state *cpustate) /* TRUSTED */
{
	uint8_t appb;
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 0;

	cpustate->amlength1 = ReadAMAddress(cpustate);

	if (cpustate->amflag)
		appb = (uint8_t)cpustate->reg[cpustate->amout];
	else
		appb = MemRead8(cpustate->program, cpustate->amout);

	SUBB(appb, 1);

	if (cpustate->amflag)
		SETREG8(cpustate->reg[cpustate->amout], appb);
	else
		MemWrite8(cpustate->program, cpustate->amout, appb);

	return cpustate->amlength1 + 1;
}

static uint32_t opDECH(v60_state *cpustate) /* TRUSTED */
{
	uint16_t apph;
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 1;

	cpustate->amlength1 = ReadAMAddress(cpustate);

	if (cpustate->amflag)
		apph = (uint16_t)cpustate->reg[cpustate->amout];
	else
		apph = MemRead16(cpustate->program, cpustate->amout);

	SUBW(apph, 1);

	if (cpustate->amflag)
		SETREG16(cpustate->reg[cpustate->amout], apph);
	else
		MemWrite16(cpustate->program, cpustate->amout, apph);

	return cpustate->amlength1 + 1;
}

static uint32_t opDECW(v60_state *cpustate) /* TRUSTED */
{
	uint32_t appw;
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 2;

	cpustate->amlength1 = ReadAMAddress(cpustate);

	if (cpustate->amflag)
		appw = cpustate->reg[cpustate->amout];
	else
		appw = MemRead32(cpustate->program, cpustate->amout);

	SUBL(appw, 1);

	if (cpustate->amflag)
		cpustate->reg[cpustate->amout] = appw;
	else
		MemWrite32(cpustate->program, cpustate->amout, appw);

	return cpustate->amlength1 + 1;
}

static uint32_t opJMP(v60_state *cpustate) /* TRUSTED */
{
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 0;

	// Read the address of the operand
	ReadAMAddress(cpustate);

	// It cannot be a register!!
	assert(cpustate->amflag == 0);

	// Jump there
	cpustate->PC = cpustate->amout;

	return 0;
}

static uint32_t opJSR(v60_state *cpustate) /* TRUSTED */
{
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 0;

	// Read the address of the operand
	cpustate->amlength1 = ReadAMAddress(cpustate);

	// It cannot be a register!!
	assert(cpustate->amflag == 0);

	// Save NextPC into the stack
	cpustate->SP -= 4;
	MemWrite32(cpustate->program, cpustate->SP, cpustate->PC + cpustate->amlength1 + 1);

	// Jump there
	cpustate->PC = cpustate->amout;

	return 0;
}

static uint32_t opPREPARE(v60_state *cpustate)	/* somewhat TRUSTED */
{
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 2;

	// Read the operand
	cpustate->amlength1 = ReadAM(cpustate);

	// step 1: save frame pointer on the stack
	cpustate->SP -= 4;
	MemWrite32(cpustate->program, cpustate->SP, cpustate->FP);

	// step 2: cpustate->FP = new cpustate->SP
	cpustate->FP = cpustate->SP;

	// step 3: cpustate->SP -= operand
	cpustate->SP -= cpustate->amout;

	return cpustate->amlength1 + 1;
}

static uint32_t opRET(v60_state *cpustate) /* TRUSTED */
{
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 2;

	// Read the operand
	ReadAM(cpustate);

	// Read return address from stack
	cpustate->PC = MemRead32(cpustate->program, cpustate->SP);
	cpustate->SP +=4;

	// Restore cpustate->AP from stack
	cpustate->AP = MemRead32(cpustate->program, cpustate->SP);
	cpustate->SP +=4;

	// Skip stack frame
	cpustate->SP += cpustate->amout;

	return 0;
}

static uint32_t opTRAP(v60_state *cpustate)
{
	uint32_t oldPSW;

	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 0;

	// Read the operand
	cpustate->amlength1 = ReadAM(cpustate);

	// Normalize the flags
	NORMALIZEFLAGS(cpustate);

	switch ((cpustate->amout >> 4) & 0xF)
	{
	case 0:
		if (!cpustate->_OV) return cpustate->amlength1 + 1;
		else break;
	case 1:
		if (cpustate->_OV) return cpustate->amlength1 + 1;
		else break;
	case 2:
		if (!cpustate->_CY) return cpustate->amlength1 + 1;
		else break;
	case 3:
		if (cpustate->_CY) return cpustate->amlength1 + 1;
		else break;
	case 4:
		if (!cpustate->_Z) return cpustate->amlength1 + 1;
		else break;
	case 5:
		if (cpustate->_Z) return cpustate->amlength1 + 1;
		else break;
	case 6:
		if (!(cpustate->_CY | cpustate->_Z)) return cpustate->amlength1 + 1;
		else break;
	case 7:
		if ((cpustate->_CY | cpustate->_Z)) return cpustate->amlength1 + 1;
		else break;
	case 8:
		if (!cpustate->_S) return cpustate->amlength1 + 1;
		else break;
	case 9:
		if (cpustate->_S) return cpustate->amlength1 + 1;
		else break;
	case 10:
		break;
	case 11:
		return cpustate->amlength1 + 1;
	case 12:
		if (!(cpustate->_S^cpustate->_OV)) return cpustate->amlength1 + 1;
		else break;
	case 13:
		if ((cpustate->_S^cpustate->_OV)) return cpustate->amlength1 + 1;
		else break;
	case 14:
		if (!((cpustate->_S^cpustate->_OV)|cpustate->_Z)) return cpustate->amlength1 + 1;
		else break;
	case 15:
		if (((cpustate->_S^cpustate->_OV)|cpustate->_Z)) return cpustate->amlength1 + 1;
		else break;
	}

	oldPSW = v60_update_psw_for_exception(cpustate, 0, 0);

	// Issue the software trap with interrupts
	cpustate->SP -= 4;
	MemWrite32(cpustate->program, cpustate->SP, EXCEPTION_CODE_AND_SIZE(0x3000 + 0x100 * (cpustate->amout & 0xF), 4));

	cpustate->SP -= 4;
	MemWrite32(cpustate->program, cpustate->SP, oldPSW);

	cpustate->SP -= 4;
	MemWrite32(cpustate->program, cpustate->SP, cpustate->PC + cpustate->amlength1 + 1);

	cpustate->PC = GETINTVECT(cpustate, 48 + (cpustate->amout & 0xF));

	return 0;
}

static uint32_t opRETIU(v60_state *cpustate) /* TRUSTED */
{
	uint32_t newPSW;
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 1;

	// Read the operand
	ReadAM(cpustate);

	// Restore cpustate->PC and cpustate->PSW from stack
	cpustate->PC = MemRead32(cpustate->program, cpustate->SP);
	cpustate->SP += 4;

	newPSW = MemRead32(cpustate->program, cpustate->SP);
	cpustate->SP += 4;

	// Destroy stack frame
	cpustate->SP += cpustate->amout;

	v60WritePSW(cpustate, newPSW);

	return 0;
}

static uint32_t opRETIS(v60_state *cpustate)
{
	uint32_t newPSW;

	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 1;

	// Read the operand
	ReadAM(cpustate);

	// Restore cpustate->PC and cpustate->PSW from stack
	cpustate->PC = MemRead32(cpustate->program, cpustate->SP);
	cpustate->SP += 4;

	newPSW = MemRead32(cpustate->program, cpustate->SP);
	cpustate->SP += 4;

	// Destroy stack frame
	cpustate->SP += cpustate->amout;

	v60WritePSW(cpustate, newPSW);

	return 0;
}

static uint32_t opSTTASK(v60_state *cpustate)
{
	int i;
	uint32_t adr;

	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 2;

	cpustate->amlength1 = ReadAM(cpustate);

	adr = cpustate->TR;

	v60WritePSW(cpustate, v60ReadPSW(cpustate) | 0x10000000);
	v60SaveStack(cpustate);

	MemWrite32(cpustate->program, adr, cpustate->TKCW);
	adr += 4;
	if(cpustate->SYCW & 0x100) {
		MemWrite32(cpustate->program, adr, cpustate->L0SP);
		adr += 4;
	}
	if(cpustate->SYCW & 0x200) {
		MemWrite32(cpustate->program, adr, cpustate->L1SP);
		adr += 4;
	}
	if(cpustate->SYCW & 0x400) {
		MemWrite32(cpustate->program, adr, cpustate->L2SP);
		adr += 4;
	}
	if(cpustate->SYCW & 0x800) {
		MemWrite32(cpustate->program, adr, cpustate->L3SP);
		adr += 4;
	}

	// 31 registers supported, _not_ 32
	for(i = 0; i < 31; i++)
		if(cpustate->amout & (1 << i)) {
			MemWrite32(cpustate->program, adr, cpustate->reg[i]);
			adr += 4;
		}

	// #### Ignore the virtual addressing crap.

	return cpustate->amlength1 + 1;
}

static uint32_t opGETPSW(v60_state *cpustate)
{
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 2;
	cpustate->modwritevalw = v60ReadPSW(cpustate);

	// Write cpustate->PSW to the operand
	cpustate->amlength1 = WriteAM(cpustate);

	return cpustate->amlength1 + 1;
}

static uint32_t opTASI(v60_state *cpustate)
{
	uint8_t appb;
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 0;

	// Load the address of the operand
	cpustate->amlength1 = ReadAMAddress(cpustate);

	// Load uint8_t from the address
	if (cpustate->amflag)
		appb = (uint8_t)cpustate->reg[cpustate->amout & 0x1F];
	else
		appb = MemRead8(cpustate->program, cpustate->amout);

	// Set the flags for SUB appb, FF
	SUBB(appb, 0xff);

	// Write FF in the operand
	if (cpustate->amflag)
		SETREG8(cpustate->reg[cpustate->amout & 0x1F], 0xFF);
	else
		MemWrite8(cpustate->program, cpustate->amout, 0xFF);

	return cpustate->amlength1 + 1;
}

static uint32_t opCLRTLB(v60_state *cpustate)
{
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 2;

	// Read the operand
	cpustate->amlength1 = ReadAM(cpustate);

	// @@@ TLB not yet emulated

	return cpustate->amlength1 + 1;
}

static uint32_t opPOPM(v60_state *cpustate)
{
	int i;

	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 2;

	// Read the bit register list
	cpustate->amlength1 = ReadAM(cpustate);

	for (i = 0;i < 31;i++)
		if (cpustate->amout & (1 << i))
		{
			cpustate->reg[i] = MemRead32(cpustate->program, cpustate->SP);
			cpustate->SP += 4;
		}

	if (cpustate->amout & (1 << 31))
	{
		v60WritePSW(cpustate, (v60ReadPSW(cpustate) & 0xffff0000) | MemRead16(cpustate->program, cpustate->SP));
		cpustate->SP += 4;
	}

	return cpustate->amlength1 + 1;
}

static uint32_t opPUSHM(v60_state *cpustate)
{
	int i;

	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 2;

	// Read the bit register list
	cpustate->amlength1 = ReadAM(cpustate);

	if (cpustate->amout & (1 << 31))
	{
		cpustate->SP -= 4;
		MemWrite32(cpustate->program, cpustate->SP, v60ReadPSW(cpustate));
	}

	for (i = 0;i < 31;i++)
		if (cpustate->amout & (1 << (30 - i)))
		{
			cpustate->SP -= 4;
			MemWrite32(cpustate->program, cpustate->SP, cpustate->reg[(30 - i)]);
		}


	return cpustate->amlength1 + 1;
}

static uint32_t opTESTB(v60_state *cpustate) /* TRUSTED */
{
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 0;

	// Read the operand
	cpustate->amlength1 = ReadAM(cpustate);

	cpustate->_Z = (cpustate->amout == 0);
	cpustate->_S = ((cpustate->amout & 0x80) != 0);
	cpustate->_CY = 0;
	cpustate->_OV = 0;

	return cpustate->amlength1 + 1;
}

static uint32_t opTESTH(v60_state *cpustate) /* TRUSTED */
{
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 1;

	// Read the operand
	cpustate->amlength1 = ReadAM(cpustate);

	cpustate->_Z = (cpustate->amout == 0);
	cpustate->_S = ((cpustate->amout & 0x8000) != 0);
	cpustate->_CY = 0;
	cpustate->_OV = 0;

	return cpustate->amlength1 + 1;
}

static uint32_t opTESTW(v60_state *cpustate) /* TRUSTED */
{
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 2;

	// Read the operand
	cpustate->amlength1 = ReadAM(cpustate);

	cpustate->_Z = (cpustate->amout == 0);
	cpustate->_S = ((cpustate->amout & 0x80000000) != 0);
	cpustate->_CY = 0;
	cpustate->_OV = 0;

	return cpustate->amlength1 + 1;
}

static uint32_t opPUSH(v60_state *cpustate)
{
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 2;

	cpustate->amlength1 = ReadAM(cpustate);

	cpustate->SP-=4;
	MemWrite32(cpustate->program, cpustate->SP, cpustate->amout);

	return cpustate->amlength1 + 1;
}

static uint32_t opPOP(v60_state *cpustate)
{
	cpustate->modadd = cpustate->PC + 1;
	cpustate->moddim = 2;
	cpustate->modwritevalw = MemRead32(cpustate->program, cpustate->SP);
	cpustate->SP +=4;
	cpustate->amlength1 = WriteAM(cpustate);

	return cpustate->amlength1 + 1;
}


static uint32_t opINCB_0(v60_state *cpustate) { cpustate->modm = 0; return opINCB(cpustate); }
static uint32_t opINCB_1(v60_state *cpustate) { cpustate->modm = 1; return opINCB(cpustate); }
static uint32_t opINCH_0(v60_state *cpustate) { cpustate->modm = 0; return opINCH(cpustate); }
static uint32_t opINCH_1(v60_state *cpustate) { cpustate->modm = 1; return opINCH(cpustate); }
static uint32_t opINCW_0(v60_state *cpustate) { cpustate->modm = 0; return opINCW(cpustate); }
static uint32_t opINCW_1(v60_state *cpustate) { cpustate->modm = 1; return opINCW(cpustate); }

static uint32_t opDECB_0(v60_state *cpustate) { cpustate->modm = 0; return opDECB(cpustate); }
static uint32_t opDECB_1(v60_state *cpustate) { cpustate->modm = 1; return opDECB(cpustate); }
static uint32_t opDECH_0(v60_state *cpustate) { cpustate->modm = 0; return opDECH(cpustate); }
static uint32_t opDECH_1(v60_state *cpustate) { cpustate->modm = 1; return opDECH(cpustate); }
static uint32_t opDECW_0(v60_state *cpustate) { cpustate->modm = 0; return opDECW(cpustate); }
static uint32_t opDECW_1(v60_state *cpustate) { cpustate->modm = 1; return opDECW(cpustate); }

static uint32_t opJMP_0(v60_state *cpustate) { cpustate->modm = 0; return opJMP(cpustate); }
static uint32_t opJMP_1(v60_state *cpustate) { cpustate->modm = 1; return opJMP(cpustate); }

static uint32_t opJSR_0(v60_state *cpustate) { cpustate->modm = 0; return opJSR(cpustate); }
static uint32_t opJSR_1(v60_state *cpustate) { cpustate->modm = 1; return opJSR(cpustate); }

static uint32_t opPREPARE_0(v60_state *cpustate) { cpustate->modm = 0; return opPREPARE(cpustate); }
static uint32_t opPREPARE_1(v60_state *cpustate) { cpustate->modm = 1; return opPREPARE(cpustate); }

static uint32_t opRET_0(v60_state *cpustate) { cpustate->modm = 0; return opRET(cpustate); }
static uint32_t opRET_1(v60_state *cpustate) { cpustate->modm = 1; return opRET(cpustate); }

static uint32_t opTRAP_0(v60_state *cpustate) { cpustate->modm = 0; return opTRAP(cpustate); }
static uint32_t opTRAP_1(v60_state *cpustate) { cpustate->modm = 1; return opTRAP(cpustate); }

static uint32_t opRETIU_0(v60_state *cpustate) { cpustate->modm = 0; return opRETIU(cpustate); }
static uint32_t opRETIU_1(v60_state *cpustate) { cpustate->modm = 1; return opRETIU(cpustate); }

static uint32_t opRETIS_0(v60_state *cpustate) { cpustate->modm = 0; return opRETIS(cpustate); }
static uint32_t opRETIS_1(v60_state *cpustate) { cpustate->modm = 1; return opRETIS(cpustate); }

static uint32_t opGETPSW_0(v60_state *cpustate) { cpustate->modm = 0; return opGETPSW(cpustate); }
static uint32_t opGETPSW_1(v60_state *cpustate) { cpustate->modm = 1; return opGETPSW(cpustate); }

static uint32_t opTASI_0(v60_state *cpustate) { cpustate->modm = 0; return opTASI(cpustate); }
static uint32_t opTASI_1(v60_state *cpustate) { cpustate->modm = 1; return opTASI(cpustate); }

static uint32_t opCLRTLB_0(v60_state *cpustate) { cpustate->modm = 0; return opCLRTLB(cpustate); }
static uint32_t opCLRTLB_1(v60_state *cpustate) { cpustate->modm = 1; return opCLRTLB(cpustate); }

static uint32_t opPOPM_0(v60_state *cpustate) { cpustate->modm = 0; return opPOPM(cpustate); }
static uint32_t opPOPM_1(v60_state *cpustate) { cpustate->modm = 1; return opPOPM(cpustate); }

static uint32_t opPUSHM_0(v60_state *cpustate) { cpustate->modm = 0; return opPUSHM(cpustate); }
static uint32_t opPUSHM_1(v60_state *cpustate) { cpustate->modm = 1; return opPUSHM(cpustate); }

static uint32_t opTESTB_0(v60_state *cpustate) { cpustate->modm = 0; return opTESTB(cpustate); }
static uint32_t opTESTB_1(v60_state *cpustate) { cpustate->modm = 1; return opTESTB(cpustate); }

static uint32_t opTESTH_0(v60_state *cpustate) { cpustate->modm = 0; return opTESTH(cpustate); }
static uint32_t opTESTH_1(v60_state *cpustate) { cpustate->modm = 1; return opTESTH(cpustate); }

static uint32_t opTESTW_0(v60_state *cpustate) { cpustate->modm = 0; return opTESTW(cpustate); }
static uint32_t opTESTW_1(v60_state *cpustate) { cpustate->modm = 1; return opTESTW(cpustate); }

static uint32_t opPUSH_0(v60_state *cpustate) { cpustate->modm = 0; return opPUSH(cpustate); }
static uint32_t opPUSH_1(v60_state *cpustate) { cpustate->modm = 1; return opPUSH(cpustate); }

static uint32_t opPOP_0(v60_state *cpustate) { cpustate->modm = 0; return opPOP(cpustate); }
static uint32_t opPOP_1(v60_state *cpustate) { cpustate->modm = 1; return opPOP(cpustate); }

static uint32_t opSTTASK_0(v60_state *cpustate) { cpustate->modm = 0; return opSTTASK(cpustate); }
static uint32_t opSTTASK_1(v60_state *cpustate) { cpustate->modm = 1; return opSTTASK(cpustate); }
