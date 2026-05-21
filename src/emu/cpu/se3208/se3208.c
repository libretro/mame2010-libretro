#include "emu.h"
#include "debugger.h"
#include "se3208.h"

/*
    SE3208 CPU Emulator by ElSemi

    For information about this CPU:
    www.adc.co.kr

*/

typedef struct _se3208_state_t se3208_state_t;
struct _se3208_state_t
{
	//GPR
	uint32_t R[8];
	//SPR
	uint32_t PC;
	uint32_t SR;
	uint32_t SP;
	uint32_t ER;
	uint32_t PPC;

	device_irq_callback irq_callback;
	legacy_cpu_device *device;
	const address_space *program;
	uint8_t IRQ;
	uint8_t NMI;

	int icount;
};

#define FLAG_C		0x0080
#define FLAG_V		0x0010
#define FLAG_S		0x0020
#define FLAG_Z		0x0040

#define FLAG_M		0x0200
#define FLAG_E		0x0800
#define FLAG_AUT	0x1000
#define FLAG_ENI	0x2000
#define FLAG_NMI	0x4000

#define CLRFLAG(f)	se3208_state->SR&=~(f);
#define SETFLAG(f)	se3208_state->SR|=(f);
#define TESTFLAG(f)	(se3208_state->SR&(f))

#define EXTRACT(val,sbit,ebit)	(((val)>>sbit)&((1<<((ebit-sbit)+1))-1))
#define SEX8(val)	((val&0x80)?(val|0xFFFFFF00):(val&0xFF))
#define SEX16(val)	((val&0x8000)?(val|0xFFFF0000):(val&0xFFFF))
#define ZEX8(val)	((val)&0xFF)
#define ZEX16(val)	((val)&0xFFFF)
#define SEX(bits,val)	((val)&(1<<(bits-1))?((val)|(~((1<<bits)-1))):(val&((1<<bits)-1)))

//Precompute the instruction decoding in a big table
typedef void (*_OP)(se3208_state_t *se3208_state, uint16_t Opcode);
#define INST(a) static void a(se3208_state_t *se3208_state, uint16_t Opcode)
static _OP *OpTable=NULL;

INLINE se3208_state_t *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SE3208);
	return (se3208_state_t *)downcast<legacy_cpu_device *>(device)->token();
}

INLINE uint32_t read_dword_unaligned(const address_space *space, uint32_t address)
{
	if (address & 3)
		return memory_read_byte_32le(space,address) | memory_read_byte_32le(space,address+1)<<8 | memory_read_byte_32le(space,address+2)<<16 | memory_read_byte_32le(space,address+3)<<24;
	else
		return memory_read_dword_32le(space,address);
}

INLINE uint16_t read_word_unaligned(const address_space *space, uint32_t address)
{
	if (address & 1)
		return memory_read_byte_32le(space,address) | memory_read_byte_32le(space,address+1)<<8;
	else
		return memory_read_word_32le(space,address);
}

INLINE void write_dword_unaligned(const address_space *space, uint32_t address, uint32_t data)
{
	if (address & 3)
	{
		memory_write_byte_32le(space, address, data & 0xff);
		memory_write_byte_32le(space, address+1, (data>>8)&0xff);
		memory_write_byte_32le(space, address+2, (data>>16)&0xff);
		memory_write_byte_32le(space, address+3, (data>>24)&0xff);
	}
	else
	{
		memory_write_dword_32le(space, address, data);
	}
}

INLINE void write_word_unaligned(const address_space *space, uint32_t address, uint16_t data)
{
	if (address & 1)
	{
		memory_write_byte_32le(space, address, data & 0xff);
		memory_write_byte_32le(space, address+1, (data>>8)&0xff);
	}
	else
	{
		memory_write_word_32le(space, address, data);
	}
}


INLINE uint8_t SE3208_Read8(se3208_state_t *se3208_state, uint32_t addr)
{
	return memory_read_byte_32le(se3208_state->program,addr);
}

INLINE uint16_t SE3208_Read16(se3208_state_t *se3208_state, uint32_t addr)
{
	return read_word_unaligned(se3208_state->program,addr);
}

INLINE uint32_t SE3208_Read32(se3208_state_t *se3208_state, uint32_t addr)
{
	return read_dword_unaligned(se3208_state->program,addr);
}

INLINE void SE3208_Write8(se3208_state_t *se3208_state, uint32_t addr,uint8_t val)
{
	memory_write_byte_32le(se3208_state->program,addr,val);
}

INLINE void SE3208_Write16(se3208_state_t *se3208_state, uint32_t addr,uint16_t val)
{
	write_word_unaligned(se3208_state->program,addr,val);
}

INLINE void SE3208_Write32(se3208_state_t *se3208_state, uint32_t addr,uint32_t val)
{
	write_dword_unaligned(se3208_state->program,addr,val);
}



INLINE uint32_t AddWithFlags(se3208_state_t *se3208_state, uint32_t a,uint32_t b)
{
	uint32_t r=a+b;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!r)
		SETFLAG(FLAG_Z);
	if(r&0x80000000)
		SETFLAG(FLAG_S);
	if(((((a&b)|(~r&(a|b)))>>31))&1)
		SETFLAG(FLAG_C);
	if(((((a^r)&(b^r))>>31))&1)
		SETFLAG(FLAG_V);
	return r;
}

INLINE uint32_t SubWithFlags(se3208_state_t *se3208_state, uint32_t a,uint32_t b)	//a-b
{
	uint32_t r=a-b;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!r)
		SETFLAG(FLAG_Z);
	if(r&0x80000000)
		SETFLAG(FLAG_S);
	if((((b&r)|(~a&(b|r)))>>31)&1)
		SETFLAG(FLAG_C);
	if((((b^a)&(r^a))>>31)&1)
		SETFLAG(FLAG_V);
	return r;
}

INLINE uint32_t AdcWithFlags(se3208_state_t *se3208_state,uint32_t a,uint32_t b)
{
	uint32_t C=(se3208_state->SR&FLAG_C)?1:0;
	uint32_t r=a+b+C;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!r)
		SETFLAG(FLAG_Z);
	if(r&0x80000000)
		SETFLAG(FLAG_S);
	if(((((a&b)|(~r&(a|b)))>>31))&1)
		SETFLAG(FLAG_C);
	if(((((a^r)&(b^r))>>31))&1)
		SETFLAG(FLAG_V);
	return r;

}

INLINE uint32_t SbcWithFlags(se3208_state_t *se3208_state,uint32_t a,uint32_t b)
{
	uint32_t C=(se3208_state->SR&FLAG_C)?1:0;
	uint32_t r=a-b-C;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!r)
		SETFLAG(FLAG_Z);
	if(r&0x80000000)
		SETFLAG(FLAG_S);
	if((((b&r)|(~a&(b|r)))>>31)&1)
		SETFLAG(FLAG_C);
	if((((b^a)&(r^a))>>31)&1)
		SETFLAG(FLAG_V);
	return r;
}

INLINE uint32_t MulWithFlags(se3208_state_t *se3208_state,uint32_t a,uint32_t b)
{
	int64_t r=(int64_t) a*(int64_t) b;
	CLRFLAG(FLAG_V);
	if(r>>32)
		SETFLAG(FLAG_V);
	return (uint32_t) (r&0xffffffff);
}

INLINE uint32_t NegWithFlags(se3208_state_t *se3208_state,uint32_t a)
{
	return SubWithFlags(se3208_state,0,a);
}

INLINE uint32_t AsrWithFlags(se3208_state_t *se3208_state,uint32_t Val, uint8_t By)
{
	signed int v=(signed int) Val;
	v>>=By;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!v)
		SETFLAG(FLAG_Z);
	if(v&0x80000000)
		SETFLAG(FLAG_S);
	if(Val&(1<<(By-1)))
		SETFLAG(FLAG_C);
	return (uint32_t) v;
}

INLINE uint32_t LsrWithFlags(se3208_state_t *se3208_state,uint32_t Val, uint8_t By)
{
	uint32_t v=Val;
	v>>=By;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!v)
		SETFLAG(FLAG_Z);
	if(v&0x80000000)
		SETFLAG(FLAG_S);
	if(Val&(1<<(By-1)))
		SETFLAG(FLAG_C);
	return v;
}

INLINE uint32_t AslWithFlags(se3208_state_t *se3208_state,uint32_t Val, uint8_t By)
{
	uint32_t v=Val;
	v<<=By;
	CLRFLAG(FLAG_Z|FLAG_C|FLAG_V|FLAG_S);
	if(!v)
		SETFLAG(FLAG_Z);
	if(v&0x80000000)
		SETFLAG(FLAG_S);
	if(Val&(1<<(32-By)))
		SETFLAG(FLAG_C);
	return v;
}


INST(INVALIDOP)
{
	//assert(false);
}

INST(LDB)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);
	uint32_t Val;

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=SEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(STB)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write8(se3208_state, Index+Offset,ZEX8(se3208_state->R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LDS)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);
	uint32_t Val;

	Offset<<=1;

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=SEX16(Val);

	CLRFLAG(FLAG_E);
}

INST(STS)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=1;

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write16(se3208_state, Index+Offset,ZEX16(se3208_state->R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LD)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	se3208_state->R[SrcDst]=SE3208_Read32(se3208_state, Index+Offset);

	CLRFLAG(FLAG_E);
}

INST(ST)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write32(se3208_state, Index+Offset,se3208_state->R[SrcDst]);

	CLRFLAG(FLAG_E);
}

INST(LDBU)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);
	uint32_t Val;

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=ZEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(LDSU)
{
	uint32_t Offset=EXTRACT(Opcode,0,4);
	uint32_t Index=EXTRACT(Opcode,5,7);
	uint32_t SrcDst=EXTRACT(Opcode,8,10);
	uint32_t Val;

	Offset<<=1;

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=ZEX16(Val);

	CLRFLAG(FLAG_E);
}


INST(LERI)
{
	uint32_t Imm=EXTRACT(Opcode,0,13);
	if(TESTFLAG(FLAG_E))
		se3208_state->ER=(EXTRACT(se3208_state->ER,0,17)<<14)|Imm;
	else
		se3208_state->ER=SEX(14,Imm);


	SETFLAG(FLAG_E);
}

INST(LDSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Index=se3208_state->SP;
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	se3208_state->R[SrcDst]=SE3208_Read32(se3208_state, Index+Offset);

	CLRFLAG(FLAG_E);
}

INST(STSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t Index=se3208_state->SP;
	uint32_t SrcDst=EXTRACT(Opcode,8,10);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write32(se3208_state, Index+Offset,se3208_state->R[SrcDst]);

	CLRFLAG(FLAG_E);
}

static void PushVal(se3208_state_t *se3208_state, uint32_t Val)
{
	se3208_state->SP-=4;
	SE3208_Write32(se3208_state, se3208_state->SP,Val);
}

static uint32_t PopVal(se3208_state_t *se3208_state)
{
	uint32_t Val=SE3208_Read32(se3208_state, se3208_state->SP);
	se3208_state->SP+=4;
	return Val;
}

INST(PUSH)
{
	uint32_t Set=EXTRACT(Opcode,0,10);
	if(Set&(1<<10))
		PushVal(se3208_state,se3208_state->PC);
	if(Set&(1<<9))
		PushVal(se3208_state,se3208_state->SR);
	if(Set&(1<<8))
		PushVal(se3208_state,se3208_state->ER);
	if(Set&(1<<7))
		PushVal(se3208_state,se3208_state->R[7]);
	if(Set&(1<<6))
		PushVal(se3208_state,se3208_state->R[6]);
	if(Set&(1<<5))
		PushVal(se3208_state,se3208_state->R[5]);
	if(Set&(1<<4))
		PushVal(se3208_state,se3208_state->R[4]);
	if(Set&(1<<3))
		PushVal(se3208_state,se3208_state->R[3]);
	if(Set&(1<<2))
		PushVal(se3208_state,se3208_state->R[2]);
	if(Set&(1<<1))
		PushVal(se3208_state,se3208_state->R[1]);
	if(Set&(1<<0))
		PushVal(se3208_state,se3208_state->R[0]);
}

INST(POP)
{
	uint32_t Set=EXTRACT(Opcode,0,10);
	if(Set&(1<<0))
		se3208_state->R[0]=PopVal(se3208_state);
	if(Set&(1<<1))
		se3208_state->R[1]=PopVal(se3208_state);
	if(Set&(1<<2))
		se3208_state->R[2]=PopVal(se3208_state);
	if(Set&(1<<3))
		se3208_state->R[3]=PopVal(se3208_state);
	if(Set&(1<<4))
		se3208_state->R[4]=PopVal(se3208_state);
	if(Set&(1<<5))
		se3208_state->R[5]=PopVal(se3208_state);
	if(Set&(1<<6))
		se3208_state->R[6]=PopVal(se3208_state);
	if(Set&(1<<7))
		se3208_state->R[7]=PopVal(se3208_state);
	if(Set&(1<<8))
		se3208_state->ER=PopVal(se3208_state);
	if(Set&(1<<9))
		se3208_state->SR=PopVal(se3208_state);
	if(Set&(1<<10))
	{
		se3208_state->PC=PopVal(se3208_state)-2;		//PC automatically incresases by 2
	}
}

INST(LEATOSP)
{
	uint32_t Offset=EXTRACT(Opcode,9,12);
	uint32_t Index=EXTRACT(Opcode,3,5);

	if(Index)
		Index=se3208_state->R[Index];
	else
		Index=0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);
	else
		Offset=SEX(4,Offset);

	se3208_state->SP=Index+Offset;

	CLRFLAG(FLAG_E);
}

INST(LEAFROMSP)
{
	uint32_t Offset=EXTRACT(Opcode,9,12);
	uint32_t Index=EXTRACT(Opcode,3,5);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);
	else
		Offset=SEX(4,Offset);

	se3208_state->R[Index]=se3208_state->SP+Offset;

	CLRFLAG(FLAG_E);
}

INST(LEASPTOSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	Offset<<=2;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,23)<<8)|(Offset&0xff);
	else
		Offset=SEX(10,Offset);

	se3208_state->SP=se3208_state->SP+Offset;

	CLRFLAG(FLAG_E);
}

INST(MOV)
{
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,9,11);

	se3208_state->R[Dst]=se3208_state->R[Src];
}

INST(LDI)
{
	uint32_t Dst=EXTRACT(Opcode,8,10);
	uint32_t Imm=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX8(Imm);

	se3208_state->R[Dst]=Imm;

	CLRFLAG(FLAG_E);
}

INST(LDBSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,3);
	uint32_t Index=se3208_state->SP;
	uint32_t SrcDst=EXTRACT(Opcode,4,6);
	uint32_t Val;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=SEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(STBSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,3);
	uint32_t Index=se3208_state->SP;
	uint32_t SrcDst=EXTRACT(Opcode,4,6);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write8(se3208_state, Index+Offset,ZEX8(se3208_state->R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LDSSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,3);
	uint32_t Index=se3208_state->SP;
	uint32_t SrcDst=EXTRACT(Opcode,4,6);
	uint32_t Val;

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=SEX16(Val);

	CLRFLAG(FLAG_E);
}

INST(STSSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,3);
	uint32_t Index=se3208_state->SP;
	uint32_t SrcDst=EXTRACT(Opcode,4,6);

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	SE3208_Write16(se3208_state, Index+Offset,ZEX16(se3208_state->R[SrcDst]));

	CLRFLAG(FLAG_E);
}

INST(LDBUSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,3);
	uint32_t Index=se3208_state->SP;
	uint32_t SrcDst=EXTRACT(Opcode,4,6);
	uint32_t Val;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read8(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=ZEX8(Val);

	CLRFLAG(FLAG_E);
}

INST(LDSUSP)
{
	uint32_t Offset=EXTRACT(Opcode,0,3);
	uint32_t Index=se3208_state->SP;
	uint32_t SrcDst=EXTRACT(Opcode,4,6);
	uint32_t Val;

	Offset<<=1;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,27)<<4)|(Offset&0xf);

	Val=SE3208_Read16(se3208_state, Index+Offset);
	se3208_state->R[SrcDst]=ZEX16(Val);

	CLRFLAG(FLAG_E);
}

INST(ADDI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=AddWithFlags(se3208_state,se3208_state->R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(SUBI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=SubWithFlags(se3208_state,se3208_state->R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(ADCI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=AdcWithFlags(se3208_state,se3208_state->R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(SBCI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=SbcWithFlags(se3208_state,se3208_state->R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(ANDI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=se3208_state->R[Src]&Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(ORI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=se3208_state->R[Src]|Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(XORI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	se3208_state->R[Dst]=se3208_state->R[Src]^Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(CMPI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	SubWithFlags(se3208_state,se3208_state->R[Src],Imm);

	CLRFLAG(FLAG_E);
}

INST(TSTI)
{
	uint32_t Imm=EXTRACT(Opcode,9,12);
	uint32_t Src=EXTRACT(Opcode,3,5);
	uint32_t Dst;

	if(TESTFLAG(FLAG_E))
		Imm=(EXTRACT(se3208_state->ER,0,27)<<4)|(Imm&0xf);
	else
		Imm=SEX(4,Imm);

	Dst=se3208_state->R[Src]&Imm;

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!Dst)
		SETFLAG(FLAG_Z);
	if(Dst&0x80000000)
		SETFLAG(FLAG_S);
}

INST(ADD)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=AddWithFlags(se3208_state,se3208_state->R[Src1],se3208_state->R[Src2]);
}

INST(SUB)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=SubWithFlags(se3208_state,se3208_state->R[Src1],se3208_state->R[Src2]);
}

INST(ADC)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=AdcWithFlags(se3208_state,se3208_state->R[Src1],se3208_state->R[Src2]);
}

INST(SBC)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=SbcWithFlags(se3208_state,se3208_state->R[Src1],se3208_state->R[Src2]);
}

INST(AND)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=se3208_state->R[Src1]&se3208_state->R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(OR)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=se3208_state->R[Src1]|se3208_state->R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(XOR)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=se3208_state->R[Src1]^se3208_state->R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(CMP)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);

	SubWithFlags(se3208_state,se3208_state->R[Src1],se3208_state->R[Src2]);
}

INST(TST)
{
	uint32_t Src2=EXTRACT(Opcode,9,11);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst;

	Dst=se3208_state->R[Src1]&se3208_state->R[Src2];

	CLRFLAG(FLAG_S|FLAG_Z);
	if(!Dst)
		SETFLAG(FLAG_Z);
	if(Dst&0x80000000)
		SETFLAG(FLAG_S);
}

INST(MULS)
{
	uint32_t Src2=EXTRACT(Opcode,6,8);
	uint32_t Src1=EXTRACT(Opcode,3,5);
	uint32_t Dst=EXTRACT(Opcode,0,2);

	se3208_state->R[Dst]=MulWithFlags(se3208_state,se3208_state->R[Src1],se3208_state->R[Src2]);

	CLRFLAG(FLAG_E);
}

INST(NEG)
{
	uint32_t Dst=EXTRACT(Opcode,9,11);
	uint32_t Src=EXTRACT(Opcode,3,5);

	se3208_state->R[Dst]=NegWithFlags(se3208_state,se3208_state->R[Src]);
}

INST(CALL)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;
	PushVal(se3208_state,se3208_state->PC+2);
	se3208_state->PC=se3208_state->PC+Offset;

	CLRFLAG(FLAG_E);
}

INST(JV)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_V))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);

}

INST(JNV)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_V))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JC)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_C))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JNC)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_C))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JP)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_S))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JM)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_S))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JNZ)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!TESTFLAG(FLAG_Z))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JZ)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JGE)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t S=TESTFLAG(FLAG_S)?1:0;
	uint32_t V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!(S^V))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLE)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t S=TESTFLAG(FLAG_S)?1:0;
	uint32_t V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z) || (S^V))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}
	CLRFLAG(FLAG_E);
}

INST(JHI)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!(TESTFLAG(FLAG_Z) || TESTFLAG(FLAG_C)))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLS)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(TESTFLAG(FLAG_Z) || TESTFLAG(FLAG_C))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JGT)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t S=TESTFLAG(FLAG_S)?1:0;
	uint32_t V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(!(TESTFLAG(FLAG_Z) || (S^V)))
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}

INST(JLT)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);
	uint32_t S=TESTFLAG(FLAG_S)?1:0;
	uint32_t V=TESTFLAG(FLAG_V)?1:0;

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);
	Offset<<=1;

	if(S^V)
	{
		se3208_state->PC=se3208_state->PC+Offset;
	}

	CLRFLAG(FLAG_E);
}



INST(JMP)
{
	uint32_t Offset=EXTRACT(Opcode,0,7);

	if(TESTFLAG(FLAG_E))
		Offset=(EXTRACT(se3208_state->ER,0,22)<<8)|Offset;
	else
		Offset=SEX(8,Offset);

	Offset<<=1;

	se3208_state->PC=se3208_state->PC+Offset;

	CLRFLAG(FLAG_E);
}

INST(JR)
{
	uint32_t Src=EXTRACT(Opcode,0,3);

	se3208_state->PC=se3208_state->R[Src]-2;

	CLRFLAG(FLAG_E);
}

INST(CALLR)
{
	uint32_t Src=EXTRACT(Opcode,0,3);
	PushVal(se3208_state,se3208_state->PC+2);
	se3208_state->PC=se3208_state->R[Src]-2;

	CLRFLAG(FLAG_E);
}

INST(ASR)
{
	uint32_t CS=Opcode&(1<<10);
	uint32_t Dst=EXTRACT(Opcode,0,2);
	uint32_t Imm=EXTRACT(Opcode,5,9);
	uint32_t Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		se3208_state->R[Dst]=AsrWithFlags(se3208_state,se3208_state->R[Dst],se3208_state->R[Cnt]&0x1f);
	else
		se3208_state->R[Dst]=AsrWithFlags(se3208_state,se3208_state->R[Dst],Imm&0x1f);

	CLRFLAG(FLAG_E);
}

INST(LSR)
{
	uint32_t CS=Opcode&(1<<10);
	uint32_t Dst=EXTRACT(Opcode,0,2);
	uint32_t Imm=EXTRACT(Opcode,5,9);
	uint32_t Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		se3208_state->R[Dst]=LsrWithFlags(se3208_state,se3208_state->R[Dst],se3208_state->R[Cnt]&0x1f);
	else
		se3208_state->R[Dst]=LsrWithFlags(se3208_state,se3208_state->R[Dst],Imm&0x1f);

	CLRFLAG(FLAG_E);
}

INST(ASL)
{
	uint32_t CS=Opcode&(1<<10);
	uint32_t Dst=EXTRACT(Opcode,0,2);
	uint32_t Imm=EXTRACT(Opcode,5,9);
	uint32_t Cnt=EXTRACT(Opcode,5,7);

	if(CS)
		se3208_state->R[Dst]=AslWithFlags(se3208_state,se3208_state->R[Dst],se3208_state->R[Cnt]&0x1f);
	else
		se3208_state->R[Dst]=AslWithFlags(se3208_state,se3208_state->R[Dst],Imm&0x1f);

	CLRFLAG(FLAG_E);
}

INST(EXTB)
{
	uint32_t Dst=EXTRACT(Opcode,0,3);
	uint32_t Val=se3208_state->R[Dst];

	se3208_state->R[Dst]=SEX8(Val);

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);

}

INST(EXTS)
{
	uint32_t Dst=EXTRACT(Opcode,0,3);
	uint32_t Val=se3208_state->R[Dst];

	se3208_state->R[Dst]=SEX16(Val);

	CLRFLAG(FLAG_S|FLAG_Z|FLAG_E);
	if(!se3208_state->R[Dst])
		SETFLAG(FLAG_Z);
	if(se3208_state->R[Dst]&0x80000000)
		SETFLAG(FLAG_S);
}

INST(SET)
{
	uint32_t Imm=EXTRACT(Opcode,0,3);

	se3208_state->SR|=(1<<Imm);
}

INST(CLR)
{
	uint32_t Imm=EXTRACT(Opcode,0,3);

	se3208_state->SR&=~(1<<Imm);
}

INST(SWI)
{
	uint32_t Imm=EXTRACT(Opcode,0,3);

	if(!TESTFLAG(FLAG_ENI))
		return;
	PushVal(se3208_state,se3208_state->PC);
	PushVal(se3208_state,se3208_state->SR);

	CLRFLAG(FLAG_ENI|FLAG_E|FLAG_M);

	se3208_state->PC=SE3208_Read32(se3208_state, 4*Imm+0x40)-2;
}

INST(HALT)
{
//  uint32_t Imm=EXTRACT(Opcode,0,3);

//  DEBUGMESSAGE("HALT\t0x%x",Imm);
}

INST(MVTC)
{
//  uint32_t Imm=EXTRACT(Opcode,0,3);

//  DEBUGMESSAGE("MVTC\t%%R0,%%CR%d",Imm);
}

INST(MVFC)
{
//  uint32_t Imm=EXTRACT(Opcode,0,3);

//  DEBUGMESSAGE("MVFC\t%%CR0%d,%%R0",Imm);
}


static _OP DecodeOp(uint16_t Opcode)
{
	switch(EXTRACT(Opcode,14,15))
	{
		case 0x0:
			{
				uint8_t Op=EXTRACT(Opcode,11,13);
				switch(Op)
				{
					case 0x0:
						return LDB;
					case 0x1:
						return LDS;
					case 0x2:
						return LD;
					case 0x3:
						return LDBU;
					case 0x4:
						return STB;
					case 0x5:
						return STS;
					case 0x6:
						return ST;
					case 0x7:
						return LDSU;
				}
			}
			break;
		case 0x1:
			return LERI;
		case 0x2:
			{
				switch(EXTRACT(Opcode,11,13))
				{
					case 0:
						return LDSP;
					case 1:
						return STSP;
					case 2:
						return PUSH;
					case 3:
						return POP;
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:	//arith
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
					case 14:
					case 15:
						switch(EXTRACT(Opcode,6,8))
						{
							case 0:
								return ADDI;
							case 1:
								return ADCI;
							case 2:
								return SUBI;
							case 3:
								return SBCI;
							case 4:
								return ANDI;
							case 5:
								return ORI;
							case 6:
								return XORI;
							case 7:
								switch(EXTRACT(Opcode,0,2))
								{
									case 0:
										return CMPI;
									case 1:
										return TSTI;
									case 2:
										return LEATOSP;
									case 3:
										return LEAFROMSP;
								}
								break;
						}
						break;
				}
			}
			break;
		case 3:
			switch(EXTRACT(Opcode,12,13))
			{
				case 0:
					switch(EXTRACT(Opcode,6,8))
					{
						case 0:
							return ADD;
						case 1:
							return ADC;
						case 2:
							return SUB;
						case 3:
							return SBC;
						case 4:
							return AND;
						case 5:
							return OR;
						case 6:
							return XOR;
						case 7:
							switch(EXTRACT(Opcode,0,2))
							{
								case 0:
									return CMP;
								case 1:
									return TST;
								case 2:
									return MOV;
								case 3:
									return NEG;
							}
							break;
					}
					break;
				case 1:		//Jumps
					switch(EXTRACT(Opcode,8,11))
					{
						case 0x0:
							return JNV;
						case 0x1:
							return JV;
						case 0x2:
							return JP;
						case 0x3:
							return JM;
						case 0x4:
							return JNZ;
						case 0x5:
							return JZ;
						case 0x6:
							return JNC;
						case 0x7:
							return JC;
						case 0x8:
							return JGT;
						case 0x9:
							return JLT;
						case 0xa:
							return JGE;
						case 0xb:
							return JLE;
						case 0xc:
							return JHI;
						case 0xd:
							return JLS;
						case 0xe:
							return JMP;
						case 0xf:
							return CALL;
					}
					break;
				case 2:
					if(Opcode&(1<<11))
						return LDI;
					else	//SP Ops
					{
						if(Opcode&(1<<10))
						{
							switch(EXTRACT(Opcode,7,9))
							{
								case 0:
									return LDBSP;
								case 1:
									return LDSSP;
								case 3:
									return LDBUSP;
								case 4:
									return STBSP;
								case 5:
									return STSSP;
								case 7:
									return LDSUSP;
							}
						}
						else
						{
							if(Opcode&(1<<9))
							{
								return LEASPTOSP;
							}
							else
							{
								if(Opcode&(1<<8))
								{

								}
								else
								{
									switch(EXTRACT(Opcode,4,7))
									{
										case 0:
											return EXTB;
										case 1:
											return EXTS;
										case 8:
											return JR;
										case 9:
											return CALLR;
										case 10:
											return SET;
										case 11:
											return CLR;
										case 12:
											return SWI;
										case 13:
											return HALT;
									}
								}
							}
						}
					}
					break;
				case 3:
					switch(EXTRACT(Opcode,9,11))
					{
						case 0:
						case 1:
						case 2:
						case 3:
							switch(EXTRACT(Opcode,3,4))
							{
								case 0:
									return ASR;
								case 1:
									return LSR;
								case 2:
									return ASL;
								//case 3:
								//  return LSL;
							}
							break;
						case 4:
							return MULS;
						case 6:
							if(Opcode&(1<<3))
								return MVFC;
							else
								return MVTC;
							break;
					}
					break;
			}
			break;

	}
	return INVALIDOP;
}


static void BuildTable(void)
{
	int i;
	if(!OpTable)
		OpTable=global_alloc_array(_OP, 0x10000);
	for(i=0;i<0x10000;++i)
		OpTable[i]=DecodeOp(i);
}

static CPU_RESET( se3208 )
{
	se3208_state_t *se3208_state = get_safe_token(device);

	device_irq_callback save_irqcallback = se3208_state->irq_callback;
	memset(se3208_state,0,sizeof(se3208_state_t));
	se3208_state->irq_callback = save_irqcallback;
	se3208_state->device = device;
	se3208_state->program = device->space(AS_PROGRAM);
	se3208_state->PC=SE3208_Read32(se3208_state, 0);
	se3208_state->SR=0;
	se3208_state->IRQ=CLEAR_LINE;
	se3208_state->NMI=CLEAR_LINE;
}

static void SE3208_NMI(se3208_state_t *se3208_state)
{
	PushVal(se3208_state,se3208_state->PC);
	PushVal(se3208_state,se3208_state->SR);

	CLRFLAG(FLAG_NMI|FLAG_ENI|FLAG_E|FLAG_M);

	se3208_state->PC=SE3208_Read32(se3208_state, 4);
}

static void SE3208_Interrupt(se3208_state_t *se3208_state)
{
	if(!TESTFLAG(FLAG_ENI))
		return;

	PushVal(se3208_state,se3208_state->PC);
	PushVal(se3208_state,se3208_state->SR);

	CLRFLAG(FLAG_ENI|FLAG_E|FLAG_M);


	if(!(TESTFLAG(FLAG_AUT)))
		se3208_state->PC=SE3208_Read32(se3208_state, 8);
	else
		se3208_state->PC=SE3208_Read32(se3208_state, 4*se3208_state->irq_callback(se3208_state->device, 0));
}


static CPU_EXECUTE( se3208 )
{
	se3208_state_t *se3208_state = get_safe_token(device);

	do
	{
		uint16_t Opcode=memory_decrypted_read_word(se3208_state->program, WORD_XOR_LE(se3208_state->PC));

		debugger_instruction_hook(device, se3208_state->PC);

		OpTable[Opcode](se3208_state, Opcode);
		se3208_state->PPC=se3208_state->PC;
		se3208_state->PC+=2;
		//Check interrupts
		if(se3208_state->NMI==ASSERT_LINE)
		{
			SE3208_NMI(se3208_state);
			se3208_state->NMI=CLEAR_LINE;
		}
		else if(se3208_state->IRQ==ASSERT_LINE && TESTFLAG(FLAG_ENI))
		{
			SE3208_Interrupt(se3208_state);
		}
		--(se3208_state->icount);
	} while(se3208_state->icount>0);
}

static CPU_INIT( se3208 )
{
	se3208_state_t *se3208_state = get_safe_token(device);

	BuildTable();

	se3208_state->irq_callback = irqcallback;
	se3208_state->device = device;
	se3208_state->program = device->space(AS_PROGRAM);
}

static CPU_EXIT( se3208 )
{
	if(OpTable) {
		global_free(OpTable);
		OpTable = NULL;
	}
}


static void set_irq_line(se3208_state_t *se3208_state, int line,int state)
{
	if(line==INPUT_LINE_NMI)	//NMI
		se3208_state->NMI=state;
	else
		se3208_state->IRQ=state;
}


static CPU_SET_INFO( se3208 )
{
	se3208_state_t *se3208_state = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + SE3208_INT:		set_irq_line(se3208_state, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(se3208_state, INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_REGISTER + SE3208_PC:
		case CPUINFO_INT_PC:							se3208_state->PC = info->i;					break;
		case CPUINFO_INT_REGISTER + SE3208_SP:
		case CPUINFO_INT_SP:							se3208_state->SP = info->i; 				break;
		case CPUINFO_INT_REGISTER + SE3208_ER:  		se3208_state->ER = info->i;					break;
		case CPUINFO_INT_REGISTER + SE3208_SR:			se3208_state->SR = info->i;				    break;
		case CPUINFO_INT_REGISTER + SE3208_R0:			se3208_state->R[ 0] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R1:			se3208_state->R[ 1] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R2:			se3208_state->R[ 2] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R3:			se3208_state->R[ 3] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R4:			se3208_state->R[ 4] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R5:			se3208_state->R[ 5] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R6:			se3208_state->R[ 6] = info->i;				break;
		case CPUINFO_INT_REGISTER + SE3208_R7:			se3208_state->R[ 7] = info->i;				break;
	}
}


CPU_GET_INFO( se3208 )
{
	se3208_state_t *se3208_state = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(se3208_state_t);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + SE3208_INT:		info->i = se3208_state->IRQ;					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = se3208_state->NMI;					break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = se3208_state->PPC;					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SE3208_PC:			info->i = se3208_state->PC;					break;
		case CPUINFO_INT_REGISTER + SE3208_SP:
		case CPUINFO_INT_SP:							info->i = se3208_state->SP;					break;
		case CPUINFO_INT_REGISTER + SE3208_SR:			info->i = se3208_state->SR;					break;
		case CPUINFO_INT_REGISTER + SE3208_ER:			info->i = se3208_state->ER;					break;
		case CPUINFO_INT_REGISTER + SE3208_R0:			info->i = se3208_state->R[ 0];				break;
		case CPUINFO_INT_REGISTER + SE3208_R1:			info->i = se3208_state->R[ 1];				break;
		case CPUINFO_INT_REGISTER + SE3208_R2:			info->i = se3208_state->R[ 2];				break;
		case CPUINFO_INT_REGISTER + SE3208_R3:			info->i = se3208_state->R[ 3];				break;
		case CPUINFO_INT_REGISTER + SE3208_R4:			info->i = se3208_state->R[ 4];				break;
		case CPUINFO_INT_REGISTER + SE3208_R5:			info->i = se3208_state->R[ 5];				break;
		case CPUINFO_INT_REGISTER + SE3208_R6:			info->i = se3208_state->R[ 6];				break;
		case CPUINFO_INT_REGISTER + SE3208_R7:			info->i = se3208_state->R[ 7];				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(se3208);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(se3208);		break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(se3208);	break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(se3208);		break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(se3208);break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(se3208);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &se3208_state->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "SE3208");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Advanced Digital Chips Inc."); break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.00");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Miguel Angel Horna, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c %c%c%c%c%c",
					se3208_state->SR&FLAG_C?'C':'.',
					se3208_state->SR&FLAG_V?'V':'.',
					se3208_state->SR&FLAG_S?'S':'.',
					se3208_state->SR&FLAG_Z?'Z':'.',

					se3208_state->SR&FLAG_M?'M':'.',
					se3208_state->SR&FLAG_E?'E':'.',
					se3208_state->SR&FLAG_AUT?'A':'.',
					se3208_state->SR&FLAG_ENI?'I':'.',
					se3208_state->SR&FLAG_NMI?'N':'.'

					);

			break;

		case CPUINFO_STR_REGISTER + SE3208_PC:				sprintf(info->s, "PC  :%08X", se3208_state->PC); break;
		case CPUINFO_STR_REGISTER + SE3208_SR:				sprintf(info->s, "SR  :%08X", se3208_state->SR); break;
		case CPUINFO_STR_REGISTER + SE3208_ER:				sprintf(info->s, "ER  :%08X", se3208_state->ER); break;
		case CPUINFO_STR_REGISTER + SE3208_SP:				sprintf(info->s, "SP  :%08X", se3208_state->SP); break;
		case CPUINFO_STR_REGISTER + SE3208_R0:				sprintf(info->s, "R0  :%08X", se3208_state->R[ 0]); break;
		case CPUINFO_STR_REGISTER + SE3208_R1:				sprintf(info->s, "R1  :%08X", se3208_state->R[ 1]); break;
		case CPUINFO_STR_REGISTER + SE3208_R2:				sprintf(info->s, "R2  :%08X", se3208_state->R[ 2]); break;
		case CPUINFO_STR_REGISTER + SE3208_R3:				sprintf(info->s, "R3  :%08X", se3208_state->R[ 3]); break;
		case CPUINFO_STR_REGISTER + SE3208_R4:				sprintf(info->s, "R4  :%08X", se3208_state->R[ 4]); break;
		case CPUINFO_STR_REGISTER + SE3208_R5:				sprintf(info->s, "R5  :%08X", se3208_state->R[ 5]); break;
		case CPUINFO_STR_REGISTER + SE3208_R6:				sprintf(info->s, "R6  :%08X", se3208_state->R[ 6]); break;
		case CPUINFO_STR_REGISTER + SE3208_R7:				sprintf(info->s, "R7  :%08X", se3208_state->R[ 7]); break;
		case CPUINFO_STR_REGISTER + SE3208_PPC:				sprintf(info->s, "PPC  :%08X", se3208_state->PPC); break;
	}
}


DEFINE_LEGACY_CPU_DEVICE(SE3208, se3208);
