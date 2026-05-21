
/*
 *  HALT: must add log
 */

static uint32_t opBRK(v60_state *cpustate)
{
/*
    uint32_t oldPSW = v60_update_psw_for_exception(cpustate, 0, 0);

    cpustate->SP -=4;
    MemWrite32(cpustate->program, cpustate->SP, EXCEPTION_CODE_AND_SIZE(0x0d00, 4));
    cpustate->SP -=4;
    MemWrite32(cpustate->program, cpustate->SP, oldPSW);
    cpustate->SP -=4;
    MemWrite32(cpustate->program, cpustate->SP, cpustate->PC + 1);
    cpustate->PC = GETINTVECT(cpustate, 13);
*/
	logerror("Skipping BRK opcode! cpustate->PC=%x", cpustate->PC);

	return 1;
}

static uint32_t opBRKV(v60_state *cpustate)
{
	uint32_t oldPSW = v60_update_psw_for_exception(cpustate, 0, 0);

	cpustate->SP -=4;
	MemWrite32(cpustate->program, cpustate->SP, cpustate->PC);
	cpustate->SP -=4;
	MemWrite32(cpustate->program, cpustate->SP, EXCEPTION_CODE_AND_SIZE(0x1501, 4));
	cpustate->SP -=4;
	MemWrite32(cpustate->program, cpustate->SP, oldPSW);
	cpustate->SP -=4;
	MemWrite32(cpustate->program, cpustate->SP, cpustate->PC + 1);
	cpustate->PC = GETINTVECT(cpustate, 21);

	return 0;
}

static uint32_t opCLRTLBA(v60_state *cpustate)
{
	// @@@ TLB not yet supported
	logerror("Skipping CLRTLBA opcode! cpustate->PC=%x\n", cpustate->PC);
	return 1;
}

static uint32_t opDISPOSE(v60_state *cpustate)
{
	cpustate->SP = cpustate->FP;
	cpustate->FP = MemRead32(cpustate->program, cpustate->SP);
	cpustate->SP +=4;

	return 1;
}

static uint32_t opHALT(v60_state *cpustate)
{
	// @@@ It should wait for an interrupt to occur
	//logerror("HALT found: skipping");
	return 1;
}

static uint32_t opNOP(v60_state *cpustate) /* TRUSTED */
{
	return 1;
}

static uint32_t opRSR(v60_state *cpustate)
{
	cpustate->PC = MemRead32(cpustate->program, cpustate->SP);
	cpustate->SP +=4;

	return 0;
}

static uint32_t opTRAPFL(v60_state *cpustate)
{
	if ((cpustate->TKCW & 0x1F0) & ((v60ReadPSW(cpustate) & 0x1F00) >> 4))
	{
		// @@@ FPU exception
		fatalerror("Hit TRAPFL! cpustate->PC=%x", cpustate->PC);
	}

	return 1;
}







