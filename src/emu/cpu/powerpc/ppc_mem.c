#define DUMP_PAGEFAULTS 0

INLINE uint8_t READ8(uint32_t a)
{
	return ppc.read8(ppc.program, a);
}

INLINE uint16_t READ16(uint32_t a)
{
	if( a & 0x1 )
		return ppc.read16_unaligned(ppc.program, a);
	else
		return ppc.read16(ppc.program, a);
}

INLINE uint32_t READ32(uint32_t a)
{
	if( a & 0x3 )
		return ppc.read32_unaligned(ppc.program, a);
	else
		return ppc.read32(ppc.program, a);
}

INLINE uint64_t READ64(uint32_t a)
{
	if( a & 0x7 )
		return ppc.read64_unaligned(ppc.program, a);
	else
		return ppc.read64(ppc.program, a);
}

INLINE void WRITE8(uint32_t a, uint8_t d)
{
	ppc.write8(ppc.program, a, d);
}

INLINE void WRITE16(uint32_t a, uint16_t d)
{
	if( a & 0x1 )
		ppc.write16_unaligned(ppc.program, a, d);
	else
		ppc.write16(ppc.program, a, d);
}

INLINE void WRITE32(uint32_t a, uint32_t d)
{
	if( ppc.reserved ) {
		if( a == ppc.reserved_address ) {
			ppc.reserved = 0;
		}
	}

	if( a & 0x3 )
		ppc.write32_unaligned(ppc.program, a, d);
	else
		ppc.write32(ppc.program, a, d);
}

INLINE void WRITE64(uint32_t a, uint64_t d)
{
	if( a & 0x7 )
		ppc.write64_unaligned(ppc.program, a, d);
	else
		ppc.write64(ppc.program, a, d);
}

/***********************************************************************/

static uint16_t ppc_read16_unaligned(const address_space *space, uint32_t a)
{
	return ((uint16_t)ppc.read8(space, a+0) << 8) | ((uint16_t)ppc.read8(space, a+1) << 0);
}

static uint32_t ppc_read32_unaligned(const address_space *space, uint32_t a)
{
	return ((uint32_t)ppc.read8(space, a+0) << 24) | ((uint32_t)ppc.read8(space, a+1) << 16) |
				   ((uint32_t)ppc.read8(space, a+2) << 8) | ((uint32_t)ppc.read8(space, a+3) << 0);
}

static uint64_t ppc_read64_unaligned(const address_space *space, uint32_t a)
{
	return ((uint64_t)READ32(space, a+0) << 32) | (uint64_t)(READ32(space, a+4));
}

static void ppc_write16_unaligned(const address_space *space, uint32_t a, uint16_t d)
{
	ppc.write8(space, a+0, (uint8_t)(d >> 8));
	ppc.write8(space, a+1, (uint8_t)(d));
}

static void ppc_write32_unaligned(const address_space *space, uint32_t a, uint32_t d)
{
	ppc.write8(space, a+0, (uint8_t)(d >> 24));
	ppc.write8(space, a+1, (uint8_t)(d >> 16));
	ppc.write8(space, a+2, (uint8_t)(d >> 8));
	ppc.write8(space, a+3, (uint8_t)(d >> 0));
}

static void ppc_write64_unaligned(const address_space *space, uint32_t a, uint64_t d)
{
	ppc.write32(space, a+0, (uint32_t)(d >> 32));
	ppc.write32(space, a+4, (uint32_t)(d));
}

/***********************************************************************/

#define DSISR_PAGE		0x40000000
#define DSISR_PROT		0x08000000
#define DSISR_STORE		0x02000000

enum
{
	PPC_TRANSLATE_DATA	= 0x0000,
	PPC_TRANSLATE_CODE	= 0x0001,

	PPC_TRANSLATE_READ	= 0x0000,
	PPC_TRANSLATE_WRITE	= 0x0002,

	PPC_TRANSLATE_NOEXCEPTION = 0x0004
};

#ifdef MESS
#define PPC_MMU_ENABLED		1
#else /* !MESS */
/* MMU not enabled in MAME; model3 drivers don't work properly */
#define PPC_MMU_ENABLED		0
#endif /* MESS */



static int ppc_is_protected(uint32_t pp, int flags)
{
	if (flags & PPC_TRANSLATE_WRITE)
	{
		if ((pp & 0x00000003) != 0x00000002)
			return TRUE;
	}
	else
	{
		if ((pp & 0x00000003) == 0x00000000)
			return TRUE;
	}
	return FALSE;
}

static int ppc_translate_address(offs_t *addr_ptr, int flags)
{
	if (PPC_MMU_ENABLED)
	{
		const BATENT *bat;
		uint32_t address;
		uint32_t sr, vsid, hash;
		uint32_t pteg_address;
		uint32_t target_pte, bl, mask;
		uint64_t pte;
		uint64_t *pteg_ptr[2];
		int i, hash_type;
		uint32_t dsisr = DSISR_PROT;

		bat = (flags & PPC_TRANSLATE_CODE) ? ppc.ibat : ppc.dbat;

		address = *addr_ptr;

		/* first check the block address translation table */
		for (i = 0; i < 4; i++)
		{
			if (bat[i].u & ((MSR & MSR_PR) ? 0x00000001 : 0x00000002))
			{
				bl = bat[i].u & 0x00001FFC;
				mask = (~bl << 15) & 0xFFFE0000;

				if ((address & mask) == (bat[i].u & 0xFFFE0000))
				{
					if (ppc_is_protected(bat[i].l, flags))
						goto exception;

					*addr_ptr = (bat[i].l & 0xFFFE0000)
						| (address & ((bl << 15) | 0x0001FFFF));
					return 1;
				}
			}
		}

		/* now try page address translation */
		sr = ppc.sr[(address >> 28) & 0x0F];
		if (sr & 0x80000000)
		{
			/* direct store translation */
			if ((flags & PPC_TRANSLATE_NOEXCEPTION) == 0)
				fatalerror("ppc: direct store translation not yet implemented");
			return 0;
		}
		else
		{
			/* is no execute is set? */
			if ((flags & PPC_TRANSLATE_CODE) && (sr & 0x10000000))
				goto exception;

			vsid = sr & 0x00FFFFFF;
			hash = (vsid & 0x0007FFFF) ^ ((address >> 12) & 0xFFFF);
			target_pte = (vsid << 7) | ((address >> 22) & 0x3F) | 0x80000000;

			/* we have to try both types of hashes */
			for (hash_type = 0; hash_type <= 1; hash_type++)
			{
				pteg_address = (ppc.sdr1 & 0xFFFF0000)
					| (((ppc.sdr1 & 0x01FF) & (hash >> 10)) << 16)
					| ((hash & 0x03FF) << 6);

				pteg_ptr[hash_type] = memory_get_read_ptr(ppc->program, pteg_address);
				if (pteg_ptr[hash_type])
				{
					for (i = 0; i < 8; i++)
					{
						pte = pteg_ptr[hash_type][i];

						/* is valid? */
						if (((pte >> 32) & 0xFFFFFFFF) == target_pte)
						{
							if (ppc_is_protected((uint32_t) pte, flags))
								goto exception;

							*addr_ptr = ((uint32_t) (pte & 0xFFFFF000))
								| (address & 0x0FFF);
							return 1;
						}
					}
				}

				hash ^= 0x7FFFF;
				target_pte ^= 0x40;
			}

			if (DUMP_PAGEFAULTS)
			{
				mame_printf_debug("PAGE FAULT: address=%08X PC=%08X SDR1=%08X MSR=%08X\n", address, ppc.pc, ppc.sdr1, ppc.msr);
				mame_printf_debug("\n");

				for (i = 0; i < 4; i++)
				{
					bl = bat[i].u & 0x00001FFC;
					mask = (~bl << 15) & 0xFFFE0000;
					mame_printf_debug("    BAT[%d]=%08X%08X    (A & %08X = %08X)\n", i, bat[i].u, bat[i].l,
						mask, bat[i].u & 0xFFFE0000);
				}
				mame_printf_debug("\n");
				mame_printf_debug("    VSID=%06X HASH=%05X HASH\'=%05X\n", vsid, hash, hash ^ 0x7FFFF);

				for (hash_type = 0; hash_type <= 1; hash_type++)
				{
					if (pteg_ptr[hash_type])
					{
						for (i = 0; i < 8; i++)
						{
							pte = pteg_ptr[hash_type][i];
							mame_printf_debug("    PTE[%i%c]=%08X%08X\n",
								i,
								hash_type ? '\'' : ' ',
								(unsigned) (pte >> 32),
								(unsigned) (pte >> 0));
						}
					}
				}
			}
		}

		dsisr = DSISR_PAGE;

exception:
		/* lookup failure - exception */
		if ((flags & PPC_TRANSLATE_NOEXCEPTION) == 0)
		{
			if (flags & PPC_TRANSLATE_CODE)
			{
				ppc_exception(EXCEPTION_ISI);
			}
			else
			{
				ppc.dar = address;
				if (flags & PPC_TRANSLATE_WRITE)
					ppc.dsisr = dsisr | DSISR_STORE;
				else
					ppc.dsisr = dsisr;

				ppc_exception(EXCEPTION_DSI);
			}
		}
		return 0;
	}
	else
	{
		/* MMU not enabled in MAME; model3 drivers don't work properly */
		return 1;
	}
}

static int ppc_translate_address_cb(int space, offs_t *addr)
{
	int success = 1;

	if (space == ADDRESS_SPACE_PROGRAM)
	{
		if (MSR & MSR_DR)
			success = ppc_translate_address(addr, PPC_TRANSLATE_CODE | PPC_TRANSLATE_READ | PPC_TRANSLATE_NOEXCEPTION);
	}
	return success;
}

static uint8_t ppc_read8_translated(const address_space *space, offs_t address)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_READ);
	return memory_read_byte_64be(space, address);
}

static uint16_t ppc_read16_translated(const address_space *space, offs_t address)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_READ);
	return memory_read_word_64be(space, address);
}

static uint32_t ppc_read32_translated(const address_space *space, offs_t address)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_READ);
	return memory_read_dword_64be(space, address);
}

static uint64_t ppc_read64_translated(const address_space *space, offs_t address)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_READ);
	return memory_read_qword_64be(space, address);
}

static void ppc_write8_translated(const address_space *space, offs_t address, uint8_t data)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_WRITE);
	memory_write_byte_64be(space, address, data);
}

static void ppc_write16_translated(const address_space *space, offs_t address, uint16_t data)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_WRITE);
	memory_write_word_64be(space, address, data);
}

static void ppc_write32_translated(const address_space *space, offs_t address, uint32_t data)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_WRITE);
	memory_write_dword_64be(space, address, data);
}

static void ppc_write64_translated(const address_space *space, offs_t address, uint64_t data)
{
	ppc_translate_address(&address, PPC_TRANSLATE_DATA | PPC_TRANSLATE_WRITE);
	memory_write_qword_64be(space, address, data);
}

#ifndef PPC_DRC
static uint32_t ppc_readop_translated(const address_space *space, offs_t address)
{
	ppc_translate_address(&address, PPC_TRANSLATE_CODE | PPC_TRANSLATE_READ);
	return memory_read_dword_64be(space, address);
}
#endif

/***********************************************************************/


static CPU_DISASSEMBLE( ppc )
{
	uint32_t op;
	op = BIG_ENDIANIZE_INT32(*((uint32_t *) oprom));
	return ppc_dasm_one(buffer, pc, op);
}

/***********************************************************************/

static CPU_READOP( ppc )
{
	if (!(ppc.msr & MSR_IR))
		return 0;

	*value = 0;

	if (ppc_translate_address(&offset, PPC_TRANSLATE_CODE | PPC_TRANSLATE_READ | PPC_TRANSLATE_NOEXCEPTION))
	{
		switch(size)
		{
			case 1:	*value = memory_read_byte(ppc.program, offset);	break;
			case 2:	*value = memory_read_word(ppc.program, offset);	break;
			case 4:	*value = memory_read_dword(ppc.program, offset);	break;
			case 8:	*value = memory_read_qword(ppc.program, offset);	break;
		}
	}

	return 1;
}

static CPU_READ( ppc )
{
	if (!(ppc.msr & MSR_DR))
		return 0;

	*value = 0;

	if (ppc_translate_address(&offset, PPC_TRANSLATE_DATA | PPC_TRANSLATE_READ | PPC_TRANSLATE_NOEXCEPTION))
	{
		switch(size)
		{
			case 1:	*value = memory_read_byte(ppc.program, offset);	break;
			case 2:	*value = memory_read_word(ppc.program, offset);	break;
			case 4:	*value = memory_read_dword(ppc.program, offset);	break;
			case 8:	*value = memory_read_qword(ppc.program, offset);	break;
		}
	}

	return 1;
}

static CPU_WRITE( ppc )
{
	if (!(ppc.msr & MSR_DR))
		return 0;

	if (ppc_translate_address(&offset, PPC_TRANSLATE_DATA | PPC_TRANSLATE_WRITE | PPC_TRANSLATE_NOEXCEPTION))
	{
		switch(size)
		{
			case 1:	memory_write_byte(ppc.program, offset, value);	break;
			case 2:	memory_write_word(ppc.program, offset, value);	break;
			case 4:	memory_write_dword(ppc.program, offset, value);	break;
			case 8:	memory_write_qword(ppc.program, offset, value);	break;
		}
	}

	return 1;
}
