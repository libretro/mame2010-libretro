// Pentium+ specific opcodes

static void PENTIUMOP(rdmsr)(i386_state *cpustate)			// Opcode 0x0f 32
{
	// TODO
	CYCLES(cpustate,CYCLES_RDMSR);
}

static void PENTIUMOP(wrmsr)(i386_state *cpustate)			// Opcode 0x0f 30
{
	// TODO
	CYCLES(cpustate,1);		// TODO: correct cycle count
}

static void PENTIUMOP(rdtsc)(i386_state *cpustate)			// Opcode 0x0f 31
{
	uint64_t ts = cpustate->tsc + (cpustate->base_cycles - cpustate->cycles);
	REG32(EAX) = (uint32_t)(ts);
	REG32(EDX) = (uint32_t)(ts >> 32);

	CYCLES(cpustate,CYCLES_RDTSC);
}

static void I386OP(cyrix_unknown)(i386_state *cpustate)		// Opcode 0x0f 74
{
	CYCLES(cpustate,1);
}

static void PENTIUMOP(cmpxchg8b_m64)(i386_state *cpustate)	// Opcode 0x0f c7
{
	uint8_t modm = FETCH(cpustate);
	if( modm >= 0xc0 ) {
		fatalerror("pentium: cmpxchg8b_m64 - invalid modm");
	} else {
		uint32_t ea = GetEA(cpustate,modm);
		uint64_t value = READ64(cpustate,ea);
		uint64_t edx_eax = (((uint64_t) REG32(EDX)) << 32) | REG32(EAX);
		uint64_t ecx_ebx = (((uint64_t) REG32(ECX)) << 32) | REG32(EBX);

		if( value == edx_eax ) {
			WRITE64(cpustate,ea, ecx_ebx);
			cpustate->ZF = 1;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_MEM_T);
		} else {
			REG32(EDX) = (uint32_t) (value >> 32);
			REG32(EAX) = (uint32_t) (value >>  0);
			cpustate->ZF = 0;
			CYCLES(cpustate,CYCLES_CMPXCHG_REG_MEM_F);
		}
	}
}


