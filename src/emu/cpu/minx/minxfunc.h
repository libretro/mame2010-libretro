INLINE uint8_t ADD8( minx_state *minx, uint8_t arg1, uint8_t arg2 )
{
	uint32_t res = arg1 + arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x80 ) & ( arg2 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE uint16_t ADD16( minx_state *minx, uint16_t arg1, uint16_t arg2 )
{
	uint32_t res = arg1 + arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x8000 ) & ( arg2 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


INLINE uint8_t ADDC8( minx_state *minx, uint8_t arg1, uint8_t arg2 )
{
        uint32_t res = arg1 + arg2 + ( ( minx->F & FLAG_C ) ? 1 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x80 ) & ( arg2 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE uint16_t ADDC16( minx_state *minx, uint16_t arg1, uint16_t arg2 )
{
	uint32_t res = arg1 + arg2 + ( ( minx->F & FLAG_C ) ? 1 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ^ 0x8000 ) & ( arg2 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


INLINE uint8_t INC8( minx_state *minx, uint8_t arg )
{
	uint8_t old_F = minx->F;
	uint8_t res = ADD8( minx, arg, 1 );
	minx->F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE uint16_t INC16( minx_state *minx, uint16_t arg )
{
	uint8_t old_F = minx->F;
	uint16_t res = ADD16( minx, arg, 1 );
	minx->F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE uint8_t SUB8( minx_state *minx, uint8_t arg1, uint8_t arg2 )
{
	uint32_t res = arg1 - arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE uint16_t SUB16( minx_state *minx, uint16_t arg1, uint16_t arg2 )
{
	uint32_t res = arg1 - arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


INLINE uint8_t SUBC8( minx_state *minx, uint8_t arg1, uint8_t arg2 )
{
	uint32_t res = arg1 - arg2 - ( ( minx->F & FLAG_C ) ? 1 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x80 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF00 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE uint16_t SUBC16( minx_state *minx, uint16_t arg1, uint16_t arg2 )
{
	uint32_t res = arg1 - arg2 - ( ( minx->F & FLAG_C ) ? 1 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x8000 ) ? FLAG_S : 0 )
		| ( ( ( arg2 ^ arg1 ) & ( arg1 ^ res ) & 0x8000 ) ? FLAG_O : 0 )
		| ( ( res & 0xFF0000 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFFFF;
}


INLINE uint8_t DEC8( minx_state *minx, uint8_t arg )
{
	uint8_t old_F = minx->F;
	uint8_t res = SUB8( minx, arg, 1 );
	minx->F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE uint16_t DEC16( minx_state *minx, uint16_t arg )
{
	uint8_t old_F = minx->F;
	uint16_t res = SUB16( minx, arg, 1 );
	minx->F = ( old_F & ~ ( FLAG_Z ) )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE uint8_t AND8( minx_state *minx, uint8_t arg1, uint8_t arg2 )
{
	uint8_t res = arg1 & arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE uint8_t OR8( minx_state *minx, uint8_t arg1, uint8_t arg2 )
{
	uint8_t res = arg1 | arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE uint8_t XOR8( minx_state *minx, uint8_t arg1, uint8_t arg2 )
{
	uint8_t res = arg1 ^ arg2;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE uint8_t NOT8( minx_state *minx, uint8_t arg )
{
	uint8_t res = ~arg;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE uint8_t NEG8( minx_state *minx, uint8_t arg )
{
	uint8_t res = -arg;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE uint8_t SAL8( minx_state *minx, uint8_t arg )
{
	uint16_t res = arg << 1;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg != 0 && res == 0 ) ? FLAG_O : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE uint8_t SAR8( minx_state *minx, uint8_t arg )
{
	uint16_t res = ( arg >> 1 ) | ( arg & 0x80 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_O | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg != 0x80 && res == 0x80 ) ? FLAG_O : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE uint8_t SHL8( minx_state *minx, uint8_t arg )
{
	uint16_t res = arg << 1;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res;
}


INLINE uint8_t SHR8( minx_state *minx, uint8_t arg )
{
	uint16_t res = arg >> 1;
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE uint8_t ROLC8( minx_state *minx, uint8_t arg )
{
	uint16_t res = ( arg << 1 ) | ( ( minx->F & FLAG_C ) ? 1 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE uint8_t RORC8( minx_state *minx, uint8_t arg )
{
	uint16_t res = ( arg >> 1 ) | ( ( minx->F & FLAG_C ) ? 0x80 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE uint8_t ROL8( minx_state *minx, uint8_t arg )
{
	uint16_t res = ( arg << 1 ) | ( ( arg & 0x80 ) ? 1 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x80 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE uint8_t ROR8( minx_state *minx, uint8_t arg )
{
	uint16_t res = ( arg >> 1 ) | ( ( arg & 0x01 ) ? 0x80 : 0 );
	minx->F = ( minx->F & ~ ( FLAG_S | FLAG_C | FLAG_Z ) )
		| ( ( res & 0x80 ) ? FLAG_S : 0 )
		| ( ( arg & 0x01 ) ? FLAG_C : 0 )
		| ( ( res ) ? 0 : FLAG_Z )
	;
	return res & 0xFF;
}


INLINE void PUSH8( minx_state *minx, uint8_t arg )
{
	minx->SP = minx->SP - 1;
	WR( minx->SP, arg );
}


INLINE void PUSH16( minx_state *minx, uint16_t arg )
{
	PUSH8( minx, arg >> 8 );
	PUSH8( minx, arg & 0x00FF );
}


INLINE uint8_t POP8( minx_state *minx )
{
	uint8_t res = RD( minx->SP );
	minx->SP = minx->SP + 1;
	return res;
}


INLINE uint16_t POP16( minx_state *minx )
{
	return POP8(minx) | ( POP8(minx) << 8 );
}


INLINE void JMP( minx_state *minx, uint16_t arg )
{
	minx->V = minx->U;
	minx->PC = arg;
}


INLINE void CALL( minx_state *minx, uint16_t arg )
{
	PUSH8( minx, minx->V );
	PUSH16( minx, minx->PC );
	JMP( minx, arg );
}


#define AD1_IHL	uint32_t addr1 = ( minx->I << 16 ) | minx->HL
#define AD1_IN8	uint32_t addr1 = ( minx->I << 16 ) | ( minx->N << 8 ) | rdop(minx)
#define AD1_I16	uint32_t addr1 = ( minx->I << 16 ) | rdop16(minx)
#define AD1_XIX	uint32_t addr1 = ( minx->XI << 16 ) | minx->X
#define AD1_YIY	uint32_t addr1 = ( minx->YI << 16 ) | minx->Y
#define AD1_X8	uint32_t addr1 = ( minx->XI << 16 ) | ( minx->X + rdop(minx) )
#define AD1_Y8	uint32_t addr1 = ( minx->YI << 16 ) | ( minx->Y + rdop(minx) )
#define AD1_XL	uint32_t addr1 = ( minx->XI << 16 ) | ( minx->X + ( minx->HL & 0x00FF ) )
#define AD1_YL	uint32_t addr1 = ( minx->YI << 16 ) | ( minx->Y + ( minx->HL & 0x00FF ) )
#define AD2_IHL	uint32_t addr2 = ( minx->I << 16 ) | minx->HL
#define AD2_IN8	uint32_t addr2 = ( minx->I << 16 ) | ( minx->N << 8 ) | rdop(minx)
#define AD2_I16	uint32_t addr2 = ( minx->I << 16 ) | rdop(minx); addr2 |= ( rdop(minx) << 8 )
#define AD2_XIX	uint32_t addr2 = ( minx->XI << 16 ) | minx->X
#define AD2_YIY	uint32_t addr2 = ( minx->YI << 16 ) | minx->Y
#define AD2_X8	uint32_t addr2 = ( minx->XI << 16 ) | ( minx->X + rdop(minx) )
#define AD2_Y8	uint32_t addr2 = ( minx->YI << 16 ) | ( minx->Y + rdop(minx) )
#define AD2_XL	uint32_t addr2 = ( minx->XI << 16 ) | ( minx->X + ( minx->HL & 0x00FF ) )
#define AD2_YL	uint32_t addr2 = ( minx->YI << 16 ) | ( minx->Y + ( minx->HL & 0x00FF ) )
