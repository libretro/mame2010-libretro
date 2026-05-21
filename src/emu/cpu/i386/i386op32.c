static uint32_t I386OP(shift_rotate32)(i386_state *cpustate, uint8_t modrm, uint32_t value, uint8_t shift)
{
	uint32_t dst, src;
	dst = value;
	src = value;

	if( shift == 0 ) {
		CYCLES_RM(cpustate,modrm, 3, 7);
	} else if( shift == 1 ) {

		switch( (modrm >> 3) & 0x7 )
		{
			case 0:			/* ROL rm32, 1 */
				cpustate->CF = (src & 0x80000000) ? 1 : 0;
				dst = (src << 1) + cpustate->CF;
				cpustate->OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:			/* ROR rm32, 1 */
				cpustate->CF = (src & 0x1) ? 1 : 0;
				dst = (cpustate->CF << 31) | (src >> 1);
				cpustate->OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:			/* RCL rm32, 1 */
				dst = (src << 1) + cpustate->CF;
				cpustate->CF = (src & 0x80000000) ? 1 : 0;
				cpustate->OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:			/* RCR rm32, 1 */
				dst = (cpustate->CF << 31) | (src >> 1);
				cpustate->CF = src & 0x1;
				cpustate->OF = ((src ^ dst) & 0x80000000) ? 1 : 0;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:			/* SHL/SAL rm32, 1 */
			case 6:
				dst = src << 1;
				cpustate->CF = (src & 0x80000000) ? 1 : 0;
				cpustate->OF = (((cpustate->CF << 31) ^ dst) & 0x80000000) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:			/* SHR rm32, 1 */
				dst = src >> 1;
				cpustate->CF = src & 0x1;
				cpustate->OF = (src & 0x80000000) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:			/* SAR rm32, 1 */
				dst = (int32_t)(src) >> 1;
				cpustate->CF = src & 0x1;
				cpustate->OF = 0;
				SetSZPF32(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	} else {

		switch( (modrm >> 3) & 0x7 )
		{
			case 0:			/* ROL rm32, i8 */
				dst = ((src & ((uint32_t)0xffffffff >> shift)) << shift) |
					  ((src & ((uint32_t)0xffffffff << (32-shift))) >> (32-shift));
				cpustate->CF = (src >> (32-shift)) & 0x1;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 1:			/* ROR rm32, i8 */
				dst = ((src & ((uint32_t)0xffffffff << shift)) >> shift) |
					  ((src & ((uint32_t)0xffffffff >> (32-shift))) << (32-shift));
				cpustate->CF = (src >> (shift-1)) & 0x1;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 2:			/* RCL rm32, i8 */
				dst = ((src & ((uint32_t)0xffffffff >> shift)) << shift) |
					  ((src & ((uint32_t)0xffffffff << (33-shift))) >> (33-shift)) |
					  (cpustate->CF << (shift-1));
				cpustate->CF = (src >> (32-shift)) & 0x1;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 3:			/* RCR rm32, i8 */
				dst = ((src & ((uint32_t)0xffffffff << shift)) >> shift) |
					  ((src & ((uint32_t)0xffffffff >> (32-shift))) << (33-shift)) |
					  (cpustate->CF << (32-shift));
				cpustate->CF = (src >> (shift-1)) & 0x1;
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_CARRY_REG, CYCLES_ROTATE_CARRY_MEM);
				break;
			case 4:			/* SHL/SAL rm32, i8 */
			case 6:
				dst = src << shift;
				cpustate->CF = (src & (1 << (32-shift))) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 5:			/* SHR rm32, i8 */
				dst = src >> shift;
				cpustate->CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
			case 7:			/* SAR rm32, i8 */
				dst = (int32_t)src >> shift;
				cpustate->CF = (src & (1 << (shift-1))) ? 1 : 0;
				SetSZPF32(dst);
				CYCLES_RM(cpustate,modrm, CYCLES_ROTATE_REG, CYCLES_ROTATE_MEM);
				break;
		}

	}
	return dst;
}



static void I386OP(adc_rm32_r32)(i386_state *cpustate)		// Opcode 0x11
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = ADC32(cpustate, dst, src, cpustate->CF);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = ADC32(cpustate, dst, src, cpustate->CF);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(adc_r32_rm32)(i386_state *cpustate)		// Opcode 0x13
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = ADC32(cpustate, dst, src, cpustate->CF);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = ADC32(cpustate, dst, src, cpustate->CF);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(adc_eax_i32)(i386_state *cpustate)		// Opcode 0x15
{
	uint32_t src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = ADC32(cpustate, dst, src, cpustate->CF);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(add_rm32_r32)(i386_state *cpustate)		// Opcode 0x01
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = ADD32(cpustate,dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = ADD32(cpustate,dst, src);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(add_r32_rm32)(i386_state *cpustate)		// Opcode 0x03
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = ADD32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = ADD32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(add_eax_i32)(i386_state *cpustate)		// Opcode 0x05
{
	uint32_t src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = ADD32(cpustate,dst, src);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(and_rm32_r32)(i386_state *cpustate)		// Opcode 0x21
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = AND32(cpustate,dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = AND32(cpustate,dst, src);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(and_r32_rm32)(i386_state *cpustate)		// Opcode 0x23
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = AND32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = AND32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(and_eax_i32)(i386_state *cpustate)		// Opcode 0x25
{
	uint32_t src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = AND32(cpustate,dst, src);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(bsf_r32_rm32)(i386_state *cpustate)		// Opcode 0x0f bc
{
	uint32_t src, dst, temp;
	uint8_t modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = READ32(cpustate,ea);
	}

	dst = 0;

	if( src == 0 ) {
		cpustate->ZF = 1;
	} else {
		cpustate->ZF = 0;
		temp = 0;
		while( (src & (1 << temp)) == 0 ) {
			temp++;
			dst = temp;
			CYCLES(cpustate,CYCLES_BSF);
		}
	}
	CYCLES(cpustate,CYCLES_BSF_BASE);
	STORE_REG32(modrm, dst);
}

static void I386OP(bsr_r32_rm32)(i386_state *cpustate)		// Opcode 0x0f bd
{
	uint32_t src, dst, temp;
	uint8_t modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = READ32(cpustate,ea);
	}

	dst = 0;

	if( src == 0 ) {
		cpustate->ZF = 1;
	} else {
		cpustate->ZF = 0;
		dst = temp = 31;
		while( (src & (1 << temp)) == 0 ) {
			temp--;
			dst = temp;
			CYCLES(cpustate,CYCLES_BSR);
		}
	}
	CYCLES(cpustate,CYCLES_BSR_BASE);
	STORE_REG32(modrm, dst);
}

static void I386OP(bt_rm32_r32)(i386_state *cpustate)		// Opcode 0x0f a3
{
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		uint32_t dst = LOAD_RM32(modrm);
		uint32_t bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;

		CYCLES(cpustate,CYCLES_BT_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		uint32_t dst = READ32(cpustate,ea);
		uint32_t bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;

		CYCLES(cpustate,CYCLES_BT_REG_MEM);
	}
}

static void I386OP(btc_rm32_r32)(i386_state *cpustate)		// Opcode 0x0f bb
{
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		uint32_t dst = LOAD_RM32(modrm);
		uint32_t bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst ^= (1 << bit);

		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_BTC_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		uint32_t dst = READ32(cpustate,ea);
		uint32_t bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst ^= (1 << bit);

		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_BTC_REG_MEM);
	}
}

static void I386OP(btr_rm32_r32)(i386_state *cpustate)		// Opcode 0x0f b3
{
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		uint32_t dst = LOAD_RM32(modrm);
		uint32_t bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst &= ~(1 << bit);

		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_BTR_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		uint32_t dst = READ32(cpustate,ea);
		uint32_t bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst &= ~(1 << bit);

		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_BTR_REG_MEM);
	}
}

static void I386OP(bts_rm32_r32)(i386_state *cpustate)		// Opcode 0x0f ab
{
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		uint32_t dst = LOAD_RM32(modrm);
		uint32_t bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst |= (1 << bit);

		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_BTS_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		uint32_t dst = READ32(cpustate,ea);
		uint32_t bit = LOAD_REG32(modrm);

		if( dst & (1 << bit) )
			cpustate->CF = 1;
		else
			cpustate->CF = 0;
		dst |= (1 << bit);

		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_BTS_REG_MEM);
	}
}

static void I386OP(call_abs32)(i386_state *cpustate)		// Opcode 0x9a
{
	uint32_t offset = FETCH32(cpustate);
	uint16_t ptr = FETCH16(cpustate);

	if( PROTECTED_MODE ) {
		/* TODO */
		fatalerror("i386: call_abs32 in protected mode unimplemented");
	} else {
		PUSH32(cpustate, cpustate->sreg[CS].selector );
		PUSH32(cpustate, cpustate->eip );
		cpustate->sreg[CS].selector = ptr;
		cpustate->eip = offset;
		i386_load_segment_descriptor(cpustate,CS);
	}
	CYCLES(cpustate,CYCLES_CALL_INTERSEG);
	CHANGE_PC(cpustate,cpustate->eip);
}

static void I386OP(call_rel32)(i386_state *cpustate)		// Opcode 0xe8
{
	int32_t disp = FETCH32(cpustate);

	PUSH32(cpustate, cpustate->eip );
	cpustate->eip += disp;
	CHANGE_PC(cpustate,cpustate->eip);
	CYCLES(cpustate,CYCLES_CALL);		/* TODO: Timing = 7 + m */
}

static void I386OP(cdq)(i386_state *cpustate)				// Opcode 0x99
{
	if( REG32(EAX) & 0x80000000 ) {
		REG32(EDX) = 0xffffffff;
	} else {
		REG32(EDX) = 0x00000000;
	}
	CYCLES(cpustate,CYCLES_CWD);
}

static void I386OP(cmp_rm32_r32)(i386_state *cpustate)		// Opcode 0x39
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		SUB32(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		SUB32(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_REG_MEM);
	}
}

static void I386OP(cmp_r32_rm32)(i386_state *cpustate)		// Opcode 0x3b
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		SUB32(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		SUB32(cpustate,dst, src);
		CYCLES(cpustate,CYCLES_CMP_MEM_REG);
	}
}

static void I386OP(cmp_eax_i32)(i386_state *cpustate)		// Opcode 0x3d
{
	uint32_t src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	SUB32(cpustate,dst, src);
	CYCLES(cpustate,CYCLES_CMP_IMM_ACC);
}

static void I386OP(cmpsd)(i386_state *cpustate)				// Opcode 0xa7
{
	uint32_t eas, ead, src, dst;
	if( cpustate->segment_prefix ) {
		eas = i386_translate(cpustate, cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI) );
	} else {
		eas = i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI) );
	}
	ead = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI) );
	src = READ32(cpustate,eas);
	dst = READ32(cpustate,ead);
	SUB32(cpustate,dst, src);
	BUMP_SI(cpustate,4);
	BUMP_DI(cpustate,4);
	CYCLES(cpustate,CYCLES_CMPS);
}

static void I386OP(cwde)(i386_state *cpustate)				// Opcode 0x98
{
	REG32(EAX) = (int32_t)((int16_t)REG16(AX));
	CYCLES(cpustate,CYCLES_CBW);
}

static void I386OP(dec_eax)(i386_state *cpustate)			// Opcode 0x48
{
	REG32(EAX) = DEC32(cpustate, REG32(EAX) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_ecx)(i386_state *cpustate)			// Opcode 0x49
{
	REG32(ECX) = DEC32(cpustate, REG32(ECX) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_edx)(i386_state *cpustate)			// Opcode 0x4a
{
	REG32(EDX) = DEC32(cpustate, REG32(EDX) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_ebx)(i386_state *cpustate)			// Opcode 0x4b
{
	REG32(EBX) = DEC32(cpustate, REG32(EBX) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_esp)(i386_state *cpustate)			// Opcode 0x4c
{
	REG32(ESP) = DEC32(cpustate, REG32(ESP) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_ebp)(i386_state *cpustate)			// Opcode 0x4d
{
	REG32(EBP) = DEC32(cpustate, REG32(EBP) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_esi)(i386_state *cpustate)			// Opcode 0x4e
{
	REG32(ESI) = DEC32(cpustate, REG32(ESI) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(dec_edi)(i386_state *cpustate)			// Opcode 0x4f
{
	REG32(EDI) = DEC32(cpustate, REG32(EDI) );
	CYCLES(cpustate,CYCLES_DEC_REG);
}

static void I386OP(imul_r32_rm32)(i386_state *cpustate)		// Opcode 0x0f af
{
	uint8_t modrm = FETCH(cpustate);
	int64_t result;
	int64_t src, dst;
	if( modrm >= 0xc0 ) {
		src = (int64_t)(int32_t)LOAD_RM32(modrm);
		CYCLES(cpustate,CYCLES_IMUL32_REG_REG);		/* TODO: Correct multiply timing */
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = (int64_t)(int32_t)READ32(cpustate,ea);
		CYCLES(cpustate,CYCLES_IMUL32_REG_REG);		/* TODO: Correct multiply timing */
	}

	dst = (int64_t)(int32_t)LOAD_REG32(modrm);
	result = src * dst;

	STORE_REG32(modrm, (uint32_t)result);

	cpustate->CF = cpustate->OF = !(result == (int64_t)(int32_t)result);
}

static void I386OP(imul_r32_rm32_i32)(i386_state *cpustate)	// Opcode 0x69
{
	uint8_t modrm = FETCH(cpustate);
	int64_t result;
	int64_t src, dst;
	if( modrm >= 0xc0 ) {
		dst = (int64_t)(int32_t)LOAD_RM32(modrm);
		CYCLES(cpustate,CYCLES_IMUL32_REG_IMM_REG);		/* TODO: Correct multiply timing */
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		dst = (int64_t)(int32_t)READ32(cpustate,ea);
		CYCLES(cpustate,CYCLES_IMUL32_MEM_IMM_REG);		/* TODO: Correct multiply timing */
	}

	src = (int64_t)(int32_t)FETCH32(cpustate);
	result = src * dst;

	STORE_REG32(modrm, (uint32_t)result);

	cpustate->CF = cpustate->OF = !(result == (int64_t)(int32_t)result);
}

static void I386OP(imul_r32_rm32_i8)(i386_state *cpustate)	// Opcode 0x6b
{
	uint8_t modrm = FETCH(cpustate);
	int64_t result;
	int64_t src, dst;
	if( modrm >= 0xc0 ) {
		dst = (int64_t)(int32_t)LOAD_RM32(modrm);
		CYCLES(cpustate,CYCLES_IMUL32_REG_IMM_REG);		/* TODO: Correct multiply timing */
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		dst = (int64_t)(int32_t)READ32(cpustate,ea);
		CYCLES(cpustate,CYCLES_IMUL32_MEM_IMM_REG);		/* TODO: Correct multiply timing */
	}

	src = (int64_t)(int8_t)FETCH(cpustate);
	result = src * dst;

	STORE_REG32(modrm, (uint32_t)result);

	cpustate->CF = cpustate->OF = !(result == (int64_t)(int32_t)result);
}

static void I386OP(in_eax_i8)(i386_state *cpustate)			// Opcode 0xe5
{
	uint16_t port = FETCH(cpustate);
	uint32_t data = READPORT32(port);
	REG32(EAX) = data;
	CYCLES(cpustate,CYCLES_IN_VAR);
}

static void I386OP(in_eax_dx)(i386_state *cpustate)			// Opcode 0xed
{
	uint16_t port = REG16(DX);
	uint32_t data = READPORT32(port);
	REG32(EAX) = data;
	CYCLES(cpustate,CYCLES_IN);
}

static void I386OP(inc_eax)(i386_state *cpustate)			// Opcode 0x40
{
	REG32(EAX) = INC32(cpustate, REG32(EAX) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_ecx)(i386_state *cpustate)			// Opcode 0x41
{
	REG32(ECX) = INC32(cpustate, REG32(ECX) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_edx)(i386_state *cpustate)			// Opcode 0x42
{
	REG32(EDX) = INC32(cpustate, REG32(EDX) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_ebx)(i386_state *cpustate)			// Opcode 0x43
{
	REG32(EBX) = INC32(cpustate, REG32(EBX) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_esp)(i386_state *cpustate)			// Opcode 0x44
{
	REG32(ESP) = INC32(cpustate, REG32(ESP) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_ebp)(i386_state *cpustate)			// Opcode 0x45
{
	REG32(EBP) = INC32(cpustate, REG32(EBP) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_esi)(i386_state *cpustate)			// Opcode 0x46
{
	REG32(ESI) = INC32(cpustate, REG32(ESI) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(inc_edi)(i386_state *cpustate)			// Opcode 0x47
{
	REG32(EDI) = INC32(cpustate, REG32(EDI) );
	CYCLES(cpustate,CYCLES_INC_REG);
}

static void I386OP(iret32)(i386_state *cpustate)			// Opcode 0xcf
{
	if( PROTECTED_MODE ) {
		/* TODO: Virtual 8086-mode */
		/* TODO: Nested task */
		/* TODO: #SS(0) exception */
		/* TODO: All the protection-related stuff... */
		cpustate->eip = POP32(cpustate);
		cpustate->sreg[CS].selector = POP32(cpustate) & 0xffff;
		set_flags(cpustate, POP32(cpustate) );
		i386_load_segment_descriptor(cpustate,CS);
		CHANGE_PC(cpustate,cpustate->eip);
	} else {
		/* TODO: #SS(0) exception */
		/* TODO: #GP(0) exception */
		cpustate->eip = POP32(cpustate);
		cpustate->sreg[CS].selector = POP32(cpustate) & 0xffff;
		set_flags(cpustate, POP32(cpustate) );
		i386_load_segment_descriptor(cpustate,CS);
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_IRET);
}

static void I386OP(ja_rel32)(i386_state *cpustate)			// Opcode 0x0f 87
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->CF == 0 && cpustate->ZF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jbe_rel32)(i386_state *cpustate)			// Opcode 0x0f 86
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->CF != 0 || cpustate->ZF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jc_rel32)(i386_state *cpustate)			// Opcode 0x0f 82
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->CF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jg_rel32)(i386_state *cpustate)			// Opcode 0x0f 8f
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->ZF == 0 && (cpustate->SF == cpustate->OF) ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jge_rel32)(i386_state *cpustate)			// Opcode 0x0f 8d
{
	int32_t disp = FETCH32(cpustate);
	if( (cpustate->SF == cpustate->OF) ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jl_rel32)(i386_state *cpustate)			// Opcode 0x0f 8c
{
	int32_t disp = FETCH32(cpustate);
	if( (cpustate->SF != cpustate->OF) ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jle_rel32)(i386_state *cpustate)			// Opcode 0x0f 8e
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->ZF != 0 || (cpustate->SF != cpustate->OF) ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnc_rel32)(i386_state *cpustate)			// Opcode 0x0f 83
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->CF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jno_rel32)(i386_state *cpustate)			// Opcode 0x0f 81
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->OF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnp_rel32)(i386_state *cpustate)			// Opcode 0x0f 8b
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->PF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jns_rel32)(i386_state *cpustate)			// Opcode 0x0f 89
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->SF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jnz_rel32)(i386_state *cpustate)			// Opcode 0x0f 85
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->ZF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jo_rel32)(i386_state *cpustate)			// Opcode 0x0f 80
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->OF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jp_rel32)(i386_state *cpustate)			// Opcode 0x0f 8a
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->PF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(js_rel32)(i386_state *cpustate)			// Opcode 0x0f 88
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->SF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jz_rel32)(i386_state *cpustate)			// Opcode 0x0f 84
{
	int32_t disp = FETCH32(cpustate);
	if( cpustate->ZF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP);		/* TODO: Timing = 7 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCC_FULL_DISP_NOBRANCH);
	}
}

static void I386OP(jcxz32)(i386_state *cpustate)			// Opcode 0xe3
{
	int8_t disp = FETCH(cpustate);
	if( REG32(ECX) == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
		CYCLES(cpustate,CYCLES_JCXZ);		/* TODO: Timing = 9 + m */
	} else {
		CYCLES(cpustate,CYCLES_JCXZ_NOBRANCH);
	}
}

static void I386OP(jmp_rel32)(i386_state *cpustate)			// Opcode 0xe9
{
	uint32_t disp = FETCH32(cpustate);
	/* TODO: Segment limit */
	cpustate->eip += disp;
	CHANGE_PC(cpustate,cpustate->eip);
	CYCLES(cpustate,CYCLES_JMP);		/* TODO: Timing = 7 + m */
}

static void I386OP(jmp_abs32)(i386_state *cpustate)			// Opcode 0xea
{
	uint32_t address = FETCH32(cpustate);
	uint16_t segment = FETCH16(cpustate);

	if( PROTECTED_MODE ) {
		/* TODO: #GP */
		/* TODO: access rights, etc. */
		cpustate->eip = address;
		cpustate->sreg[CS].selector = segment;
		i386_load_segment_descriptor(cpustate,CS);
		CHANGE_PC(cpustate,cpustate->eip);
	} else {
		cpustate->eip = address;
		cpustate->sreg[CS].selector = segment;
		i386_load_segment_descriptor(cpustate,CS);
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_JMP_INTERSEG);
}

static void I386OP(lea32)(i386_state *cpustate)				// Opcode 0x8d
{
	uint8_t modrm = FETCH(cpustate);
	uint32_t ea = GetNonTranslatedEA(cpustate,modrm);
	if (!cpustate->address_size)
	{
		ea &= 0xffff;
	}
	STORE_REG32(modrm, ea);
	CYCLES(cpustate,CYCLES_LEA);
}

static void I386OP(enter32)(i386_state *cpustate)			// Opcode 0xc8
{
	uint16_t framesize = FETCH16(cpustate);
	uint8_t level = FETCH(cpustate) % 32;
	uint8_t x;
	uint32_t frameptr;
	PUSH32(cpustate,REG32(EBP));
	if(!STACK_32BIT)
		frameptr = REG16(SP);
	else
		frameptr = REG32(ESP);

	if(level > 0)
	{
		for(x=1;x<level-1;x++)
		{
			REG32(EBP) -= 4;
			PUSH32(cpustate,READ32(cpustate,REG32(EBP)));
		}
		PUSH32(cpustate,frameptr);
	}
	REG32(EBP) = frameptr;
	if(!STACK_32BIT)
		REG16(SP) -= framesize;
	else
		REG32(ESP) -= framesize;
	CYCLES(cpustate,CYCLES_ENTER);
}

static void I386OP(leave32)(i386_state *cpustate)			// Opcode 0xc9
{
	if(!STACK_32BIT)
		REG16(SP) = REG16(BP);
	else
		REG32(ESP) = REG32(EBP);
	REG32(EBP) = POP32(cpustate);
	CYCLES(cpustate,CYCLES_LEAVE);
}

static void I386OP(lodsd)(i386_state *cpustate)				// Opcode 0xad
{
	uint32_t eas;
	if( cpustate->segment_prefix ) {
		eas = i386_translate(cpustate, cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI) );
	} else {
		eas = i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI) );
	}
	REG32(EAX) = READ32(cpustate,eas);
	BUMP_SI(cpustate,4);
	CYCLES(cpustate,CYCLES_LODS);
}

static void I386OP(loop32)(i386_state *cpustate)			// Opcode 0xe2
{
	int8_t disp = FETCH(cpustate);
	int32_t reg = (cpustate->address_size)?--REG32(ECX):--REG16(CX);
	if( reg != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_LOOP);		/* TODO: Timing = 11 + m */
}

static void I386OP(loopne32)(i386_state *cpustate)			// Opcode 0xe0
{
	int8_t disp = FETCH(cpustate);
	int32_t reg = (cpustate->address_size)?--REG32(ECX):--REG16(CX);
	if( reg != 0 && cpustate->ZF == 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_LOOPNZ);		/* TODO: Timing = 11 + m */
}

static void I386OP(loopz32)(i386_state *cpustate)			// Opcode 0xe1
{
	int8_t disp = FETCH(cpustate);
	int32_t reg = (cpustate->address_size)?--REG32(ECX):--REG16(CX);
	if( reg != 0 && cpustate->ZF != 0 ) {
		cpustate->eip += disp;
		CHANGE_PC(cpustate,cpustate->eip);
	}
	CYCLES(cpustate,CYCLES_LOOPZ);		/* TODO: Timing = 11 + m */
}

static void I386OP(mov_rm32_r32)(i386_state *cpustate)		// Opcode 0x89
{
	uint32_t src;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		STORE_RM32(modrm, src);
		CYCLES(cpustate,CYCLES_MOV_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = LOAD_REG32(modrm);
		WRITE32(cpustate,ea, src);
		CYCLES(cpustate,CYCLES_MOV_REG_MEM);
	}
}

static void I386OP(mov_r32_rm32)(i386_state *cpustate)		// Opcode 0x8b
{
	uint32_t src;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOV_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = READ32(cpustate,ea);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOV_MEM_REG);
	}
}

static void I386OP(mov_rm32_i32)(i386_state *cpustate)		// Opcode 0xc7
{
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		uint32_t value = FETCH32(cpustate);
		STORE_RM32(modrm, value);
		CYCLES(cpustate,CYCLES_MOV_IMM_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		uint32_t value = FETCH32(cpustate);
		WRITE32(cpustate,ea, value);
		CYCLES(cpustate,CYCLES_MOV_IMM_MEM);
	}
}

static void I386OP(mov_eax_m32)(i386_state *cpustate)		// Opcode 0xa1
{
	uint32_t offset, ea;
	if( cpustate->address_size ) {
		offset = FETCH32(cpustate);
	} else {
		offset = FETCH16(cpustate);
	}
	if( cpustate->segment_prefix ) {
		ea = i386_translate(cpustate, cpustate->segment_override, offset );
	} else {
		ea = i386_translate(cpustate, DS, offset );
	}
	REG32(EAX) = READ32(cpustate,ea);
	CYCLES(cpustate,CYCLES_MOV_MEM_ACC);
}

static void I386OP(mov_m32_eax)(i386_state *cpustate)		// Opcode 0xa3
{
	uint32_t offset, ea;
	if( cpustate->address_size ) {
		offset = FETCH32(cpustate);
	} else {
		offset = FETCH16(cpustate);
	}
	if( cpustate->segment_prefix ) {
		ea = i386_translate(cpustate, cpustate->segment_override, offset );
	} else {
		ea = i386_translate(cpustate, DS, offset );
	}
	WRITE32(cpustate, ea, REG32(EAX) );
	CYCLES(cpustate,CYCLES_MOV_ACC_MEM);
}

static void I386OP(mov_eax_i32)(i386_state *cpustate)		// Opcode 0xb8
{
	REG32(EAX) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_ecx_i32)(i386_state *cpustate)		// Opcode 0xb9
{
	REG32(ECX) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_edx_i32)(i386_state *cpustate)		// Opcode 0xba
{
	REG32(EDX) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_ebx_i32)(i386_state *cpustate)		// Opcode 0xbb
{
	REG32(EBX) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_esp_i32)(i386_state *cpustate)		// Opcode 0xbc
{
	REG32(ESP) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_ebp_i32)(i386_state *cpustate)		// Opcode 0xbd
{
	REG32(EBP) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_esi_i32)(i386_state *cpustate)		// Opcode 0xbe
{
	REG32(ESI) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(mov_edi_i32)(i386_state *cpustate)		// Opcode 0xbf
{
	REG32(EDI) = FETCH32(cpustate);
	CYCLES(cpustate,CYCLES_MOV_IMM_REG);
}

static void I386OP(movsd)(i386_state *cpustate)				// Opcode 0xa5
{
	uint32_t eas, ead, v;
	if( cpustate->segment_prefix ) {
		eas = i386_translate(cpustate, cpustate->segment_override, cpustate->address_size ? REG32(ESI) : REG16(SI) );
	} else {
		eas = i386_translate(cpustate, DS, cpustate->address_size ? REG32(ESI) : REG16(SI) );
	}
	ead = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI) );
	v = READ32(cpustate,eas);
	WRITE32(cpustate,ead, v);
	BUMP_SI(cpustate,4);
	BUMP_DI(cpustate,4);
	CYCLES(cpustate,CYCLES_MOVS);
}

static void I386OP(movsx_r32_rm8)(i386_state *cpustate)		// Opcode 0x0f be
{
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int32_t src = (int8_t)LOAD_RM8(modrm);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVSX_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		int32_t src = (int8_t)READ8(cpustate,ea);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVSX_MEM_REG);
	}
}

static void I386OP(movsx_r32_rm16)(i386_state *cpustate)	// Opcode 0x0f bf
{
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		int32_t src = (int16_t)LOAD_RM16(modrm);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVSX_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		int32_t src = (int16_t)READ16(cpustate,ea);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVSX_MEM_REG);
	}
}

static void I386OP(movzx_r32_rm8)(i386_state *cpustate)		// Opcode 0x0f b6
{
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		uint32_t src = (uint8_t)LOAD_RM8(modrm);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVZX_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		uint32_t src = (uint8_t)READ8(cpustate,ea);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVZX_MEM_REG);
	}
}

static void I386OP(movzx_r32_rm16)(i386_state *cpustate)	// Opcode 0x0f b7
{
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		uint32_t src = (uint16_t)LOAD_RM16(modrm);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVZX_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		uint32_t src = (uint16_t)READ16(cpustate,ea);
		STORE_REG32(modrm, src);
		CYCLES(cpustate,CYCLES_MOVZX_MEM_REG);
	}
}

static void I386OP(or_rm32_r32)(i386_state *cpustate)		// Opcode 0x09
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = OR32(cpustate,dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = OR32(cpustate,dst, src);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(or_r32_rm32)(i386_state *cpustate)		// Opcode 0x0b
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = OR32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = OR32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(or_eax_i32)(i386_state *cpustate)		// Opcode 0x0d
{
	uint32_t src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = OR32(cpustate,dst, src);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(out_eax_i8)(i386_state *cpustate)		// Opcode 0xe7
{
	uint16_t port = FETCH(cpustate);
	uint32_t data = REG32(EAX);
	WRITEPORT32(port, data);
	CYCLES(cpustate,CYCLES_OUT_VAR);
}

static void I386OP(out_eax_dx)(i386_state *cpustate)		// Opcode 0xef
{
	uint16_t port = REG16(DX);
	uint32_t data = REG32(EAX);
	WRITEPORT32(port, data);
	CYCLES(cpustate,CYCLES_OUT);
}

static void I386OP(pop_eax)(i386_state *cpustate)			// Opcode 0x58
{
	REG32(EAX) = POP32(cpustate);
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_ecx)(i386_state *cpustate)			// Opcode 0x59
{
	REG32(ECX) = POP32(cpustate);
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_edx)(i386_state *cpustate)			// Opcode 0x5a
{
	REG32(EDX) = POP32(cpustate);
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_ebx)(i386_state *cpustate)			// Opcode 0x5b
{
	REG32(EBX) = POP32(cpustate);
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_esp)(i386_state *cpustate)			// Opcode 0x5c
{
	REG32(ESP) = POP32(cpustate);
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_ebp)(i386_state *cpustate)			// Opcode 0x5d
{
	REG32(EBP) = POP32(cpustate);
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_esi)(i386_state *cpustate)			// Opcode 0x5e
{
	REG32(ESI) = POP32(cpustate);
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_edi)(i386_state *cpustate)			// Opcode 0x5f
{
	REG32(EDI) = POP32(cpustate);
	CYCLES(cpustate,CYCLES_POP_REG_SHORT);
}

static void I386OP(pop_ds32)(i386_state *cpustate)			// Opcode 0x1f
{
	cpustate->sreg[DS].selector = POP32(cpustate);
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(cpustate,DS);
	} else {
		i386_load_segment_descriptor(cpustate,DS);
	}
	CYCLES(cpustate,CYCLES_POP_SREG);
}

static void I386OP(pop_es32)(i386_state *cpustate)			// Opcode 0x07
{
	cpustate->sreg[ES].selector = POP32(cpustate);
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(cpustate,ES);
	} else {
		i386_load_segment_descriptor(cpustate,ES);
	}
	CYCLES(cpustate,CYCLES_POP_SREG);
}

static void I386OP(pop_fs32)(i386_state *cpustate)			// Opcode 0x0f a1
{
	cpustate->sreg[FS].selector = POP32(cpustate);
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(cpustate,FS);
	} else {
		i386_load_segment_descriptor(cpustate,FS);
	}
	CYCLES(cpustate,CYCLES_POP_SREG);
}

static void I386OP(pop_gs32)(i386_state *cpustate)			// Opcode 0x0f a9
{
	cpustate->sreg[GS].selector = POP32(cpustate);
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(cpustate,GS);
	} else {
		i386_load_segment_descriptor(cpustate,GS);
	}
	CYCLES(cpustate,CYCLES_POP_SREG);
}

static void I386OP(pop_ss32)(i386_state *cpustate)			// Opcode 0x17
{
	cpustate->sreg[SS].selector = POP32(cpustate);
	if( PROTECTED_MODE ) {
		i386_load_segment_descriptor(cpustate,SS);
	} else {
		i386_load_segment_descriptor(cpustate,SS);
	}
	CYCLES(cpustate,CYCLES_POP_SREG);
}

static void I386OP(pop_rm32)(i386_state *cpustate)			// Opcode 0x8f
{
	uint8_t modrm = FETCH(cpustate);
	uint32_t value = POP32(cpustate);

	if( modrm >= 0xc0 ) {
		STORE_RM32(modrm, value);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		WRITE32(cpustate,ea, value);
	}
	CYCLES(cpustate,CYCLES_POP_RM);
}

static void I386OP(popad)(i386_state *cpustate)				// Opcode 0x61
{
	REG32(EDI) = POP32(cpustate);
	REG32(ESI) = POP32(cpustate);
	REG32(EBP) = POP32(cpustate);
	REG32(ESP) += 4;
	REG32(EBX) = POP32(cpustate);
	REG32(EDX) = POP32(cpustate);
	REG32(ECX) = POP32(cpustate);
	REG32(EAX) = POP32(cpustate);
	CYCLES(cpustate,CYCLES_POPA);
}

static void I386OP(popfd)(i386_state *cpustate)				// Opcode 0x9d
{
	uint32_t value = POP32(cpustate);
	set_flags(cpustate,value);
	CYCLES(cpustate,CYCLES_POPF);
}

static void I386OP(push_eax)(i386_state *cpustate)			// Opcode 0x50
{
	PUSH32(cpustate, REG32(EAX) );
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_ecx)(i386_state *cpustate)			// Opcode 0x51
{
	PUSH32(cpustate, REG32(ECX) );
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_edx)(i386_state *cpustate)			// Opcode 0x52
{
	PUSH32(cpustate, REG32(EDX) );
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_ebx)(i386_state *cpustate)			// Opcode 0x53
{
	PUSH32(cpustate, REG32(EBX) );
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_esp)(i386_state *cpustate)			// Opcode 0x54
{
	PUSH32(cpustate, REG32(ESP) );
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_ebp)(i386_state *cpustate)			// Opcode 0x55
{
	PUSH32(cpustate, REG32(EBP) );
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_esi)(i386_state *cpustate)			// Opcode 0x56
{
	PUSH32(cpustate, REG32(ESI) );
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_edi)(i386_state *cpustate)			// Opcode 0x57
{
	PUSH32(cpustate, REG32(EDI) );
	CYCLES(cpustate,CYCLES_PUSH_REG_SHORT);
}

static void I386OP(push_cs32)(i386_state *cpustate)			// Opcode 0x0e
{
	PUSH32(cpustate, cpustate->sreg[CS].selector );
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_ds32)(i386_state *cpustate)			// Opcode 0x1e
{
	PUSH32(cpustate, cpustate->sreg[DS].selector );
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_es32)(i386_state *cpustate)			// Opcode 0x06
{
	PUSH32(cpustate, cpustate->sreg[ES].selector );
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_fs32)(i386_state *cpustate)			// Opcode 0x0f a0
{
	PUSH32(cpustate, cpustate->sreg[FS].selector );
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_gs32)(i386_state *cpustate)			// Opcode 0x0f a8
{
	PUSH32(cpustate, cpustate->sreg[GS].selector );
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_ss32)(i386_state *cpustate)			// Opcode 0x16
{
	PUSH32(cpustate, cpustate->sreg[SS].selector );
	CYCLES(cpustate,CYCLES_PUSH_SREG);
}

static void I386OP(push_i32)(i386_state *cpustate)			// Opcode 0x68
{
	uint32_t value = FETCH32(cpustate);
	PUSH32(cpustate,value);
	CYCLES(cpustate,CYCLES_PUSH_IMM);
}

static void I386OP(pushad)(i386_state *cpustate)			// Opcode 0x60
{
	uint32_t temp = REG32(ESP);
	PUSH32(cpustate, REG32(EAX) );
	PUSH32(cpustate, REG32(ECX) );
	PUSH32(cpustate, REG32(EDX) );
	PUSH32(cpustate, REG32(EBX) );
	PUSH32(cpustate, temp );
	PUSH32(cpustate, REG32(EBP) );
	PUSH32(cpustate, REG32(ESI) );
	PUSH32(cpustate, REG32(EDI) );
	CYCLES(cpustate,CYCLES_PUSHA);
}

static void I386OP(pushfd)(i386_state *cpustate)			// Opcode 0x9c
{
	PUSH32(cpustate, get_flags(cpustate) & 0x00fcffff );
	CYCLES(cpustate,CYCLES_PUSHF);
}

static void I386OP(ret_near32_i16)(i386_state *cpustate)	// Opcode 0xc2
{
	int16_t disp = FETCH16(cpustate);
	cpustate->eip = POP32(cpustate);
	REG32(ESP) += disp;
	CHANGE_PC(cpustate,cpustate->eip);
	CYCLES(cpustate,CYCLES_RET_IMM);		/* TODO: Timing = 10 + m */
}

static void I386OP(ret_near32)(i386_state *cpustate)		// Opcode 0xc3
{
	cpustate->eip = POP32(cpustate);
	CHANGE_PC(cpustate,cpustate->eip);
	CYCLES(cpustate,CYCLES_RET);		/* TODO: Timing = 10 + m */
}

static void I386OP(sbb_rm32_r32)(i386_state *cpustate)		// Opcode 0x19
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = SBB32(cpustate, dst, src, cpustate->CF);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = SBB32(cpustate, dst, src, cpustate->CF);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sbb_r32_rm32)(i386_state *cpustate)		// Opcode 0x1b
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = SBB32(cpustate, dst, src, cpustate->CF);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = SBB32(cpustate, dst, src, cpustate->CF);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sbb_eax_i32)(i386_state *cpustate)		// Opcode 0x1d
{
	uint32_t src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = SBB32(cpustate, dst, src, cpustate->CF);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(scasd)(i386_state *cpustate)				// Opcode 0xaf
{
	uint32_t eas, src, dst;
	eas = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI) );
	src = READ32(cpustate,eas);
	dst = REG32(EAX);
	SUB32(cpustate,dst, src);
	BUMP_DI(cpustate,4);
	CYCLES(cpustate,CYCLES_SCAS);
}

static void I386OP(shld32_i8)(i386_state *cpustate)			// Opcode 0x0f a4
{
	/* TODO: Correct flags */
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		uint32_t dst = LOAD_RM32(modrm);
		uint32_t upper = LOAD_REG32(modrm);
		uint8_t shift = FETCH(cpustate);
		if( shift > 31 || shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_SHLD_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		uint32_t dst = READ32(cpustate,ea);
		uint32_t upper = LOAD_REG32(modrm);
		uint8_t shift = FETCH(cpustate);
		if( shift > 31 || shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			SetSZPF32(dst);
		}
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_SHLD_MEM);
	}
}

static void I386OP(shld32_cl)(i386_state *cpustate)			// Opcode 0x0f a5
{
	/* TODO: Correct flags */
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		uint32_t dst = LOAD_RM32(modrm);
		uint32_t upper = LOAD_REG32(modrm);
		uint8_t shift = REG8(CL);
		if( shift > 31 || shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_SHLD_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		uint32_t dst = READ32(cpustate,ea);
		uint32_t upper = LOAD_REG32(modrm);
		uint8_t shift = REG8(CL);
		if( shift > 31 || shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (32-shift))) ? 1 : 0;
			dst = (dst << shift) | (upper >> (32-shift));
			SetSZPF32(dst);
		}
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_SHLD_MEM);
	}
}

static void I386OP(shrd32_i8)(i386_state *cpustate)			// Opcode 0x0f ac
{
	/* TODO: Correct flags */
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		uint32_t dst = LOAD_RM32(modrm);
		uint32_t upper = LOAD_REG32(modrm);
		uint8_t shift = FETCH(cpustate);
		if( shift > 31 || shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_SHRD_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		uint32_t dst = READ32(cpustate,ea);
		uint32_t upper = LOAD_REG32(modrm);
		uint8_t shift = FETCH(cpustate);
		if( shift > 31 || shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			SetSZPF32(dst);
		}
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_SHRD_MEM);
	}
}

static void I386OP(shrd32_cl)(i386_state *cpustate)			// Opcode 0x0f ad
{
	/* TODO: Correct flags */
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		uint32_t dst = LOAD_RM32(modrm);
		uint32_t upper = LOAD_REG32(modrm);
		uint8_t shift = REG8(CL);
		if( shift > 31 || shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			SetSZPF32(dst);
		}
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_SHRD_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		uint32_t dst = READ32(cpustate,ea);
		uint32_t upper = LOAD_REG32(modrm);
		uint8_t shift = REG8(CL);
		if( shift > 31 || shift == 0 ) {

		} else {
			cpustate->CF = (dst & (1 << (shift-1))) ? 1 : 0;
			dst = (dst >> shift) | (upper << (32-shift));
			SetSZPF32(dst);
		}
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_SHRD_MEM);
	}
}

static void I386OP(stosd)(i386_state *cpustate)				// Opcode 0xab
{
	uint32_t eas = i386_translate(cpustate, ES, cpustate->address_size ? REG32(EDI) : REG16(DI) );
	WRITE32(cpustate,eas, REG32(EAX));
	BUMP_DI(cpustate,4);
	CYCLES(cpustate,CYCLES_STOS);
}

static void I386OP(sub_rm32_r32)(i386_state *cpustate)		// Opcode 0x29
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = SUB32(cpustate,dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = SUB32(cpustate,dst, src);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(sub_r32_rm32)(i386_state *cpustate)		// Opcode 0x2b
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = SUB32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = SUB32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(sub_eax_i32)(i386_state *cpustate)		// Opcode 0x2d
{
	uint32_t src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = SUB32(cpustate,dst, src);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}

static void I386OP(test_eax_i32)(i386_state *cpustate)		// Opcode 0xa9
{
	uint32_t src = FETCH32(cpustate);
	uint32_t dst = REG32(EAX);
	dst = src & dst;
	SetSZPF32(dst);
	cpustate->CF = 0;
	cpustate->OF = 0;
	CYCLES(cpustate,CYCLES_TEST_IMM_ACC);
}

static void I386OP(test_rm32_r32)(i386_state *cpustate)		// Opcode 0x85
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = src & dst;
		SetSZPF32(dst);
		cpustate->CF = 0;
		cpustate->OF = 0;
		CYCLES(cpustate,CYCLES_TEST_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = src & dst;
		SetSZPF32(dst);
		cpustate->CF = 0;
		cpustate->OF = 0;
		CYCLES(cpustate,CYCLES_TEST_REG_MEM);
	}
}

static void I386OP(xchg_eax_ecx)(i386_state *cpustate)		// Opcode 0x91
{
	uint32_t temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(ECX);
	REG32(ECX) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_edx)(i386_state *cpustate)		// Opcode 0x92
{
	uint32_t temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EDX);
	REG32(EDX) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_ebx)(i386_state *cpustate)		// Opcode 0x93
{
	uint32_t temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EBX);
	REG32(EBX) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_esp)(i386_state *cpustate)		// Opcode 0x94
{
	uint32_t temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(ESP);
	REG32(ESP) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_ebp)(i386_state *cpustate)		// Opcode 0x95
{
	uint32_t temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EBP);
	REG32(EBP) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_esi)(i386_state *cpustate)		// Opcode 0x96
{
	uint32_t temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(ESI);
	REG32(ESI) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_eax_edi)(i386_state *cpustate)		// Opcode 0x97
{
	uint32_t temp;
	temp = REG32(EAX);
	REG32(EAX) = REG32(EDI);
	REG32(EDI) = temp;
	CYCLES(cpustate,CYCLES_XCHG_REG_REG);
}

static void I386OP(xchg_r32_rm32)(i386_state *cpustate)		// Opcode 0x87
{
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		uint32_t src = LOAD_RM32(modrm);
		uint32_t dst = LOAD_REG32(modrm);
		STORE_REG32(modrm, src);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_XCHG_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		uint32_t src = READ32(cpustate,ea);
		uint32_t dst = LOAD_REG32(modrm);
		STORE_REG32(modrm, src);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_XCHG_REG_MEM);
	}
}

static void I386OP(xor_rm32_r32)(i386_state *cpustate)		// Opcode 0x31
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_REG32(modrm);
		dst = LOAD_RM32(modrm);
		dst = XOR32(cpustate,dst, src);
		STORE_RM32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = LOAD_REG32(modrm);
		dst = READ32(cpustate,ea);
		dst = XOR32(cpustate,dst, src);
		WRITE32(cpustate,ea, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_MEM);
	}
}

static void I386OP(xor_r32_rm32)(i386_state *cpustate)		// Opcode 0x33
{
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);
	if( modrm >= 0xc0 ) {
		src = LOAD_RM32(modrm);
		dst = LOAD_REG32(modrm);
		dst = XOR32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_REG_REG);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		src = READ32(cpustate,ea);
		dst = LOAD_REG32(modrm);
		dst = XOR32(cpustate,dst, src);
		STORE_REG32(modrm, dst);
		CYCLES(cpustate,CYCLES_ALU_MEM_REG);
	}
}

static void I386OP(xor_eax_i32)(i386_state *cpustate)		// Opcode 0x35
{
	uint32_t src, dst;
	src = FETCH32(cpustate);
	dst = REG32(EAX);
	dst = XOR32(cpustate,dst, src);
	REG32(EAX) = dst;
	CYCLES(cpustate,CYCLES_ALU_IMM_ACC);
}



static void I386OP(group81_32)(i386_state *cpustate)		// Opcode 0x81
{
	uint32_t ea;
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:		// ADD Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = ADD32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = ADD32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:		// OR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = OR32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = OR32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:		// ADC Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = ADC32(cpustate, dst, src, cpustate->CF);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = ADC32(cpustate, dst, src, cpustate->CF);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:		// SBB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = SBB32(cpustate, dst, src, cpustate->CF);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = SBB32(cpustate, dst, src, cpustate->CF);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:		// AND Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = AND32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = AND32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:		// SUB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = SUB32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = SUB32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:		// XOR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				dst = XOR32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				dst = XOR32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:		// CMP Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = FETCH32(cpustate);
				SUB32(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = FETCH32(cpustate);
				SUB32(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

static void I386OP(group83_32)(i386_state *cpustate)		// Opcode 0x83
{
	uint32_t ea;
	uint32_t src, dst;
	uint8_t modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:		// ADD Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				dst = ADD32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				dst = ADD32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 1:		// OR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				dst = OR32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				dst = OR32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 2:		// ADC Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				dst = ADC32(cpustate, dst, src, cpustate->CF);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				dst = ADC32(cpustate, dst, src, cpustate->CF);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 3:		// SBB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = ((uint32_t)(int32_t)(int8_t)FETCH(cpustate));
				dst = SBB32(cpustate, dst, src, cpustate->CF);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = ((uint32_t)(int32_t)(int8_t)FETCH(cpustate));
				dst = SBB32(cpustate, dst, src, cpustate->CF);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 4:		// AND Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				dst = AND32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				dst = AND32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 5:		// SUB Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				dst = SUB32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				dst = SUB32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 6:		// XOR Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				dst = XOR32(cpustate,dst, src);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				dst = XOR32(cpustate,dst, src);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_ALU_REG_MEM);
			}
			break;
		case 7:		// CMP Rm32, i32
			if( modrm >= 0xc0 ) {
				dst = LOAD_RM32(modrm);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				SUB32(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_REG);
			} else {
				ea = GetEA(cpustate,modrm);
				dst = READ32(cpustate,ea);
				src = (uint32_t)(int32_t)(int8_t)FETCH(cpustate);
				SUB32(cpustate,dst, src);
				CYCLES(cpustate,CYCLES_CMP_REG_MEM);
			}
			break;
	}
}

static void I386OP(groupC1_32)(i386_state *cpustate)		// Opcode 0xc1
{
	uint32_t dst;
	uint8_t modrm = FETCH(cpustate);
	uint8_t shift;

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM32(modrm);
		shift = FETCH(cpustate) & 0x1f;
		dst = i386_shift_rotate32(cpustate, modrm, dst, shift);
		STORE_RM32(modrm, dst);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		dst = READ32(cpustate,ea);
		shift = FETCH(cpustate) & 0x1f;
		dst = i386_shift_rotate32(cpustate, modrm, dst, shift);
		WRITE32(cpustate,ea, dst);
	}
}

static void I386OP(groupD1_32)(i386_state *cpustate)		// Opcode 0xd1
{
	uint32_t dst;
	uint8_t modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM32(modrm);
		dst = i386_shift_rotate32(cpustate, modrm, dst, 1);
		STORE_RM32(modrm, dst);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		dst = READ32(cpustate,ea);
		dst = i386_shift_rotate32(cpustate, modrm, dst, 1);
		WRITE32(cpustate,ea, dst);
	}
}

static void I386OP(groupD3_32)(i386_state *cpustate)		// Opcode 0xd3
{
	uint32_t dst;
	uint8_t modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		dst = LOAD_RM32(modrm);
		dst = i386_shift_rotate32(cpustate, modrm, dst, REG8(CL));
		STORE_RM32(modrm, dst);
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		dst = READ32(cpustate,ea);
		dst = i386_shift_rotate32(cpustate, modrm, dst, REG8(CL));
		WRITE32(cpustate,ea, dst);
	}
}

static void I386OP(groupF7_32)(i386_state *cpustate)		// Opcode 0xf7
{
	uint8_t modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* TEST Rm32, i32 */
			if( modrm >= 0xc0 ) {
				uint32_t dst = LOAD_RM32(modrm);
				uint32_t src = FETCH32(cpustate);
				dst &= src;
				cpustate->CF = cpustate->OF = cpustate->AF = 0;
				SetSZPF32(dst);
				CYCLES(cpustate,CYCLES_TEST_IMM_REG);
			} else {
				uint32_t ea = GetEA(cpustate,modrm);
				uint32_t dst = READ32(cpustate,ea);
				uint32_t src = FETCH32(cpustate);
				dst &= src;
				cpustate->CF = cpustate->OF = cpustate->AF = 0;
				SetSZPF32(dst);
				CYCLES(cpustate,CYCLES_TEST_IMM_MEM);
			}
			break;
		case 2:			/* NOT Rm32 */
			if( modrm >= 0xc0 ) {
				uint32_t dst = LOAD_RM32(modrm);
				dst = ~dst;
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_NOT_REG);
			} else {
				uint32_t ea = GetEA(cpustate,modrm);
				uint32_t dst = READ32(cpustate,ea);
				dst = ~dst;
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_NOT_MEM);
			}
			break;
		case 3:			/* NEG Rm32 */
			if( modrm >= 0xc0 ) {
				uint32_t dst = LOAD_RM32(modrm);
				dst = SUB32(cpustate, 0, dst );
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_NEG_REG);
			} else {
				uint32_t ea = GetEA(cpustate,modrm);
				uint32_t dst = READ32(cpustate,ea);
				dst = SUB32(cpustate, 0, dst );
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_NEG_MEM);
			}
			break;
		case 4:			/* MUL EAX, Rm32 */
			{
				uint64_t result;
				uint32_t src, dst;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM32(modrm);
					CYCLES(cpustate,CYCLES_MUL32_ACC_REG);		/* TODO: Correct multiply timing */
				} else {
					uint32_t ea = GetEA(cpustate,modrm);
					src = READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_MUL32_ACC_MEM);		/* TODO: Correct multiply timing */
				}

				dst = REG32(EAX);
				result = (uint64_t)src * (uint64_t)dst;
				REG32(EDX) = (uint32_t)(result >> 32);
				REG32(EAX) = (uint32_t)result;

				cpustate->CF = cpustate->OF = (REG32(EDX) != 0);
			}
			break;
		case 5:			/* IMUL EAX, Rm32 */
			{
				int64_t result;
				int64_t src, dst;
				if( modrm >= 0xc0 ) {
					src = (int64_t)(int32_t)LOAD_RM32(modrm);
					CYCLES(cpustate,CYCLES_IMUL32_ACC_REG);		/* TODO: Correct multiply timing */
				} else {
					uint32_t ea = GetEA(cpustate,modrm);
					src = (int64_t)(int32_t)READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_IMUL32_ACC_MEM);		/* TODO: Correct multiply timing */
				}

				dst = (int64_t)(int32_t)REG32(EAX);
				result = src * dst;

				REG32(EDX) = (uint32_t)(result >> 32);
				REG32(EAX) = (uint32_t)result;

				cpustate->CF = cpustate->OF = !(result == (int64_t)(int32_t)result);
			}
			break;
		case 6:			/* DIV EAX, Rm32 */
			{
				uint64_t quotient, remainder, result;
				uint32_t src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM32(modrm);
					CYCLES(cpustate,CYCLES_DIV32_ACC_REG);
				} else {
					uint32_t ea = GetEA(cpustate,modrm);
					src = READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_DIV32_ACC_MEM);
				}

				quotient = ((uint64_t)(REG32(EDX)) << 32) | (uint64_t)(REG32(EAX));
				if( src ) {
					remainder = quotient % (uint64_t)src;
					result = quotient / (uint64_t)src;
					if( result > 0xffffffff ) {
						/* TODO: Divide error */
					} else {
						REG32(EDX) = (uint32_t)remainder;
						REG32(EAX) = (uint32_t)result;
					}
				} else {
					/* TODO: Divide by zero */
				}
			}
			break;
		case 7:			/* IDIV EAX, Rm32 */
			{
				int64_t quotient, remainder, result;
				uint32_t src;
				if( modrm >= 0xc0 ) {
					src = LOAD_RM32(modrm);
					CYCLES(cpustate,CYCLES_IDIV32_ACC_REG);
				} else {
					uint32_t ea = GetEA(cpustate,modrm);
					src = READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_IDIV32_ACC_MEM);
				}

				quotient = (((int64_t)REG32(EDX)) << 32) | ((uint64_t)REG32(EAX));
				if( src ) {
					remainder = quotient % (int64_t)(int32_t)src;
					result = quotient / (int64_t)(int32_t)src;
					if( result > 0xffffffff ) {
						/* TODO: Divide error */
					} else {
						REG32(EDX) = (uint32_t)remainder;
						REG32(EAX) = (uint32_t)result;
					}
				} else {
					/* TODO: Divide by zero */
				}
			}
			break;
	}
}

static void I386OP(groupFF_32)(i386_state *cpustate)		// Opcode 0xff
{
	uint8_t modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* INC Rm32 */
			if( modrm >= 0xc0 ) {
				uint32_t dst = LOAD_RM32(modrm);
				dst = INC32(cpustate,dst);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_INC_REG);
			} else {
				uint32_t ea = GetEA(cpustate,modrm);
				uint32_t dst = READ32(cpustate,ea);
				dst = INC32(cpustate,dst);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_INC_MEM);
			}
			break;
		case 1:			/* DEC Rm32 */
			if( modrm >= 0xc0 ) {
				uint32_t dst = LOAD_RM32(modrm);
				dst = DEC32(cpustate,dst);
				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_DEC_REG);
			} else {
				uint32_t ea = GetEA(cpustate,modrm);
				uint32_t dst = READ32(cpustate,ea);
				dst = DEC32(cpustate,dst);
				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_DEC_MEM);
			}
			break;
		case 2:			/* CALL Rm32 */
			{
				uint32_t address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					CYCLES(cpustate,CYCLES_CALL_REG);		/* TODO: Timing = 7 + m */
				} else {
					uint32_t ea = GetEA(cpustate,modrm);
					address = READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_CALL_MEM);		/* TODO: Timing = 10 + m */
				}
				PUSH32(cpustate, cpustate->eip );
				cpustate->eip = address;
				CHANGE_PC(cpustate,cpustate->eip);
			}
			break;
		case 3:			/* CALL FAR Rm32 */
			{
				uint16_t selector;
				uint32_t address;
				if( modrm >= 0xc0 ) {
					fatalerror("i386: groupFF_32 /%d: NYI", (modrm >> 3) & 0x7);
				} else {
					uint32_t ea = GetEA(cpustate,modrm);
					address = READ32(cpustate,ea + 0);
					selector = READ16(cpustate,ea + 4);
					CYCLES(cpustate,CYCLES_CALL_MEM_INTERSEG);		/* TODO: Timing = 10 + m */
				}
				PUSH32(cpustate, cpustate->sreg[CS].selector );
				PUSH32(cpustate, cpustate->eip );
				cpustate->sreg[CS].selector = selector;
				cpustate->performed_intersegment_jump = 1;
				i386_load_segment_descriptor(cpustate, CS );
				cpustate->eip = address;
				CHANGE_PC(cpustate,cpustate->eip);
			}
			break;
		case 4:			/* JMP Rm32 */
			{
				uint32_t address;
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					CYCLES(cpustate,CYCLES_JMP_REG);		/* TODO: Timing = 7 + m */
				} else {
					uint32_t ea = GetEA(cpustate,modrm);
					address = READ32(cpustate,ea);
					CYCLES(cpustate,CYCLES_JMP_MEM);		/* TODO: Timing = 10 + m */
				}
				cpustate->eip = address;
				CHANGE_PC(cpustate,cpustate->eip);
			}
			break;
		case 5:			/* JMP FAR Rm32 */
			{
				uint16_t selector;
				uint32_t address;
				if( modrm >= 0xc0 ) {
					fatalerror("i386: groupFF_32 /%d: NYI", (modrm >> 3) & 0x7);
				} else {
					uint32_t ea = GetEA(cpustate,modrm);
					address = READ32(cpustate,ea + 0);
					selector = READ16(cpustate,ea + 4);
					CYCLES(cpustate,CYCLES_JMP_MEM_INTERSEG);		/* TODO: Timing = 10 + m */
				}
				cpustate->sreg[CS].selector = selector;
				cpustate->performed_intersegment_jump = 1;
				i386_load_segment_descriptor(cpustate, CS );
				cpustate->eip = address;
				CHANGE_PC(cpustate,cpustate->eip);
			}
			break;
		case 6:			/* PUSH Rm32 */
			{
				uint32_t value;
				if( modrm >= 0xc0 ) {
					value = LOAD_RM32(modrm);
				} else {
					uint32_t ea = GetEA(cpustate,modrm);
					value = READ32(cpustate,ea);
				}
				PUSH32(cpustate,value);
				CYCLES(cpustate,CYCLES_PUSH_RM);
			}
			break;
		default:
			fatalerror("i386: groupFF_32 /%d unimplemented at %08X", (modrm >> 3) & 0x7, cpustate->pc-2);
			break;
	}
}

static void I386OP(group0F00_32)(i386_state *cpustate)			// Opcode 0x0f 00
{
	uint32_t address, ea;
	uint8_t modrm = FETCH(cpustate);
	I386_SREG seg;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* SLDT */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					STORE_RM32(address, cpustate->ldtr.segment);
					CYCLES(cpustate,CYCLES_SLDT_REG);
				} else {
					ea = GetEA(cpustate,modrm);
					WRITE32(cpustate, ea, cpustate->ldtr.segment);
					CYCLES(cpustate,CYCLES_SLDT_MEM);
				}
			}
			else
			{
				i386_trap(cpustate,6, 0);
			}
			break;
		case 1: 		/* STR */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					STORE_RM32(address, cpustate->task.segment);
					CYCLES(cpustate,CYCLES_STR_REG);
				} else {
					ea = GetEA(cpustate,modrm);
					WRITE32(cpustate, ea, cpustate->task.segment);
					CYCLES(cpustate,CYCLES_STR_MEM);
				}
			}
			else
			{
				i386_trap(cpustate,6, 0);
			}
			break;
		case 2:			/* LLDT */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate(cpustate, CS, address );
					CYCLES(cpustate,CYCLES_LLDT_REG);
				} else {
					ea = GetEA(cpustate,modrm);
					CYCLES(cpustate,CYCLES_LLDT_MEM);
				}
				cpustate->ldtr.segment = READ32(cpustate,ea);
				memset(&seg, 0, sizeof(seg));
				seg.selector = cpustate->ldtr.segment;
				i386_load_protected_mode_segment(cpustate,&seg);
				cpustate->ldtr.limit = seg.limit;
				cpustate->ldtr.base = seg.base;
			}
			else
			{
				i386_trap(cpustate,6, 0);
			}
			break;

		case 3:			/* LTR */
			if ( PROTECTED_MODE && !V8086_MODE )
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate(cpustate, CS, address );
					CYCLES(cpustate,CYCLES_LTR_REG);
				} else {
					ea = GetEA(cpustate,modrm);
					CYCLES(cpustate,CYCLES_LTR_MEM);
				}
				cpustate->task.segment = READ32(cpustate,ea);
			}
			else
			{
				i386_trap(cpustate,6, 0);
			}
			break;

		default:
			fatalerror("i386: group0F00_32 /%d unimplemented", (modrm >> 3) & 0x7);
			break;
	}
}

static void I386OP(group0F01_32)(i386_state *cpustate)		// Opcode 0x0f 01
{
	uint8_t modrm = FETCH(cpustate);
	uint32_t address, ea;

	switch( (modrm >> 3) & 0x7 )
	{
		case 0:			/* SGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate(cpustate, CS, address );
				} else {
					ea = GetEA(cpustate,modrm);
				}
				WRITE16(cpustate,ea, cpustate->gdtr.limit);
				WRITE32(cpustate,ea + 2, cpustate->gdtr.base);
				CYCLES(cpustate,CYCLES_SGDT);
				break;
			}
		case 1:			/* SIDT */
			{
				if (modrm >= 0xc0)
				{
					address = LOAD_RM32(modrm);
					ea = i386_translate(cpustate, CS, address );
				}
				else
				{
					ea = GetEA(cpustate,modrm);
				}
				WRITE16(cpustate,ea, cpustate->idtr.limit);
				WRITE32(cpustate,ea + 2, cpustate->idtr.base);
				CYCLES(cpustate,CYCLES_SIDT);
				break;
			}
		case 2:			/* LGDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate(cpustate, CS, address );
				} else {
					ea = GetEA(cpustate,modrm);
				}
				cpustate->gdtr.limit = READ16(cpustate,ea);
				cpustate->gdtr.base = READ32(cpustate,ea + 2);
				CYCLES(cpustate,CYCLES_LGDT);
				break;
			}
		case 3:			/* LIDT */
			{
				if( modrm >= 0xc0 ) {
					address = LOAD_RM32(modrm);
					ea = i386_translate(cpustate, CS, address );
				} else {
					ea = GetEA(cpustate,modrm);
				}
				cpustate->idtr.limit = READ16(cpustate,ea);
				cpustate->idtr.base = READ32(cpustate,ea + 2);
				CYCLES(cpustate,CYCLES_LIDT);
				break;
			}
		default:
			fatalerror("i386: unimplemented opcode 0x0f 01 /%d at %08X", (modrm >> 3) & 0x7, cpustate->eip - 2);
			break;
	}
}

static void I386OP(group0FBA_32)(i386_state *cpustate)		// Opcode 0x0f ba
{
	uint8_t modrm = FETCH(cpustate);

	switch( (modrm >> 3) & 0x7 )
	{
		case 4:			/* BT Rm32, i8 */
			if( modrm >= 0xc0 ) {
				uint32_t dst = LOAD_RM32(modrm);
				uint8_t bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;

				CYCLES(cpustate,CYCLES_BT_IMM_REG);
			} else {
				uint32_t ea = GetEA(cpustate,modrm);
				uint32_t dst = READ32(cpustate,ea);
				uint8_t bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;

				CYCLES(cpustate,CYCLES_BT_IMM_MEM);
			}
			break;
		case 5:			/* BTS Rm32, i8 */
			if( modrm >= 0xc0 ) {
				uint32_t dst = LOAD_RM32(modrm);
				uint8_t bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst |= (1 << bit);

				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_BTS_IMM_REG);
			} else {
				uint32_t ea = GetEA(cpustate,modrm);
				uint32_t dst = READ32(cpustate,ea);
				uint8_t bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst |= (1 << bit);

				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_BTS_IMM_MEM);
			}
			break;
		case 6:			/* BTR Rm32, i8 */
			if( modrm >= 0xc0 ) {
				uint32_t dst = LOAD_RM32(modrm);
				uint8_t bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst &= ~(1 << bit);

				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_BTR_IMM_REG);
			} else {
				uint32_t ea = GetEA(cpustate,modrm);
				uint32_t dst = READ32(cpustate,ea);
				uint8_t bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst &= ~(1 << bit);

				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_BTR_IMM_MEM);
			}
			break;
		case 7:			/* BTC Rm32, i8 */
			if( modrm >= 0xc0 ) {
				uint32_t dst = LOAD_RM32(modrm);
				uint8_t bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst ^= (1 << bit);

				STORE_RM32(modrm, dst);
				CYCLES(cpustate,CYCLES_BTC_IMM_REG);
			} else {
				uint32_t ea = GetEA(cpustate,modrm);
				uint32_t dst = READ32(cpustate,ea);
				uint8_t bit = FETCH(cpustate);

				if( dst & (1 << bit) )
					cpustate->CF = 1;
				else
					cpustate->CF = 0;
				dst ^= (1 << bit);

				WRITE32(cpustate,ea, dst);
				CYCLES(cpustate,CYCLES_BTC_IMM_MEM);
			}
			break;
		default:
			fatalerror("i386: group0FBA_32 /%d unknown", (modrm >> 3) & 0x7);
			break;
	}
}

static void I386OP(lsl_r32_rm32)(i386_state *cpustate)  // Opcode 0x0f 0x03
{
	uint8_t modrm = FETCH(cpustate);
	uint32_t limit;
	I386_SREG seg;

	if(PROTECTED_MODE)
	{
		memset(&seg, 0, sizeof(seg));
		seg.selector = LOAD_RM32(modrm);
		if(seg.selector == 0)
		{
			SetZF(0);  // not a valid segment
		}
		else
		{
			// TODO: check segment type
			i386_load_protected_mode_segment(cpustate,&seg);
			limit = seg.limit;
			STORE_REG32(modrm,limit);
			SetZF(1);
		}
	}
	else
		i386_trap(cpustate,6, 0);
}

static void I386OP(bound_r32_m32_m32)(i386_state *cpustate)	// Opcode 0x62
{
	uint8_t modrm;
	int32_t val, low, high;

	modrm = FETCH(cpustate);

	if (modrm >= 0xc0)
	{
		low = high = LOAD_RM32(modrm);
	}
	else
	{
		uint32_t ea = GetEA(cpustate,modrm);
		low = READ32(cpustate,ea + 0);
		high = READ32(cpustate,ea + 4);
	}
	val = LOAD_REG32(modrm);

	if ((val < low) || (val > high))
	{
		CYCLES(cpustate,CYCLES_BOUND_OUT_RANGE);
		i386_trap(cpustate,5, 0);
	}
	else
	{
		CYCLES(cpustate,CYCLES_BOUND_IN_RANGE);
	}
}

static void I386OP(retf32)(i386_state *cpustate)			// Opcode 0xcb
{
	cpustate->eip = POP32(cpustate);
	cpustate->sreg[CS].selector = POP32(cpustate);
	i386_load_segment_descriptor(cpustate, CS );
	CHANGE_PC(cpustate,cpustate->eip);

	CYCLES(cpustate,CYCLES_RET_INTERSEG);
}

static void I386OP(retf_i32)(i386_state *cpustate)			// Opcode 0xca
{
	uint16_t count = FETCH16(cpustate);

	cpustate->eip = POP32(cpustate);
	cpustate->sreg[CS].selector = POP32(cpustate);
	i386_load_segment_descriptor(cpustate, CS );
	CHANGE_PC(cpustate,cpustate->eip);

	REG32(ESP) += count;
	CYCLES(cpustate,CYCLES_RET_IMM_INTERSEG);
}

static void I386OP(xlat32)(i386_state *cpustate)			// Opcode 0xd7
{
	uint32_t ea;
	if( cpustate->segment_prefix ) {
		ea = i386_translate(cpustate, cpustate->segment_override, REG32(EBX) + REG8(AL) );
	} else {
		ea = i386_translate(cpustate, DS, REG32(EBX) + REG8(AL) );
	}
	REG8(AL) = READ8(cpustate,ea);
	CYCLES(cpustate,CYCLES_XLAT);
}

static void I386OP(load_far_pointer32)(i386_state *cpustate, int s)
{
	uint8_t modrm = FETCH(cpustate);

	if( modrm >= 0xc0 ) {
		fatalerror("i386: load_far_pointer32 NYI");
	} else {
		uint32_t ea = GetEA(cpustate,modrm);
		STORE_REG32(modrm, READ32(cpustate,ea + 0));
		cpustate->sreg[s].selector = READ16(cpustate,ea + 4);
		i386_load_segment_descriptor(cpustate, s );
	}
}

static void I386OP(lds32)(i386_state *cpustate)				// Opcode 0xc5
{
	I386OP(load_far_pointer32)(cpustate, DS);
	CYCLES(cpustate,CYCLES_LDS);
}

static void I386OP(lss32)(i386_state *cpustate)				// Opcode 0x0f 0xb2
{
	I386OP(load_far_pointer32)(cpustate, SS);
	CYCLES(cpustate,CYCLES_LSS);
}

static void I386OP(les32)(i386_state *cpustate)				// Opcode 0xc4
{
	I386OP(load_far_pointer32)(cpustate, ES);
	CYCLES(cpustate,CYCLES_LES);
}

static void I386OP(lfs32)(i386_state *cpustate)				// Opcode 0x0f 0xb4
{
	I386OP(load_far_pointer32)(cpustate, FS);
	CYCLES(cpustate,CYCLES_LFS);
}

static void I386OP(lgs32)(i386_state *cpustate)				// Opcode 0x0f 0xb5
{
	I386OP(load_far_pointer32)(cpustate, GS);
	CYCLES(cpustate,CYCLES_LGS);
}
