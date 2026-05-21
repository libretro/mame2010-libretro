/* PowerPC common opcodes */

// it really seems like this should be elsewhere - like maybe the floating point checks can hang out someplace else
#include <math.h>

#ifndef PPC_DRC
static void ppc_unimplemented(uint32_t op)
{
	fatalerror("ppc: Unimplemented opcode %08X at %08X", op, ppc.pc);
}

static void ppc_addx(uint32_t op)
{
	uint32_t ra = REG(RA);
	uint32_t rb = REG(RB);

	REG(RT) = ra + rb;

	if( OEBIT ) {
		SET_ADD_OV(REG(RT), ra, rb);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_addcx(uint32_t op)
{
	uint32_t ra = REG(RA);
	uint32_t rb = REG(RB);

	REG(RT) = ra + rb;

	SET_ADD_CA(REG(RT), ra, rb);

	if( OEBIT ) {
		SET_ADD_OV(REG(RT), ra, rb);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}
#endif

static void ppc_addex(uint32_t op)
{
	uint32_t ra = REG(RA);
	uint32_t rb = REG(RB);
	uint32_t carry = (XER >> 29) & 0x1;
	uint32_t tmp;

	tmp = rb + carry;
	REG(RT) = ra + tmp;

	if( ADD_CA(tmp, rb, carry) || ADD_CA(REG(RT), ra, tmp) )
		XER |= XER_CA;
	else
		XER &= ~XER_CA;

	if( OEBIT ) {
		SET_ADD_OV(REG(RT), ra, rb);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

#ifndef PPC_DRC
static void ppc_addi(uint32_t op)
{
	uint32_t i = SIMM16;
	uint32_t a = RA;

	if( a )
		i += REG(a);

	REG(RT) = i;
}

static void ppc_addic(uint32_t op)
{
	uint32_t i = SIMM16;
	uint32_t ra = REG(RA);

	REG(RT) = ra + i;

	if( ADD_CA(REG(RT), ra, i) )
		XER |= XER_CA;
	else
		XER &= ~XER_CA;
}

static void ppc_addic_rc(uint32_t op)
{
	uint32_t i = SIMM16;
	uint32_t ra = REG(RA);

	REG(RT) = ra + i;

	if( ADD_CA(REG(RT), ra, i) )
		XER |= XER_CA;
	else
		XER &= ~XER_CA;

	SET_CR0(REG(RT));
}

static void ppc_addis(uint32_t op)
{
	uint32_t i = UIMM16 << 16;
	uint32_t a = RA;

	if( a )
		i += REG(a);

	REG(RT) = i;
}
#endif

static void ppc_addmex(uint32_t op)
{
	uint32_t ra = REG(RA);
	uint32_t carry = (XER >> 29) & 0x1;
	uint32_t tmp;

	tmp = ra + carry;
	REG(RT) = tmp + -1;

	if( ADD_CA(tmp, ra, carry) || ADD_CA(REG(RT), tmp, -1) )
		XER |= XER_CA;
	else
		XER &= ~XER_CA;

	if( OEBIT ) {
		SET_ADD_OV(REG(RT), ra, carry - 1);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_addzex(uint32_t op)
{
	uint32_t ra = REG(RA);
	uint32_t carry = (XER >> 29) & 0x1;

	REG(RT) = ra + carry;

	if( ADD_CA(REG(RT), ra, carry) )
		XER |= XER_CA;
	else
		XER &= ~XER_CA;

	if( OEBIT ) {
		SET_ADD_OV(REG(RT), ra, carry);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

#ifndef PPC_DRC
static void ppc_andx(uint32_t op)
{
	REG(RA) = REG(RS) & REG(RB);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_andcx(uint32_t op)
{
	REG(RA) = REG(RS) & ~REG(RB);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_andi_rc(uint32_t op)
{
	uint32_t i = UIMM16;

	REG(RA) = REG(RS) & i;

	SET_CR0(REG(RA));
}

static void ppc_andis_rc(uint32_t op)
{
	uint32_t i = UIMM16 << 16;

	REG(RA) = REG(RS) & i;

	SET_CR0(REG(RA));
}

static void ppc_bx(uint32_t op)
{
	int32_t li = op & 0x3fffffc;
	if( li & 0x2000000 )
		li |= 0xfc000000;

	if( AABIT ) {
		ppc.npc = li;
	} else {
		ppc.npc = ppc.pc + li;
	}

	if( LKBIT ) {
		LR = ppc.pc + 4;
	}
}

static void ppc_bcx(uint32_t op)
{
	int condition = check_condition_code(BO, BI);

	if( condition ) {
		if( AABIT ) {
			ppc.npc = SIMM16 & ~0x3;
		} else {
			ppc.npc = ppc.pc + (SIMM16 & ~0x3);
		}
	}

	if( LKBIT ) {
		LR = ppc.pc + 4;
	}
}

static void ppc_bcctrx(uint32_t op)
{
	int condition = check_condition_code(BO, BI);

	if( condition ) {
		ppc.npc = CTR & ~0x3;
	}

	if( LKBIT ) {
		LR = ppc.pc + 4;
	}
}

static void ppc_bclrx(uint32_t op)
{
	int condition = check_condition_code(BO, BI);

	if( condition ) {
		ppc.npc = LR & ~0x3;
	}

	if( LKBIT ) {
		LR = ppc.pc + 4;
	}
}

static void ppc_cmp(uint32_t op)
{
	int32_t ra = REG(RA);
	int32_t rb = REG(RB);
	int d = CRFD;

	if( ra < rb )
		CR(d) = 0x8;
	else if( ra > rb )
		CR(d) = 0x4;
	else
		CR(d) = 0x2;

	if( XER & XER_SO )
		CR(d) |= 0x1;
}

static void ppc_cmpi(uint32_t op)
{
	int32_t ra = REG(RA);
	int32_t i = SIMM16;
	int d = CRFD;

	if( ra < i )
		CR(d) = 0x8;
	else if( ra > i )
		CR(d) = 0x4;
	else
		CR(d) = 0x2;

	if( XER & XER_SO )
		CR(d) |= 0x1;
}

static void ppc_cmpl(uint32_t op)
{
	uint32_t ra = REG(RA);
	uint32_t rb = REG(RB);
	int d = CRFD;

	if( ra < rb )
		CR(d) = 0x8;
	else if( ra > rb )
		CR(d) = 0x4;
	else
		CR(d) = 0x2;

	if( XER & XER_SO )
		CR(d) |= 0x1;
}

static void ppc_cmpli(uint32_t op)
{
	uint32_t ra = REG(RA);
	uint32_t i = UIMM16;
	int d = CRFD;

	if( ra < i )
		CR(d) = 0x8;
	else if( ra > i )
		CR(d) = 0x4;
	else
		CR(d) = 0x2;

	if( XER & XER_SO )
		CR(d) |= 0x1;
}

static void ppc_cntlzw(uint32_t op)
{
	int n = 0;
	int t = RT;
	uint32_t m = 0x80000000;

	while(n < 32)
	{
		if( REG(t) & m )
			break;
		m >>= 1;
		n++;
	}

	REG(RA) = n;

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}
#endif

static void ppc_crand(uint32_t op)
{
	int bit = RT;
	int b = CRBIT(RA) & CRBIT(RB);
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_crandc(uint32_t op)
{
	int bit = RT;
	int b = CRBIT(RA) & ~CRBIT(RB);
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_creqv(uint32_t op)
{
	int bit = RT;
	int b = ~(CRBIT(RA) ^ CRBIT(RB));
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_crnand(uint32_t op)
{
	int bit = RT;
	int b = ~(CRBIT(RA) & CRBIT(RB));
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_crnor(uint32_t op)
{
	int bit = RT;
	int b = ~(CRBIT(RA) | CRBIT(RB));
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_cror(uint32_t op)
{
	int bit = RT;
	int b = CRBIT(RA) | CRBIT(RB);
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_crorc(uint32_t op)
{
	int bit = RT;
	int b = CRBIT(RA) | ~CRBIT(RB);
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_crxor(uint32_t op)
{
	int bit = RT;
	int b = CRBIT(RA) ^ CRBIT(RB);
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

#ifndef PPC_DRC
static void ppc_dcbf(uint32_t op)
{

}

static void ppc_dcbi(uint32_t op)
{

}

static void ppc_dcbst(uint32_t op)
{

}

static void ppc_dcbt(uint32_t op)
{

}

static void ppc_dcbtst(uint32_t op)
{

}

static void ppc_dcbz(uint32_t op)
{

}
#endif

static void ppc_divwx(uint32_t op)
{
	if( REG(RB) == 0 && REG(RA) < 0x80000000 )
	{
		REG(RT) = 0;
		if( OEBIT ) {
			XER |= XER_SO | XER_OV;
		}
	}
	else if( REG(RB) == 0 || (REG(RB) == 0xffffffff && REG(RA) == 0x80000000) )
	{
		REG(RT) = 0xffffffff;
		if( OEBIT ) {
			XER |= XER_SO | XER_OV;
		}
	}
	else
	{
		REG(RT) = (int32_t)REG(RA) / (int32_t)REG(RB);
		if( OEBIT ) {
			XER &= ~XER_OV;
		}
	}

	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_divwux(uint32_t op)
{
	if( REG(RB) == 0 )
	{
		REG(RT) = 0;
		if( OEBIT ) {
			XER |= XER_SO | XER_OV;
		}
	}
	else
	{
		REG(RT) = (uint32_t)REG(RA) / (uint32_t)REG(RB);
		if( OEBIT ) {
			XER &= ~XER_OV;
		}
	}

	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

#ifndef PPC_DRC
static void ppc_eieio(uint32_t op)
{

}

static void ppc_eqvx(uint32_t op)
{
	REG(RA) = ~(REG(RS) ^ REG(RB));

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_extsbx(uint32_t op)
{
	REG(RA) = (int32_t)(int8_t)REG(RS);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_extshx(uint32_t op)
{
	REG(RA) = (int32_t)(int16_t)REG(RS);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_icbi(uint32_t op)
{

}

static void ppc_isync(uint32_t op)
{

}

static void ppc_lbz(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	REG(RT) = (uint32_t)READ8(ea);
}

static void ppc_lbzu(uint32_t op)
{
	uint32_t ea = REG(RA) + SIMM16;

	REG(RT) = (uint32_t)READ8(ea);
	REG(RA) = ea;
}

static void ppc_lbzux(uint32_t op)
{
	uint32_t ea = REG(RA) + REG(RB);

	REG(RT) = (uint32_t)READ8(ea);
	REG(RA) = ea;
}

static void ppc_lbzx(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	REG(RT) = (uint32_t)READ8(ea);
}

static void ppc_lha(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	REG(RT) = (int32_t)(int16_t)READ16(ea);
}

static void ppc_lhau(uint32_t op)
{
	uint32_t ea = REG(RA) + SIMM16;

	REG(RT) = (int32_t)(int16_t)READ16(ea);
	REG(RA) = ea;
}

static void ppc_lhaux(uint32_t op)
{
	uint32_t ea = REG(RA) + REG(RB);

	REG(RT) = (int32_t)(int16_t)READ16(ea);
	REG(RA) = ea;
}

static void ppc_lhax(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	REG(RT) = (int32_t)(int16_t)READ16(ea);
}

static void ppc_lhbrx(uint32_t op)
{
	uint32_t ea;
	uint16_t w;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	w = READ16(ea);
	REG(RT) = (uint32_t)BYTE_REVERSE16(w);
}

static void ppc_lhz(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	REG(RT) = (uint32_t)READ16(ea);
}

static void ppc_lhzu(uint32_t op)
{
	uint32_t ea = REG(RA) + SIMM16;

	REG(RT) = (uint32_t)READ16(ea);
	REG(RA) = ea;
}

static void ppc_lhzux(uint32_t op)
{
	uint32_t ea = REG(RA) + REG(RB);

	REG(RT) = (uint32_t)READ16(ea);
	REG(RA) = ea;
}

static void ppc_lhzx(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	REG(RT) = (uint32_t)READ16(ea);
}
#endif

static void ppc_lmw(uint32_t op)
{
	int r = RT;
	uint32_t ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	while( r <= 31 )
	{
		REG(r) = READ32(ea);
		ea += 4;
		r++;
	}
}

static void ppc_lswi(uint32_t op)
{
	int n, r, i;
	uint32_t ea = 0;
	if( RA != 0 )
		ea = REG(RA);

	if( RB == 0 )
		n = 32;
	else
		n = RB;

	r = RT - 1;
	i = 0;

	while(n > 0)
	{
		if (i == 0) {
			r = (r + 1) % 32;
			REG(r) = 0;
		}
		REG(r) |= ((READ8(ea) & 0xff) << (24 - i));
		i += 8;
		if (i == 32) {
			i = 0;
		}
		ea++;
		n--;
	}
}

static void ppc_lswx(uint32_t op)
{
	int n, r, i;
	uint32_t ea = 0;
	if( RA != 0 )
		ea = REG(RA);

	ea += REG(RB);

	n = ppc.xer & 0x7f;

	r = RT - 1;
	i = 0;

	while(n > 0)
	{
		if (i == 0) {
			r = (r + 1) % 32;
			REG(r) = 0;
		}
		REG(r) |= ((READ8(ea) & 0xff) << (24 - i));
		i += 8;
		if (i == 32) {
			i = 0;
		}
		ea++;
		n--;
	}
}

static void ppc_lwarx(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	ppc.reserved_address = ea;
	ppc.reserved = 1;

	REG(RT) = READ32(ea);
}

#ifndef PPC_DRC
static void ppc_lwbrx(uint32_t op)
{
	uint32_t ea;
	uint32_t w;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	w = READ32(ea);
	REG(RT) = BYTE_REVERSE32(w);
}

static void ppc_lwz(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	REG(RT) = READ32(ea);
}

static void ppc_lwzu(uint32_t op)
{
	uint32_t ea = REG(RA) + SIMM16;

	REG(RT) = READ32(ea);
	REG(RA) = ea;
}

static void ppc_lwzux(uint32_t op)
{
	uint32_t ea = REG(RA) + REG(RB);

	REG(RT) = READ32(ea);
	REG(RA) = ea;
}

static void ppc_lwzx(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	REG(RT) = READ32(ea);
}

static void ppc_mcrf(uint32_t op)
{
	CR(RT >> 2) = CR(RA >> 2);
}

static void ppc_mcrxr(uint32_t op)
{
	CR(RT >> 2) = (XER >> 28) & 0x0F;
	XER &= ~0xf0000000;
}

static void ppc_mfcr(uint32_t op)
{
	REG(RT) = ppc_get_cr();
}

static void ppc_mfmsr(uint32_t op)
{
	REG(RT) = ppc_get_msr();
}

static void ppc_mfspr(uint32_t op)
{
	REG(RT) = ppc_get_spr(SPR);
}
#endif

static void ppc_mtcrf(uint32_t op)
{
	int fxm = FXM;
	int t = RT;

	if( fxm & 0x80 )	CR(0) = (REG(t) >> 28) & 0xf;
	if( fxm & 0x40 )	CR(1) = (REG(t) >> 24) & 0xf;
	if( fxm & 0x20 )	CR(2) = (REG(t) >> 20) & 0xf;
	if( fxm & 0x10 )	CR(3) = (REG(t) >> 16) & 0xf;
	if( fxm & 0x08 )	CR(4) = (REG(t) >> 12) & 0xf;
	if( fxm & 0x04 )	CR(5) = (REG(t) >> 8) & 0xf;
	if( fxm & 0x02 )	CR(6) = (REG(t) >> 4) & 0xf;
	if( fxm & 0x01 )	CR(7) = (REG(t) >> 0) & 0xf;
}

#ifndef PPC_DRC
static void ppc_mtmsr(uint32_t op)
{
	ppc_set_msr(REG(RS));
}

static void ppc_mtspr(uint32_t op)
{
	ppc_set_spr(SPR, REG(RS));
}

static void ppc_mulhwx(uint32_t op)
{
	int64_t ra = (int64_t)(int32_t)REG(RA);
	int64_t rb = (int64_t)(int32_t)REG(RB);

	REG(RT) = (uint32_t)((ra * rb) >> 32);

	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_mulhwux(uint32_t op)
{
	uint64_t ra = (uint64_t)REG(RA);
	uint64_t rb = (uint64_t)REG(RB);

	REG(RT) = (uint32_t)((ra * rb) >> 32);

	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_mulli(uint32_t op)
{
	int32_t ra = (int32_t)REG(RA);
	int32_t i = SIMM16;

	REG(RT) = ra * i;
}

static void ppc_mullwx(uint32_t op)
{
	int64_t ra = (int64_t)(int32_t)REG(RA);
	int64_t rb = (int64_t)(int32_t)REG(RB);
	int64_t r;

	r = ra * rb;
	REG(RT) = (uint32_t)r;

	if( OEBIT ) {
		XER &= ~XER_OV;

		if( r != (int64_t)(int32_t)r )
			XER |= XER_OV | XER_SO;
	}

	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_nandx(uint32_t op)
{
	REG(RA) = ~(REG(RS) & REG(RB));

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_negx(uint32_t op)
{
	REG(RT) = -REG(RA);

	if( OEBIT ) {
		if( REG(RT) == 0x80000000 )
			XER |= XER_OV | XER_SO;
		else
			XER &= ~XER_OV;
	}

	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_norx(uint32_t op)
{
	REG(RA) = ~(REG(RS) | REG(RB));

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_orx(uint32_t op)
{
	REG(RA) = REG(RS) | REG(RB);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_orcx(uint32_t op)
{
	REG(RA) = REG(RS) | ~REG(RB);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_ori(uint32_t op)
{
	REG(RA) = REG(RS) | UIMM16;
}

static void ppc_oris(uint32_t op)
{
	REG(RA) = REG(RS) | (UIMM16 << 16);
}

static void ppc_rfi(uint32_t op)
{
	uint32_t msr;
	ppc.npc = ppc_get_spr(SPR_SRR0);
	msr = ppc_get_spr(SPR_SRR1);
	ppc_set_msr( msr );
}

static void ppc_rlwimix(uint32_t op)
{
	uint32_t r;
	uint32_t mask = GET_ROTATE_MASK(MB, ME);
	uint32_t rs = REG(RS);
	int sh = SH;

	r = (rs << sh) | (rs >> (32-sh));
	REG(RA) = (REG(RA) & ~mask) | (r & mask);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_rlwinmx(uint32_t op)
{
	uint32_t r;
	uint32_t mask = GET_ROTATE_MASK(MB, ME);
	uint32_t rs = REG(RS);
	int sh = SH;

	r = (rs << sh) | (rs >> (32-sh));
	REG(RA) = r & mask;

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_rlwnmx(uint32_t op)
{
	uint32_t r;
	uint32_t mask = GET_ROTATE_MASK(MB, ME);
	uint32_t rs = REG(RS);
	int sh = REG(RB) & 0x1f;

	r = (rs << sh) | (rs >> (32-sh));
	REG(RA) = r & mask;

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}
#endif

#ifndef PPC_DRC
static void ppc_sc(uint32_t op)
{
	if (ppc.is603) {
		ppc603_exception(EXCEPTION_SYSTEM_CALL);
	}
	if (ppc.is602) {
		ppc602_exception(EXCEPTION_SYSTEM_CALL);
	}
	if (IS_PPC403()) {
		ppc403_exception(EXCEPTION_SYSTEM_CALL);
	}
}
#endif

static void ppc_slwx(uint32_t op)
{
	int sh = REG(RB) & 0x3f;

	if( sh > 31 ) {
		REG(RA) = 0;
	}
	else {
		REG(RA) = REG(RS) << sh;
	}

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_srawx(uint32_t op)
{
	int sh = REG(RB) & 0x3f;

	XER &= ~XER_CA;

	if( sh > 31 ) {
		if (REG(RS) & 0x80000000)
			REG(RA) = 0xffffffff;
		else
			REG(RA) = 0;
		if( REG(RA) )
			XER |= XER_CA;
	}
	else {
		REG(RA) = (int32_t)(REG(RS)) >> sh;
		if( ((int32_t)(REG(RS)) < 0) && (REG(RS) & BITMASK_0(sh)) )
			XER |= XER_CA;
	}

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_srawix(uint32_t op)
{
	int sh = SH;

	XER &= ~XER_CA;
	if( ((int32_t)(REG(RS)) < 0) && (REG(RS) & BITMASK_0(sh)) )
		XER |= XER_CA;

	REG(RA) = (int32_t)(REG(RS)) >> sh;

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_srwx(uint32_t op)
{
	int sh = REG(RB) & 0x3f;

	if( sh > 31 ) {
		REG(RA) = 0;
	}
	else {
		REG(RA) = REG(RS) >> sh;
	}

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

#ifndef PPC_DRC
static void ppc_stb(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	WRITE8(ea, (uint8_t)REG(RS));
}

static void ppc_stbu(uint32_t op)
{
	uint32_t ea = REG(RA) + SIMM16;

	WRITE8(ea, (uint8_t)REG(RS));
	REG(RA) = ea;
}

static void ppc_stbux(uint32_t op)
{
	uint32_t ea = REG(RA) + REG(RB);

	WRITE8(ea, (uint8_t)REG(RS));
	REG(RA) = ea;
}

static void ppc_stbx(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	WRITE8(ea, (uint8_t)REG(RS));
}

static void ppc_sth(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	WRITE16(ea, (uint16_t)REG(RS));
}

static void ppc_sthbrx(uint32_t op)
{
	uint32_t ea;
	uint16_t w;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	w = REG(RS);
	WRITE16(ea, (uint16_t)BYTE_REVERSE16(w));
}

static void ppc_sthu(uint32_t op)
{
	uint32_t ea = REG(RA) + SIMM16;

	WRITE16(ea, (uint16_t)REG(RS));
	REG(RA) = ea;
}

static void ppc_sthux(uint32_t op)
{
	uint32_t ea = REG(RA) + REG(RB);

	WRITE16(ea, (uint16_t)REG(RS));
	REG(RA) = ea;
}

static void ppc_sthx(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	WRITE16(ea, (uint16_t)REG(RS));
}
#endif

static void ppc_stmw(uint32_t op)
{
	uint32_t ea;
	int r = RS;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	while( r <= 31 )
	{
		WRITE32(ea, REG(r));
		ea += 4;
		r++;
	}
}

static void ppc_stswi(uint32_t op)
{
	int n, r, i;
	uint32_t ea = 0;
	if( RA != 0 )
		ea = REG(RA);

	if( RB == 0 )
		n = 32;
	else
		n = RB;

	r = RT - 1;
	i = 0;

	while(n > 0)
	{
		if (i == 0) {
			r = (r + 1) % 32;
		}
		WRITE8(ea, (REG(r) >> (24-i)) & 0xff);
		i += 8;
		if (i == 32) {
			i = 0;
		}
		ea++;
		n--;
	}
}

static void ppc_stswx(uint32_t op)
{
	int n, r, i;
	uint32_t ea = 0;
	if( RA != 0 )
		ea = REG(RA);

	ea += REG(RB);

	n = ppc.xer & 0x7f;

	r = RT - 1;
	i = 0;

	while(n > 0)
	{
		if (i == 0) {
			r = (r + 1) % 32;
		}
		WRITE8(ea, (REG(r) >> (24-i)) & 0xff);
		i += 8;
		if (i == 32) {
			i = 0;
		}
		ea++;
		n--;
	}
}

#ifndef PPC_DRC
static void ppc_stw(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	WRITE32(ea, REG(RS));
}

static void ppc_stwbrx(uint32_t op)
{
	uint32_t ea;
	uint32_t w;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	w = REG(RS);
	WRITE32(ea, BYTE_REVERSE32(w));
}
#endif

static void ppc_stwcx_rc(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	if( ppc.reserved ) {
		WRITE32(ea, REG(RS));

		ppc.reserved = 0;
		ppc.reserved_address = 0;

		CR(0) = 0x2;
		if( XER & XER_SO )
			CR(0) |= 0x1;
	} else {
		CR(0) = 0;
		if( XER & XER_SO )
			CR(0) |= 0x1;
	}
}

#ifndef PPC_DRC
static void ppc_stwu(uint32_t op)
{
	uint32_t ea = REG(RA) + SIMM16;

	WRITE32(ea, REG(RS));
	REG(RA) = ea;
}

static void ppc_stwux(uint32_t op)
{
	uint32_t ea = REG(RA) + REG(RB);

	WRITE32(ea, REG(RS));
	REG(RA) = ea;
}

static void ppc_stwx(uint32_t op)
{
	uint32_t ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	WRITE32(ea, REG(RS));
}

static void ppc_subfx(uint32_t op)
{
	uint32_t ra = REG(RA);
	uint32_t rb = REG(RB);
	REG(RT) = rb - ra;

	if( OEBIT ) {
		SET_SUB_OV(REG(RT), rb, ra);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}
#endif

static void ppc_subfcx(uint32_t op)
{
	uint32_t ra = REG(RA);
	uint32_t rb = REG(RB);
	REG(RT) = rb - ra;

	SET_SUB_CA(REG(RT), rb, ra);

	if( OEBIT ) {
		SET_SUB_OV(REG(RT), rb, ra);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

#ifndef PPC_DRC
static void ppc_subfex(uint32_t op)
{
	uint32_t ra = REG(RA);
	uint32_t rb = REG(RB);
	uint32_t carry = (XER >> 29) & 0x1;
	uint32_t r;

	r = ~ra + carry;
	REG(RT) = rb + r;

	SET_ADD_CA(r, ~ra, carry);		/* step 1 carry */
	if( REG(RT) < r )				/* step 2 carry */
		XER |= XER_CA;

	if( OEBIT ) {
		SET_SUB_OV(REG(RT), rb, ra);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_subfic(uint32_t op)
{
	uint32_t i = SIMM16;
	uint32_t ra = REG(RA);

	REG(RT) = i - ra;

	SET_SUB_CA(REG(RT), i, ra);
}
#endif

static void ppc_subfmex(uint32_t op)
{
	uint32_t ra = REG(RA);
	uint32_t carry = (XER >> 29) & 0x1;
	uint32_t r;

	r = ~ra + carry;
	REG(RT) = r - 1;

	SET_SUB_CA(r, ~ra, carry);		/* step 1 carry */
	if( REG(RT) < r )
		XER |= XER_CA;				/* step 2 carry */

	if( OEBIT ) {
		SET_SUB_OV(REG(RT), -1, ra);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_subfzex(uint32_t op)
{
	uint32_t ra = REG(RA);
	uint32_t carry = (XER >> 29) & 0x1;

	REG(RT) = ~ra + carry;

	SET_ADD_CA(REG(RT), ~ra, carry);

	if( OEBIT ) {
		SET_SUB_OV(REG(RT), 0, REG(RA));
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

#ifndef PPC_DRC
static void ppc_sync(uint32_t op)
{

}
#endif

#ifndef PPC_DRC
static void ppc_tw(uint32_t op)
{
	int exception = 0;
	int32_t a = REG(RA);
	int32_t b = REG(RB);
	int to = RT;

	if( (a < b) && (to & 0x10) ) {
		exception = 1;
	}
	if( (a > b) && (to & 0x08) ) {
		exception = 1;
	}
	if( (a == b) && (to & 0x04) ) {
		exception = 1;
	}
	if( ((uint32_t)a < (uint32_t)b) && (to & 0x02) ) {
		exception = 1;
	}
	if( ((uint32_t)a > (uint32_t)b) && (to & 0x01) ) {
		exception = 1;
	}

	if (exception) {
		if (ppc.is603) {
			ppc603_exception(EXCEPTION_TRAP);
		}
		if (ppc.is602) {
			ppc602_exception(EXCEPTION_TRAP);
		}
		if (IS_PPC403()) {
			ppc403_exception(EXCEPTION_TRAP);
		}
	}
}
#endif

#ifndef PPC_DRC
static void ppc_twi(uint32_t op)
{
	int exception = 0;
	int32_t a = REG(RA);
	int32_t i = SIMM16;
	int to = RT;

	if( (a < i) && (to & 0x10) ) {
		exception = 1;
	}
	if( (a > i) && (to & 0x08) ) {
		exception = 1;
	}
	if( (a == i) && (to & 0x04) ) {
		exception = 1;
	}
	if( ((uint32_t)a < (uint32_t)i) && (to & 0x02) ) {
		exception = 1;
	}
	if( ((uint32_t)a > (uint32_t)i) && (to & 0x01) ) {
		exception = 1;
	}

	if (exception) {
		if (ppc.is603) {
			ppc603_exception(EXCEPTION_TRAP);
		}
		if (ppc.is602) {
			ppc602_exception(EXCEPTION_TRAP);
		}
		if (IS_PPC403()) {
			ppc403_exception(EXCEPTION_TRAP);
		}
	}
}
#endif

#ifndef PPC_DRC
static void ppc_xorx(uint32_t op)
{
	REG(RA) = REG(RS) ^ REG(RB);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_xori(uint32_t op)
{
	REG(RA) = REG(RS) ^ UIMM16;
}

static void ppc_xoris(uint32_t op)
{
	REG(RA) = REG(RS) ^ (UIMM16 << 16);
}



static void ppc_invalid(uint32_t op)
{
	fatalerror("ppc: Invalid opcode %08X PC : %X", op, ppc.pc);
}
#endif


// Everything below is new from AJG

////////////////////////////
// !here are the 6xx ops! //
////////////////////////////

#define DOUBLE_SIGN		(U64(0x8000000000000000))
#define DOUBLE_EXP		(U64(0x7ff0000000000000))
#define DOUBLE_FRAC		(U64(0x000fffffffffffff))
#define DOUBLE_ZERO		(0)

/*
  Floating point operations.
*/

INLINE int is_nan_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == DOUBLE_EXP) &&
			((x.id & DOUBLE_FRAC) != DOUBLE_ZERO) );
}

INLINE int is_qnan_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == DOUBLE_EXP) &&
			((x.id & U64(0x0007fffffffffff)) == U64(0x000000000000000)) &&
			((x.id & U64(0x000800000000000)) == U64(0x000800000000000)) );
}

INLINE int is_snan_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == DOUBLE_EXP) &&
			((x.id & DOUBLE_FRAC) != DOUBLE_ZERO) &&
			((x.id & U64(0x0008000000000000)) == DOUBLE_ZERO) );
}

INLINE int is_infinity_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == DOUBLE_EXP) &&
			((x.id & DOUBLE_FRAC) == DOUBLE_ZERO) );
}

INLINE int is_normalized_double(FPR x)
{
	uint64_t exp;

	exp = (x.id & DOUBLE_EXP) >> 52;

	return (exp >= 1) && (exp <= 2046);
}

INLINE int is_denormalized_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == 0) &&
			((x.id & DOUBLE_FRAC) != DOUBLE_ZERO) );
}

INLINE int sign_double(FPR x)
{
	return ((x.id & DOUBLE_SIGN) != 0);
}

INLINE int64_t round_to_nearest(FPR f)
{
	if (f.fd >= 0)
	{
		return (int64_t)(f.fd + 0.5);
	}
	else
	{
		return -(int64_t)(-f.fd + 0.5);
	}
}

INLINE int64_t round_toward_zero(FPR f)
{
	return (int64_t)(f.fd);
}

INLINE int64_t round_toward_positive_infinity(FPR f)
{
	double r = ceil(f.fd);
	return (int64_t)(r);
}

INLINE int64_t round_toward_negative_infinity(FPR f)
{
	double r = floor(f.fd);
	return (int64_t)(r);
}


INLINE void set_fprf(FPR f)
{
	uint32_t fprf;

	// see page 3-30, 3-31

	if (is_qnan_double(f))
	{
		fprf = 0x11;
	}
	else if (is_infinity_double(f))
	{
		if (sign_double(f))		// -INF
			fprf = 0x09;
		else					// +INF
			fprf = 0x05;
	}
	else if (is_normalized_double(f))
	{
		if (sign_double(f))		// -Normalized
			fprf = 0x08;
		else					// +Normalized
			fprf = 0x04;
	}
	else if (is_denormalized_double(f))
	{
		if (sign_double(f))		// -Denormalized
			fprf = 0x18;
		else					// +Denormalized
			fprf = 0x14;
	}
	else    // Zero
	{
		if (sign_double(f))		// -Zero
			fprf = 0x12;
		else					// +Zero
			fprf = 0x02;
	}

	ppc.fpscr &= ~0x0001f000;
	ppc.fpscr |= (fprf << 12);
}



#define SET_VXSNAN(a, b)    if (is_snan_double(a) || is_snan_double(b)) ppc.fpscr |= 0x80000000
#define SET_VXSNAN_1(c)     if (is_snan_double(c)) ppc.fpscr |= 0x80000000




static void ppc_lfs(uint32_t op)
{
	uint32_t ea = SIMM16;
	uint32_t a = RA;
	uint32_t t = RT;
	FPR32 f;

	if(a)
		ea += REG(a);

	f.i = READ32(ea);
	FPR(t).fd = (double)(f.f);
}

static void ppc_lfsu(uint32_t op)
{
	uint32_t ea = SIMM16;
	uint32_t a = RA;
	uint32_t t = RT;
	FPR32 f;

	ea += REG(a);

	f.i = READ32(ea);
	FPR(t).fd = (double)(f.f);

	REG(a) = ea;
}

#ifndef PPC_DRC
static void ppc_lfd(uint32_t op)
{
	uint32_t ea = SIMM16;
	uint32_t a = RA;
	uint32_t t = RT;

	if(a)
		ea += REG(a);

	FPR(t).id = READ64(ea);
}

static void ppc_lfdu(uint32_t op)
{
	uint32_t ea = SIMM16;
	uint32_t a = RA;
	uint32_t d = RD;

	ea += REG(a);

	FPR(d).id = READ64(ea);

	REG(a) = ea;
}
#endif

static void ppc_stfs(uint32_t op)
{
	uint32_t ea = SIMM16;
	uint32_t a = RA;
	uint32_t t = RT;
	FPR32 f;

	if(a)
		ea += REG(a);

	f.f = (float)(FPR(t).fd);
	WRITE32(ea, f.i);
}

static void ppc_stfsu(uint32_t op)
{
	uint32_t ea = SIMM16;
	uint32_t a = RA;
	uint32_t t = RT;
	FPR32 f;

	ea += REG(a);

	f.f = (float)(FPR(t).fd);
	WRITE32(ea, f.i);

	REG(a) = ea;
}

#ifndef PPC_DRC
static void ppc_stfd(uint32_t op)
{
	uint32_t ea = SIMM16;
	uint32_t a = RA;
	uint32_t t = RT;

	if(a)
		ea += REG(a);

	WRITE64(ea, FPR(t).id);
}

static void ppc_stfdu(uint32_t op)
{
	uint32_t ea = SIMM16;
	uint32_t a = RA;
	uint32_t t = RT;

	ea += REG(a);

	WRITE64(ea, FPR(t).id);

	REG(a) = ea;
}

static void ppc_lfdux(uint32_t op)
{
	uint32_t ea = REG(RB);
	uint32_t a = RA;
	uint32_t d = RD;

	ea += REG(a);

	FPR(d).id = READ64(ea);

	REG(a) = ea;
}

static void ppc_lfdx(uint32_t op)
{
	uint32_t ea = REG(RB);
	uint32_t a = RA;
	uint32_t d = RD;

	if(a)
		ea += REG(a);

	FPR(d).id = READ64(ea);
}
#endif

static void ppc_lfsux(uint32_t op)
{
	uint32_t ea = REG(RB);
	uint32_t a = RA;
	uint32_t t = RT;
	FPR32 f;

	ea += REG(a);

	f.i = READ32(ea);
	FPR(t).fd = (double)(f.f);

	REG(a) = ea;
}

static void ppc_lfsx(uint32_t op)
{
	uint32_t ea = REG(RB);
	uint32_t a = RA;
	uint32_t t = RT;
	FPR32 f;

	if(a)
		ea += REG(a);

	f.i = READ32(ea);
	FPR(t).fd = (double)(f.f);
}

static void ppc_mfsr(uint32_t op)
{
	uint32_t sr = (op >> 16) & 15;
	uint32_t t = RT;

	CHECK_SUPERVISOR();

	REG(t) = ppc.sr[sr];
}

static void ppc_mfsrin(uint32_t op)
{
	uint32_t b = RB;
	uint32_t t = RT;

	CHECK_SUPERVISOR();

	REG(t) = ppc.sr[REG(b) >> 28];
}

static void ppc_mftb(uint32_t op)
{
	uint32_t x = SPRF;

	switch(x)
	{
		case 268:	REG(RT) = (uint32_t)(ppc_read_timebase()); break;
		case 269:	REG(RT) = (uint32_t)(ppc_read_timebase() >> 32); break;
		default:	fatalerror("ppc: Invalid timebase register %d at %08X", x, ppc.pc); break;
	}
}

static void ppc_mtsr(uint32_t op)
{
	uint32_t sr = (op >> 16) & 15;
	uint32_t t = RT;

	CHECK_SUPERVISOR();

	ppc.sr[sr] = REG(t);
}

static void ppc_mtsrin(uint32_t op)
{
	uint32_t b = RB;
	uint32_t t = RT;

	CHECK_SUPERVISOR();

	ppc.sr[REG(b) >> 28] = REG(t);
}

#ifndef PPC_DRC
static void ppc_dcba(uint32_t op)
{
	/* TODO: Cache not emulated so this opcode doesn't need to be implemented */
}

static void ppc_stfdux(uint32_t op)
{
	uint32_t ea = REG(RB);
	uint32_t a = RA;
	uint32_t t = RT;

	ea += REG(a);

	WRITE64(ea, FPR(t).id);

	REG(a) = ea;
}
#endif

static void ppc_stfdx(uint32_t op)
{
	uint32_t ea = REG(RB);
	uint32_t a = RA;
	uint32_t t = RT;

	if(a)
		ea += REG(a);

	WRITE64(ea, FPR(t).id);
}

static void ppc_stfiwx(uint32_t op)
{
	uint32_t ea = REG(RB);
	uint32_t a = RA;
	uint32_t t = RT;

	if(a)
		ea += REG(a);

	WRITE32(ea, (uint32_t)FPR(t).id);
}

static void ppc_stfsux(uint32_t op)
{
	uint32_t ea = REG(RB);
	uint32_t a = RA;
	uint32_t t = RT;
	FPR32 f;

	ea += REG(a);

	f.f = (float)(FPR(t).fd);
	WRITE32(ea, f.i);

	REG(a) = ea;
}

static void ppc_stfsx(uint32_t op)
{
	uint32_t ea = REG(RB);
	uint32_t a = RA;
	uint32_t t = RT;
	FPR32 f;

	if(a)
		ea += REG(a);

	f.f = (float)(FPR(t).fd);

	WRITE32(ea, f.i);
}

#ifndef PPC_DRC
static void ppc_tlbia(uint32_t op)
{
	/* TODO: TLB not emulated so this opcode doesn't need to implemented */
}

static void ppc_tlbie(uint32_t op)
{
	/* TODO: TLB not emulated so this opcode doesn't need to implemented */
}

static void ppc_tlbsync(uint32_t op)
{
	/* TODO: TLB not emulated so this opcode doesn't need to implemented */
}

static void ppc_eciwx(uint32_t op)
{
	ppc_unimplemented(op);
}

static void ppc_ecowx(uint32_t op)
{
	ppc_unimplemented(op);
}
#endif

static void ppc_fabsx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	FPR(t).id = FPR(b).id & ~DOUBLE_SIGN;

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_faddx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));

	FPR(t).fd = FPR(a).fd + FPR(b).fd;

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fcmpo(uint32_t op)
{
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = (RT >> 2);
	uint32_t c;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));

	if(is_nan_double(FPR(a)) || is_nan_double(FPR(b)))
	{
		c = 1; /* OX */
		if(is_snan_double(FPR(a)) || is_snan_double(FPR(b))) {
			ppc.fpscr |= 0x01000000; /* VXSNAN */

			if(!(ppc.fpscr & 0x40000000) || is_qnan_double(FPR(a)) || is_qnan_double(FPR(b)))
				ppc.fpscr |= 0x00080000; /* VXVC */
		}
	}
	else if(FPR(a).fd < FPR(b).fd){
		c = 8; /* FX */
	}
	else if(FPR(a).fd > FPR(b).fd){
		c = 4; /* FEX */
	}
	else {
		c = 2; /* VX */
	}

	CR(t) = c;

	ppc.fpscr &= ~0x0001F000;
	ppc.fpscr |= (c << 12);
}

static void ppc_fcmpu(uint32_t op)
{
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = (RT >> 2);
	uint32_t c;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));

    if(is_nan_double(FPR(a)) || is_nan_double(FPR(b)))
	{
		c = 1; /* OX */
		if(is_snan_double(FPR(a)) || is_snan_double(FPR(b))) {
			ppc.fpscr |= 0x01000000; /* VXSNAN */
		}
	}
	else if(FPR(a).fd < FPR(b).fd){
		c = 8; /* FX */
	}
	else if(FPR(a).fd > FPR(b).fd){
		c = 4; /* FEX */
	}
	else {
		c = 2; /* VX */
	}

	CR(t) = c;

	ppc.fpscr &= ~0x0001F000;
	ppc.fpscr |= (c << 12);
}

static void ppc_fctiwx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t t = RT;
	int64_t r = 0;

	// TODO: fix FPSCR flags FX,VXSNAN,VXCVI

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN_1(FPR(b));

	switch(ppc.fpscr & 3)
	{
		case 0: r = (int64_t)round_to_nearest(FPR(b)); break;
		case 1: r = (int64_t)round_toward_zero(FPR(b)); break;
		case 2: r = (int64_t)round_toward_positive_infinity(FPR(b)); break;
		case 3: r = (int64_t)round_toward_negative_infinity(FPR(b)); break;
	}

    if(r > (int64_t)((int32_t)0x7FFFFFFF))
	{
		FPR(t).id = 0x7FFFFFFF;
		// FPSCR[FR] = 0
		// FPSCR[FI] = 1
		// FPSCR[XX] = 1
	}
	else if(FPR(b).fd < (int64_t)((int32_t)0x80000000))
	{
		FPR(t).id = 0x80000000;
		// FPSCR[FR] = 1
		// FPSCR[FI] = 1
		// FPSCR[XX] = 1
	}
	else
	{
		FPR(t).id = (uint32_t)r;
		// FPSCR[FR] = t.iw > t.fd
		// FPSCR[FI] = t.iw == t.fd
		// FPSCR[XX] = ?
	}

	// FPSCR[FPRF] = undefined (leave it as is)
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fctiwzx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t t = RT;
	int64_t r;

	// TODO: fix FPSCR flags FX,VXSNAN,VXCVI

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN_1(FPR(b));
	r = round_toward_zero(FPR(b));

    if(r > (int64_t)((int32_t)0x7fffffff))
	{
		FPR(t).id = 0x7fffffff;
		// FPSCR[FR] = 0
		// FPSCR[FI] = 1
		// FPSCR[XX] = 1

	}
	else if(r < (int64_t)((int32_t)0x80000000))
	{
		FPR(t).id = 0x80000000;
		// FPSCR[FR] = 1
		// FPSCR[FI] = 1
		// FPSCR[XX] = 1
	}
	else
	{
		FPR(t).id = (uint32_t)r;
		// FPSCR[FR] = t.iw > t.fd
		// FPSCR[FI] = t.iw == t.fd
		// FPSCR[XX] = ?
	}

	// FPSCR[FPRF] = undefined (leave it as is)
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fdivx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));

    FPR(t).fd = FPR(a).fd / FPR(b).fd;

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmrx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	FPR(t).fd = FPR(b).fd;

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fnabsx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	FPR(t).id = FPR(b).id | DOUBLE_SIGN;

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fnegx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	FPR(t).id = FPR(b).id ^ DOUBLE_SIGN;

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_frspx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN_1(FPR(b));

	FPR(t).fd = (float)FPR(b).fd;

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_frsqrtex(uint32_t op)
{
	uint32_t b = RB;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN_1(FPR(b));

	FPR(t).fd = 1.0 / sqrt(FPR(b).fd);	/* verify this */

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fsqrtx(uint32_t op)
{
	/* NOTE: PPC603e doesn't support this opcode */
	uint32_t b = RB;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN_1(FPR(b));

	FPR(t).fd = (double)(sqrt(FPR(b).fd));

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fsubx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));

	FPR(t).fd = FPR(a).fd - FPR(b).fd;

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_mffsx(uint32_t op)
{
	FPR(RT).id = (uint32_t)ppc.fpscr;

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_mtfsb0x(uint32_t op)
{
    uint32_t crbD;

    crbD = (op >> 21) & 0x1F;

    if (crbD != 1 && crbD != 2) // these bits cannot be explicitly cleared
        ppc.fpscr &= ~(1 << (31 - crbD));

    if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_mtfsb1x(uint32_t op)
{
    uint32_t crbD;

    crbD = (op >> 21) & 0x1F;

    if (crbD != 1 && crbD != 2) // these bits cannot be explicitly cleared
        ppc.fpscr |= (1 << (31 - crbD));

    if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_mtfsfx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t f = FM;

	f = ppc_field_xlat[FM];

	ppc.fpscr &= (~f) | ~(FPSCR_FEX | FPSCR_VX);
	ppc.fpscr |= (uint32_t)(FPR(b).id) & ~(FPSCR_FEX | FPSCR_VX);

	// FEX, VX

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_mtfsfix(uint32_t op)
{
    uint32_t crfd = CRFD;
    uint32_t imm = (op >> 12) & 0xF;

    /*
     * According to the manual:
     *
     * If bits 0 and 3 of FPSCR are to be modified, they take the immediate
     * value specified. Bits 1 and 2 (FEX and VX) are set according to the
     * "usual rule" and not from IMM[1-2].
     *
     * The "usual rule" is not emulated, so these bits simply aren't modified
     * at all here.
     */

    crfd = (7 - crfd) * 4;  // calculate LSB position of field

    if (crfd == 28)         // field containing FEX and VX is special...
    {                       // bits 1 and 2 of FPSCR must not be altered
        ppc.fpscr &= 0x9fffffff;
        ppc.fpscr |= (imm & 0x9fffffff);
    }

    ppc.fpscr &= ~(0xf << crfd);    // clear field
    ppc.fpscr |= (imm << crfd);     // insert new data

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_mcrfs(uint32_t op)
{
	uint32_t crfs, f;
	crfs = CRFA;

	f = ppc.fpscr >> ((7 - crfs) * 4);	// get crfS field from FPSCR
	f &= 0xf;

	switch(crfs)	// determine which exception bits to clear in FPSCR
	{
		case 0:		// FX, OX
			ppc.fpscr &= ~0x90000000;
			break;
		case 1:		// UX, ZX, XX, VXSNAN
			ppc.fpscr &= ~0x0f000000;
			break;
		case 2:		// VXISI, VXIDI, VXZDZ, VXIMZ
			ppc.fpscr &= ~0x00F00000;
			break;
		case 3:		// VXVC
			ppc.fpscr &= ~0x00080000;
			break;
		case 5:		// VXSOFT, VXSQRT, VXCVI
			ppc.fpscr &= ~0x00000e00;
			break;
		default:
			break;
	}

	CR(CRFD) = f;
}

static void ppc_faddsx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));

	FPR(t).fd = (float)(FPR(a).fd + FPR(b).fd);

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fdivsx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));

	FPR(t).fd = (float)(FPR(a).fd / FPR(b).fd);

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fresx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN_1(FPR(b));

	FPR(t).fd = 1.0 / FPR(b).fd; /* ??? */

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fsqrtsx(uint32_t op)
{
	/* NOTE: This opcode is not supported in PPC603e */
	uint32_t b = RB;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN_1(FPR(b));

	FPR(t).fd = (float)(sqrt(FPR(b).fd));

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fsubsx(uint32_t op)
{
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));

	FPR(t).fd = (float)(FPR(a).fd - FPR(b).fd);

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmaddx(uint32_t op)
{
	uint32_t c = RC;
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));
    SET_VXSNAN_1(FPR(c));

	FPR(t).fd = ((FPR(a).fd * FPR(c).fd) + FPR(b).fd);

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmsubx(uint32_t op)
{
	uint32_t c = RC;
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));
    SET_VXSNAN_1(FPR(c));

    FPR(t).fd = ((FPR(a).fd * FPR(c).fd) - FPR(b).fd);

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmulx(uint32_t op)
{
	uint32_t c = RC;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(c));

	FPR(t).fd = (FPR(a).fd * FPR(c).fd);

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fnmaddx(uint32_t op)
{
	uint32_t c = RC;
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));
	SET_VXSNAN_1(FPR(c));

	FPR(t).fd = (-((FPR(a).fd * FPR(c).fd) + FPR(b).fd));

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fnmsubx(uint32_t op)
{
	uint32_t c = RC;
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));
	SET_VXSNAN_1(FPR(c));

	FPR(t).fd = (-((FPR(a).fd * FPR(c).fd) - FPR(b).fd));

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fselx(uint32_t op)
{
	uint32_t c = RC;
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	FPR(t).fd = (FPR(a).fd >= 0.0) ? FPR(c).fd : FPR(b).fd;

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmaddsx(uint32_t op)
{
	uint32_t c = RC;
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));
	SET_VXSNAN_1(FPR(c));

	FPR(t).fd = (float)((FPR(a).fd * FPR(c).fd) + FPR(b).fd);

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmsubsx(uint32_t op)
{
	uint32_t c = RC;
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));
	SET_VXSNAN_1(FPR(c));

	FPR(t).fd = (float)((FPR(a).fd * FPR(c).fd) - FPR(b).fd);

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmulsx(uint32_t op)
{
	uint32_t c = RC;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();
	SET_VXSNAN(FPR(a), FPR(c));

	FPR(t).fd = (float)(FPR(a).fd * FPR(c).fd);

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fnmaddsx(uint32_t op)
{
	uint32_t c = RC;
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));
	SET_VXSNAN_1(FPR(c));

    FPR(t).fd = (float)(-((FPR(a).fd * FPR(c).fd) + FPR(b).fd));

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fnmsubsx(uint32_t op)
{
	uint32_t c = RC;
	uint32_t b = RB;
	uint32_t a = RA;
	uint32_t t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));
	SET_VXSNAN_1(FPR(c));

	FPR(t).fd = (float)(-((FPR(a).fd * FPR(c).fd) - FPR(b).fd));

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}
