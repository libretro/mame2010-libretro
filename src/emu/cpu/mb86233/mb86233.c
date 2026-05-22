/***************************************************************************

    mb86233.c
    Core implementation for the portable Fujitsu MB86233 series DSP emulator.

    Original 2010 driver by ElSemi / MAME version by Ernesto Corvi.
    Instruction decoder rewritten against the much later, much more complete
    understanding of the part's encoding, with the math-unit lookup tables
    (sin/cos/atan/inv/isqrt at offsets 0x20-0x2F) kept inside the CPU as the
    original 2010 driver had them, rather than externalised to the host
    driver.  All four upstream address spaces (program, data, io, rf) are
    bridged through the single program space the 2010 model2 driver
    provides, so the host driver doesn't have to change.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "mb86233.h"

CPU_DISASSEMBLE( mb86233 );

/***************************************************************************
    STATUS REGISTER BITS  (upstream m_st layout)
***************************************************************************/

#define F_ZRC   0x00000001U
#define F_ZRD   0x00000002U
#define F_SGC   0x00000004U
#define F_SGD   0x00000008U
#define F_CPC   0x00000010U
#define F_CPD   0x00000020U
#define F_OVC   0x00000040U
#define F_OVD   0x00000080U
#define F_UNC   0x00000100U
#define F_UND   0x00000200U
#define F_DVZC  0x00000400U
#define F_DVZD  0x00000800U
#define F_CA    0x00001000U
#define F_CPP   0x00002000U
#define F_OVM   0x00004000U
#define F_UNM   0x00008000U
#define F_SIF0  0x00010000U
#define F_SIF1  0x00020000U
#define F_SOF0  0x00040000U
#define F_PIF   0x00100000U
#define F_POF   0x00200000U
#define F_PAIF  0x00400000U
#define F_PAOF  0x00800000U
#define F_F0S   0x01000000U
#define F_F1S   0x02000000U
#define F_IT    0x04000000U
#define F_ZX0   0x08000000U
#define F_ZX1   0x10000000U
#define F_ZX2   0x20000000U
#define F_ZC0   0x40000000U
#define F_ZC1   0x80000000U

/* Old 2010 driver also exported these names through the FLAGS register
 * register-info text.  Keep them defined so the status text is still
 * meaningful even though internally we use the upstream flag set. */
#define ZERO_FLAG       F_ZRD
#define SIGN_FLAG       F_SGD

/***************************************************************************
    STATE
***************************************************************************/

typedef struct _mb86233_state mb86233_state;
struct _mb86233_state
{
    /* program counter and previous PC (for debugger and re-execute on stall) */
    uint16_t pc;
    uint16_t ppc;

    /* status register (32-bit upstream layout) */
    uint32_t st;

    /* main registers */
    uint32_t a, b, d, p;

    /* ALU staging - results computed in alu_pre, committed in alu_post_{1,2} */
    uint32_t alu_stmask;
    uint32_t alu_stset;
    uint32_t alu_r1;
    uint32_t alu_r2;

    /* address-generator registers */
    uint16_t b0, b1;     /* base pair */
    uint16_t x0, x1;     /* index pair */
    uint16_t i0, i1;     /* index increment */
    uint16_t vsmr;       /* derived from vsm */
    uint8_t  sft;        /* shift */
    uint8_t  vsm;        /* virtual stride mode */

    /* repeat machinery */
    uint8_t r;           /* repeat counter (1 = no repeat) */
    uint8_t rpc;         /* repeat-count from register */

    /* loop counters */
    uint8_t c0, c1;

    /* return-address stack */
    uint16_t pcs[4];
    uint16_t sp;

    /* misc */
    uint16_t mask;
    uint16_t m;          /* mode/rounding control (set by stm) */
    int gpio0, gpio1, gpio2, gpio3;

    /* I/O external-memory bank ("EB" in the 2010 driver, "bank" in upstream).
     * Combined with low 16 bits of an offset to form a host address. */
    uint32_t eb;

    /* stall flag - mirrors upstream m_stall semantics; set by FIFO-read
     * helpers when the host has no data ready, checked after each access. */
    int stall;

    /* FIFO callbacks (host-side, set from tgp_config) */
    mb86233_fifo_read_func  fifo_read_cb;
    mb86233_fifo_write_func fifo_write_cb;

    /* internal RAM banks - 256 dwords ARAM + 512 dwords BRAM */
    uint32_t *RAM;
    uint32_t *ARAM;
    uint32_t *BRAM;

    /* math-unit ROM tables (sin/cos/atan/inv/isqrt) */
    uint32_t *Tables;

    /* mirror of external port writes; the math units sample these */
    uint32_t extport[0x30];

    /* CPU plumbing */
    legacy_cpu_device *device;
    const address_space *program;
    /* Cached pointer to the AM_BASE-mapped instruction RAM at program-space
     * offset 0.  When non-NULL, opcode fetches go directly through this
     * array instead of memory_decrypted_read_dword, saving the address-space
     * lookup on every cycle of the hot execute loop.  Set once at CPU_INIT
     * time - the RAM region itself is allocated by the host driver via
     * AM_RAM AM_BASE(&tgp_program) and never moves. */
    uint32_t *program_base;
    /* Cached pointers to the two external regions GETEXTERNAL reaches
     * through cpustate->program for non-math-unit reads: the i960<->TGP
     * shared bufferram (byte 0x00400000+, word 0x00100000+) and the
     * coefficient/math-table ROM (byte 0xff800000+, word 0x3fe00000+).
     * Resolved once at init via memory_get_read_ptr; NULL means the host
     * mapped that region with handlers rather than plain memory, and the
     * fallback RDMEM/WRMEM path takes over. */
    uint32_t *bufferram_base;
    uint32_t  bufferram_word_lo;     /* inclusive word start */
    uint32_t  bufferram_word_hi;     /* exclusive word end */
    uint32_t *rom_base;
    uint32_t  rom_word_lo;
    uint32_t  rom_word_hi;
    int icount;
};

INLINE mb86233_state *get_safe_token(running_device *device)
{
    assert(device != NULL);
    assert(device->type() == MB86233);
    return (mb86233_state *)downcast<legacy_cpu_device *>(device)->token();
}

#define GETPC()         cpustate->pc
#define GETEB()         cpustate->eb
#define GETSR()         cpustate->st
#define GETEXTPORT()    cpustate->extport
#define GETSHIFT()      cpustate->sft

#define ROPCODE(a)      (cpustate->program_base \
                            ? cpustate->program_base[(a) & 0x7fff] \
                            : memory_decrypted_read_dword(cpustate->program, (a) << 2))
#define RDMEM(a)        memory_read_dword_32le(cpustate->program, (a) << 2)
#define WRMEM(a,v)      memory_write_dword_32le(cpustate->program, (a) << 2, v)

/***************************************************************************
    UTILITIES
***************************************************************************/

/* Sign-extend the low <bits> of val into a 32-bit signed result. */
INLINE int32_t sext(uint32_t val, unsigned bits)
{
    uint32_t mask = (bits >= 32) ? 0xFFFFFFFFu : ((1u << bits) - 1u);
    uint32_t v = val & mask;
    if (bits < 32 && (v & (1u << (bits - 1))))
        v |= ~mask;
    return (int32_t)v;
}

/* u2f / f2u are provided by emucore.h. */

INLINE uint32_t set_exp(uint32_t val, uint32_t exp)
{
    return (val & 0x807fffffu) | ((exp & 0xffu) << 23);
}

INLINE uint32_t set_mant(uint32_t val, uint32_t mant)
{
    return (val & 0x07f80000u) | ((mant & 0x00800000u) << 8) | (mant & 0x007fffffu);
}

INLINE uint32_t get_exp(uint32_t val)
{
    return (val >> 23) & 0xffu;
}

INLINE uint32_t get_mant(uint32_t val)
{
    return (val & 0x80000000u) ? (val | 0x7f800000u) : (val & 0x807fffffu);
}

/***************************************************************************
    EXTERNAL MEMORY / MATH UNITS  (preserved from the 2010 driver)
***************************************************************************/

static uint32_t ScaleExp(unsigned int v, int scale)
{
    int exp = (v >> 23) & 0xff;
    exp += scale;
    v &= ~0x7f800000;
    return v | (exp << 23);
}

/* External-port read.  EB | offset forms a host-program-space address,
 * except when EB == 0 and offset is in 0x20-0x2F: those are the math
 * units (sincos / atan / inv / isqrt) and they sample from extport[]
 * combined with the ROM tables. */
static uint32_t GETEXTERNAL(mb86233_state *cpustate, uint32_t EB, uint32_t offset)
{
    uint32_t addr;

    if (EB == 0 && offset >= 0x20 && offset <= 0x2f)
    {
        /* SIN/COS from value at extport[0x20] in 0x4000/PI steps */
        if (offset >= 0x20 && offset <= 0x23)
        {
            uint32_t r;
            uint32_t value = GETEXTPORT()[0x20];
            uint32_t off;
            value += (offset - 0x20) << 14;
            off = value & 0x3fff;
            if ((value & 0x7fff) == 0)
                r = 0;
            else if ((value & 0x7fff) == 0x4000)
                r = 0x3f800000;
            else
            {
                if (value & 0x4000)
                    off = 0x4000 - off;
                r = cpustate->Tables[off];
            }
            if (value & 0x8000)
                r |= 1u << 31;
            return r;
        }

        if (offset == 0x27)
        {
            unsigned int value = GETEXTPORT()[0x27];
            int exp = (value >> 23) & 0xff;
            unsigned int res = 0;
            unsigned int sign = 0;
            uint32_t au = GETEXTPORT()[0x24];
            uint32_t bu = GETEXTPORT()[0x25];
            int index;

            if (!exp)
            {
                if ((au & 0x7fffffff) <= (bu & 0x7fffffff))
                    res = (bu & 0x80000000u) ? 0xc000 : 0x4000;
                else
                    res = (au & 0x80000000u) ? 0x8000 : 0x0000;
                return res;
            }

            if ((au ^ bu) & 0x80000000u)
                sign = 16;

            if ((exp & 0x70) != 0x70)
                index = 0;
            else if (exp < 0x70 || exp > 0x7e)
                index = 0x3fff;
            else
            {
                int expdif = exp - 0x71;
                int base, mask, shift;
                if (expdif < 0) expdif = 0;
                base = 1 << expdif;
                mask = base - 1;
                shift = 23 - expdif;
                index = base + ((value >> shift) & mask);
            }

            res = (cpustate->Tables[index + 0x10000/4] >> sign) & 0xffff;

            if ((au & 0x7fffffff) <= (bu & 0x7fffffff))
                res = 0x4000 - res;

            if ((au & 0x80000000u) && (bu & 0x80000000u))
                res = 0x8000 | res;
            else if ((au & 0x80000000u) && !(bu & 0x80000000u))
                res = res & 0x7fff;
            else if (!(au & 0x80000000u) && (bu & 0x80000000u))
                res = 0x8000 | res;

            return res;
        }

        if (offset == 0x28)
        {
            uint32_t off2   = (GETEXTPORT()[0x28] >> 10) & 0x1fff;
            uint32_t value  = cpustate->Tables[off2 * 2 + 0x20000/4];
            uint32_t srcexp = (GETEXTPORT()[0x28] >> 23) & 0xff;
            value &= 0x7FFFFFFFu;
            return ScaleExp(value, 0x7f - srcexp);
        }
        if (offset == 0x29)
        {
            uint32_t off2   = (GETEXTPORT()[0x28] >> 10) & 0x1fff;
            uint32_t value  = cpustate->Tables[off2 * 2 + (0x20000/4) + 1];
            uint32_t srcexp = (GETEXTPORT()[0x28] >> 23) & 0xff;
            value &= 0x7FFFFFFFu;
            if (GETEXTPORT()[0x28] & (1u << 31))
                value |= 1u << 31;
            return ScaleExp(value, 0x7f - srcexp);
        }
        if (offset == 0x2a)
        {
            uint32_t off2   = ((GETEXTPORT()[0x2a] >> 11) & 0x1fff) ^ 0x1000;
            uint32_t value  = cpustate->Tables[off2 * 2 + 0x30000/4];
            uint32_t srcexp = (GETEXTPORT()[0x2a] >> 24) & 0x7f;
            value &= 0x7FFFFFFFu;
            return ScaleExp(value, 0x3f - srcexp);
        }
        if (offset == 0x2b)
        {
            uint32_t off2   = ((GETEXTPORT()[0x2a] >> 11) & 0x1fff) ^ 0x1000;
            uint32_t value  = cpustate->Tables[off2 * 2 + (0x30000/4) + 1];
            uint32_t srcexp = (GETEXTPORT()[0x2a] >> 24) & 0x7f;
            value &= 0x7FFFFFFFu;
            if (GETEXTPORT()[0x2a] & (1u << 31))
                value |= 1u << 31;
            return ScaleExp(value, 0x3f - srcexp);
        }

        return GETEXTPORT()[offset];
    }

    addr = (EB & 0xFFFF0000u) | (offset & 0xFFFFu);
    /* Direct pointer reads for the two large mapped regions - skips the
     * address-space-table walk that memory_read_dword_32le would do. */
    if (cpustate->bufferram_base &&
        addr >= cpustate->bufferram_word_lo && addr < cpustate->bufferram_word_hi)
        return cpustate->bufferram_base[addr - cpustate->bufferram_word_lo];
    if (cpustate->rom_base &&
        addr >= cpustate->rom_word_lo && addr < cpustate->rom_word_hi)
        return cpustate->rom_base[addr - cpustate->rom_word_lo];
    return RDMEM(addr);
}

static void SETEXTERNAL(mb86233_state *cpustate, uint32_t EB, uint32_t offset, uint32_t value)
{
    uint32_t addr;

    if (EB == 0 && offset >= 0x20 && offset <= 0x2f)
    {
        GETEXTPORT()[offset] = value;

        /* atan compare: store sticky external bit when |a| <= |b| */
        if (offset == 0x25 || offset == 0x24)
        {
            if ((GETEXTPORT()[0x24] & 0x7fffffff) <= (GETEXTPORT()[0x25] & 0x7fffffff))
                GETSR() |= F_CPD;     /* reuse cpd as the "external" sticky bit */
            else
                GETSR() &= ~F_CPD;
        }
        return;
    }

    addr = (EB & 0xFFFF0000u) | (offset & 0xFFFFu);
    /* Direct pointer writes for bufferram (writable shared RAM with the
     * i960).  ROM is not writable, so falls through to WRMEM where it'll
     * just be ignored by the read-only handler.  Skipping the address-
     * space-table walk on the bufferram side alone gets us most of the
     * benefit since the TGP writes geometry results back here every
     * frame. */
    if (cpustate->bufferram_base &&
        addr >= cpustate->bufferram_word_lo && addr < cpustate->bufferram_word_hi)
    {
        cpustate->bufferram_base[addr - cpustate->bufferram_word_lo] = value;
        return;
    }
    WRMEM(addr, value);
}

/***************************************************************************
    STATUS-FLAG UPDATE
***************************************************************************/

INLINE void alu_update_st(mb86233_state *cpustate)
{
    GETSR() = (GETSR() & ~cpustate->alu_stmask) | cpustate->alu_stset;
}

INLINE void stset_set_sz_int(mb86233_state *cpustate, uint32_t val)
{
    cpustate->alu_stset = val ? ((val & 0x80000000u) ? F_SGD : 0) : F_ZRD;
}

INLINE void stset_set_sz_fp(mb86233_state *cpustate, uint32_t val)
{
    cpustate->alu_stset = (val & 0x7fffffffu) ? ((val & 0x80000000u) ? F_SGD : 0) : F_ZRD;
}

/***************************************************************************
    ALU - three-phase
***************************************************************************/

static void alu_pre(mb86233_state *cpustate, uint32_t alu)
{
    switch (alu)
    {
    case 0x00: break; /* no alu */

    case 0x01: /* andd */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = cpustate->d & cpustate->a;
        stset_set_sz_int(cpustate, cpustate->alu_r1);
        break;

    case 0x02: /* orad */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = cpustate->d | cpustate->a;
        stset_set_sz_int(cpustate, cpustate->alu_r1);
        break;

    case 0x03: /* eord */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = cpustate->d ^ cpustate->a;
        stset_set_sz_int(cpustate, cpustate->alu_r1);
        break;

    case 0x04: /* notd */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = ~cpustate->d;
        stset_set_sz_int(cpustate, cpustate->alu_r1);
        break;

    case 0x05: /* fcpd */
    {
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        uint32_t r = f2u(u2f(cpustate->d) - u2f(cpustate->a));
        stset_set_sz_fp(cpustate, r);
        break;
    }

    case 0x06: /* fadd */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = f2u(u2f(cpustate->d) + u2f(cpustate->a));
        stset_set_sz_fp(cpustate, cpustate->alu_r1);
        break;

    case 0x07: /* fsbd */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = f2u(u2f(cpustate->d) - u2f(cpustate->a));
        stset_set_sz_fp(cpustate, cpustate->alu_r1);
        break;

    case 0x08: /* fml -- p = a*b */
        cpustate->alu_stmask = 0;
        cpustate->alu_r1 = f2u(u2f(cpustate->a) * u2f(cpustate->b));
        cpustate->alu_stset = 0;
        break;

    case 0x09: /* fmsd -- d = d + p; p = a*b */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = f2u(u2f(cpustate->d) + u2f(cpustate->p));
        cpustate->alu_r2 = f2u(u2f(cpustate->a) * u2f(cpustate->b));
        stset_set_sz_fp(cpustate, cpustate->alu_r1);
        break;

    case 0x0a: /* fmrd -- d = d - p; p = a*b */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = f2u(u2f(cpustate->d) - u2f(cpustate->p));
        cpustate->alu_r2 = f2u(u2f(cpustate->a) * u2f(cpustate->b));
        stset_set_sz_fp(cpustate, cpustate->alu_r1);
        break;

    case 0x0b: /* fabd */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = cpustate->d & 0x7fffffffu;
        stset_set_sz_fp(cpustate, cpustate->alu_r1);
        break;

    case 0x0c: /* fsmd -- d = d + p (no fresh mul) */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = f2u(u2f(cpustate->d) + u2f(cpustate->p));
        stset_set_sz_fp(cpustate, cpustate->alu_r1);
        break;

    case 0x0d: /* fspd -- d = p; p = a*b */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = cpustate->p;
        cpustate->alu_r2 = f2u(u2f(cpustate->a) * u2f(cpustate->b));
        stset_set_sz_fp(cpustate, cpustate->alu_r1);
        break;

    case 0x0e: /* cxfd -- convert int->float */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = f2u((float)(int32_t)cpustate->d);
        stset_set_sz_int(cpustate, cpustate->alu_r1);
        break;

    case 0x0f: /* cfxd -- convert float->int (rounding via m bits 1-2) */
    {
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        float f = u2f(cpustate->d);
        int32_t r = 0;
        switch ((cpustate->m >> 1) & 3)
        {
        case 0: r = (int32_t)(f >= 0.0f ? (f + 0.5f) : (f - 0.5f)); break; /* round */
        case 1: { /* ceil */
            int32_t i = (int32_t)f;
            if (f > (float)i) i++;
            r = i;
            break;
        }
        case 2: { /* floor */
            int32_t i = (int32_t)f;
            if (f < (float)i) i--;
            r = i;
            break;
        }
        case 3: r = (int32_t)f; break; /* truncate */
        }
        cpustate->alu_r1 = (uint32_t)r;
        stset_set_sz_int(cpustate, cpustate->alu_r1);
        break;
    }

    case 0x10: /* fdvd -- d / a */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        if ((cpustate->a & 0x7fffffffu) != 0)
            cpustate->alu_r1 = f2u(u2f(cpustate->d) / u2f(cpustate->a));
        else
            cpustate->alu_r1 = cpustate->d;
        stset_set_sz_fp(cpustate, cpustate->alu_r1);
        break;

    case 0x11: /* fned -- d = -d */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = cpustate->d ? (cpustate->d ^ 0x80000000u) : 0;
        stset_set_sz_fp(cpustate, cpustate->alu_r1);
        break;

    case 0x13: /* d = b + a */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = f2u(u2f(cpustate->b) + u2f(cpustate->a));
        stset_set_sz_fp(cpustate, cpustate->alu_r1);
        break;

    case 0x14: /* d = b - a */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = f2u(u2f(cpustate->b) - u2f(cpustate->a));
        stset_set_sz_fp(cpustate, cpustate->alu_r1);
        break;

    case 0x16: /* lsrd -- logical shift right */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = cpustate->d >> cpustate->sft;
        stset_set_sz_int(cpustate, cpustate->alu_r1);
        break;

    case 0x17: /* lsld -- logical shift left */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = cpustate->d << cpustate->sft;
        stset_set_sz_int(cpustate, cpustate->alu_r1);
        break;

    case 0x18: /* asrd -- arithmetic shift right */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = (uint32_t)((int32_t)cpustate->d >> cpustate->sft);
        stset_set_sz_int(cpustate, cpustate->alu_r1);
        break;

    case 0x19: /* asld -- arithmetic shift left */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = (uint32_t)((int32_t)cpustate->d << cpustate->sft);
        stset_set_sz_int(cpustate, cpustate->alu_r1);
        break;

    case 0x1a: /* addd -- integer */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = cpustate->d + cpustate->a;
        stset_set_sz_int(cpustate, cpustate->alu_r1);
        break;

    case 0x1b: /* subd -- integer */
        cpustate->alu_stmask = F_ZRD | F_SGD | F_CPD | F_OVD | F_DVZD;
        cpustate->alu_r1 = cpustate->d - cpustate->a;
        stset_set_sz_int(cpustate, cpustate->alu_r1);
        break;

    default:
        logerror("mb86233: unimplemented alu_pre op %02x at PC:%04x\n", alu, cpustate->ppc);
        break;
    }
}

static void alu_post_1(mb86233_state *cpustate, uint32_t alu)
{
    /* integer alu post -- D = r1 */
    switch (alu)
    {
    case 0x01: case 0x02: case 0x03: case 0x04:
    case 0x0e: case 0x0f: case 0x16: case 0x17:
    case 0x18: case 0x19: case 0x1a: case 0x1b:
        cpustate->d = cpustate->alu_r1;
        alu_update_st(cpustate);
        break;
    default:
        break;
    }
}

static void alu_post_2(mb86233_state *cpustate, uint32_t alu)
{
    /* floating-point alu post -- one extra cycle */
    switch (alu)
    {
    case 0x05:
        alu_update_st(cpustate);
        cpustate->icount--;
        break;

    case 0x06: case 0x07: case 0x0b: case 0x0c:
    case 0x10: case 0x11: case 0x13: case 0x14:
        cpustate->d = cpustate->alu_r1;
        alu_update_st(cpustate);
        cpustate->icount--;
        break;

    case 0x08:
        cpustate->p = cpustate->alu_r1;
        cpustate->icount--;
        break;

    case 0x09: case 0x0a: case 0x0d:
        cpustate->d = cpustate->alu_r1;
        cpustate->p = cpustate->alu_r2;
        alu_update_st(cpustate);
        cpustate->icount--;
        break;

    default:
        break;
    }
}

/***************************************************************************
    ADDRESS GENERATOR
***************************************************************************/

static uint16_t ea_pre_0(mb86233_state *cpustate, uint32_t r)
{
    switch (r & 0x180)
    {
    case 0x000:
        return r & 0x7f;
    case 0x080:
    case 0x100:
        return (r & 0x7f) + cpustate->b0 + cpustate->x0;
    case 0x180:
        switch (r & 0x60)
        {
        case 0x00: return cpustate->b0 + cpustate->x0;
        case 0x20: return cpustate->x0;
        case 0x40: return cpustate->b0 + (cpustate->x0 & cpustate->vsmr);
        case 0x60: return cpustate->x0 & cpustate->vsmr;
        }
    }
    return 0;
}

static void ea_post_0(mb86233_state *cpustate, uint32_t r)
{
    if (!(r & 0x100))
        return;
    if (!(r & 0x080))
        cpustate->x0 += cpustate->i0;
    else
        cpustate->x0 += (uint16_t)sext(r, 5);
}

static uint16_t ea_pre_1(mb86233_state *cpustate, uint32_t r)
{
    switch (r & 0x180)
    {
    case 0x000:
        return r & 0x7f;
    case 0x080:
    case 0x100:
        return (r & 0x7f) + cpustate->b1 + cpustate->x1;
    case 0x180:
        switch (r & 0x60)
        {
        case 0x00: return cpustate->b1 + cpustate->x1;
        case 0x20: return cpustate->x1;
        case 0x40: return cpustate->b1 + (cpustate->x1 & cpustate->vsmr);
        case 0x60: return cpustate->x1 & cpustate->vsmr;
        }
    }
    return 0;
}

static void ea_post_1(mb86233_state *cpustate, uint32_t r)
{
    if (!(r & 0x100))
        return;
    if (!(r & 0x080))
        cpustate->x1 += cpustate->i1;
    else
        cpustate->x1 += (uint16_t)sext(r, 5);
}

/***************************************************************************
    PCS STACK
***************************************************************************/

static void pcs_push(mb86233_state *cpustate)
{
    unsigned int i;
    for (i = 3; i; i--)
        cpustate->pcs[i] = cpustate->pcs[i - 1];
    cpustate->pcs[0] = cpustate->pc;
}

static void pcs_pop(mb86233_state *cpustate)
{
    unsigned int i;
    cpustate->pc = cpustate->pcs[0];
    for (i = 0; i != 3; i++)
        cpustate->pcs[i] = cpustate->pcs[i + 1];
}

/***************************************************************************
    DATA / IO / RF READ + WRITE
***************************************************************************/

/* data space - ARAM at 0x000-0x0FF, BRAM at 0x200-0x3FF, all internal */
static uint32_t data_read(mb86233_state *cpustate, uint32_t ea)
{
    ea &= 0xffff;
    if (ea < 0x100)
        return cpustate->ARAM[ea];
    if (ea >= 0x200 && ea < 0x400)
        return cpustate->BRAM[ea - 0x200];
    /* unmapped - upstream returns 0 from unmapped reads */
    return 0;
}

static void data_write(mb86233_state *cpustate, uint32_t ea, uint32_t v)
{
    ea &= 0xffff;
    if (ea < 0x100)
    {
        cpustate->ARAM[ea] = v;
        return;
    }
    if (ea >= 0x200 && ea < 0x400)
    {
        cpustate->BRAM[ea - 0x200] = v;
        return;
    }
    /* unmapped */
}

/* I/O space - bridged through GETEXTERNAL/SETEXTERNAL using EB as bank */
static uint32_t io_read(mb86233_state *cpustate, uint32_t ea)
{
    return GETEXTERNAL(cpustate, GETEB(), ea & 0xffff);
}

static void io_write(mb86233_state *cpustate, uint32_t ea, uint32_t v)
{
    SETEXTERNAL(cpustate, GETEB(), ea & 0xffff, v);
}

/* "program-as-data" reads - used for the mov mem (o), * variants in upstream.
 * Upstream pulls from m_program; we bridge the same way. */
static uint32_t prog_read_as_data(mb86233_state *cpustate, uint32_t ea)
{
    return ROPCODE(ea & 0xffff);
}

/***************************************************************************
    REGISTER FILE
***************************************************************************/

static uint32_t read_reg(mb86233_state *cpustate, uint32_t r)
{
    r &= 0x3f;

    if (r >= 0x20 && r < 0x30)
    {
        /* RF[0..F].  Only RF[1] (FIFO IN) and RF[3] (EB read-back) are
         * meaningful for model2; the rest log if hit. */
        uint32_t rf = r & 0x0f;
        switch (rf)
        {
        case 0x0: return 0;                      /* leds / status */
        case 0x1:                                /* FIFO in */
        {
            uint32_t v = 0;
            if (cpustate->fifo_read_cb &&
                cpustate->fifo_read_cb(cpustate->device, &v))
                return v;
            /* no data ready - request stall */
            cpustate->stall = 1;
            return 0;
        }
        case 0x2: return 0;                      /* FIFO out is write-only */
        case 0x3: return cpustate->eb;           /* bank reg */
        default:
            logerror("mb86233: read of unimplemented RF[%x] at PC:%04x\n", rf, cpustate->ppc);
            return 0;
        }
    }

    switch (r)
    {
    case 0x00: return cpustate->b0;
    case 0x01: return cpustate->b1;
    case 0x02: return cpustate->x0;
    case 0x03: return cpustate->x1;

    case 0x05: return cpustate->i0;
    case 0x06: return cpustate->i1;

    case 0x08: return cpustate->sp;

    case 0x0a: return cpustate->vsm;

    case 0x0c: return cpustate->c0;
    case 0x0d: return cpustate->c1;

    case 0x0f: return cpustate->pc;

    case 0x10: return cpustate->a;
    case 0x11: return get_exp(cpustate->a);
    case 0x12: return get_mant(cpustate->a);
    case 0x13: return cpustate->b;
    case 0x14: return get_exp(cpustate->b);
    case 0x15: return get_mant(cpustate->b);
    case 0x19: return cpustate->d;
    case 0x1a: return get_exp(cpustate->d);
    case 0x1b: return get_mant(cpustate->d);
    case 0x1c: return cpustate->p;
    case 0x1d: return get_exp(cpustate->p);
    case 0x1e: return get_mant(cpustate->p);
    case 0x1f: return cpustate->sft;

    case 0x34: return cpustate->rpc;
    case 0x3c: return cpustate->mask;

    default:
        logerror("mb86233: unimplemented read_reg(%02x) at PC:%04x\n", r, cpustate->ppc);
        return 0;
    }
}

static void write_reg(mb86233_state *cpustate, uint32_t r, uint32_t v)
{
    r &= 0x3f;

    if (r >= 0x20 && r < 0x30)
    {
        uint32_t rf = r & 0x0f;
        switch (rf)
        {
        case 0x0: return;                        /* leds / nop */
        case 0x1: return;                        /* FIFO in is read-only */
        case 0x2:                                /* FIFO out */
            if (cpustate->fifo_write_cb)
                cpustate->fifo_write_cb(cpustate->device, v);
            return;
        case 0x3:                                /* bank reg = EB */
            /* Match the 2010 driver convention: EB stores the bank
             * pre-shifted into the high 16 bits, OR'd with the per-access
             * low 16 bits of offset to form a host-program-space address. */
            cpustate->eb = v;
            return;
        default:
            logerror("mb86233: write to unimplemented RF[%x] = %08x at PC:%04x\n", rf, v, cpustate->ppc);
            return;
        }
    }

    switch (r)
    {
    case 0x00: cpustate->b0 = v; break;
    case 0x01: cpustate->b1 = v; break;
    case 0x02: cpustate->x0 = v; break;
    case 0x03: cpustate->x1 = v; break;

    case 0x05: cpustate->i0 = v; break;
    case 0x06: cpustate->i1 = v; break;

    case 0x08: cpustate->sp = v; break;

    case 0x0a:
        cpustate->vsm = v & 7;
        cpustate->vsmr = (uint16_t)((8u << cpustate->vsm) - 1u);
        break;

    case 0x0c:
        cpustate->c0 = v;
        if (cpustate->c0 == 1) cpustate->st |= F_ZC0;
        else                   cpustate->st &= ~F_ZC0;
        break;

    case 0x0d:
        cpustate->c1 = v;
        if (cpustate->c1 == 1) cpustate->st |= F_ZC1;
        else                   cpustate->st &= ~F_ZC1;
        break;

    case 0x0f: break;     /* PC - upstream ignores writes here */

    case 0x10: cpustate->a = v;                       break;
    case 0x11: cpustate->a = set_exp(cpustate->a, v); break;
    case 0x12: cpustate->a = set_mant(cpustate->a, v); break;
    case 0x13: cpustate->b = v;                       break;
    case 0x14: cpustate->b = set_exp(cpustate->b, v); break;
    case 0x15: cpustate->b = set_mant(cpustate->b, v); break;
    case 0x19: cpustate->d = v;                       break;
    case 0x1a: cpustate->d = set_exp(cpustate->d, v); break;
    case 0x1b: cpustate->d = set_mant(cpustate->d, v); break;
    case 0x1c: cpustate->p = v;                       break;
    case 0x1d: cpustate->p = set_exp(cpustate->p, v); break;
    case 0x1e: cpustate->p = set_mant(cpustate->p, v); break;
    case 0x1f: cpustate->sft = v;                     break;

    case 0x34: cpustate->rpc = v; break;
    case 0x3c: cpustate->mask = v; break;

    default:
        logerror("mb86233: unimplemented write_reg(%02x, %08x) at PC:%04x\n", r, v, cpustate->ppc);
        break;
    }
}

/* mem-write helpers: write to data space at (ea_pre_1 + optional bank offset) */
static void write_mem_internal_1(mb86233_state *cpustate, uint32_t r, uint32_t v, int bank)
{
    uint16_t ea = ea_pre_1(cpustate, r);
    if (bank)
        ea += 0x200;
    data_write(cpustate, ea, v);
    ea_post_1(cpustate, r);
}

static void write_mem_io_1(mb86233_state *cpustate, uint32_t r, uint32_t v)
{
    uint16_t ea = ea_pre_1(cpustate, r);
    io_write(cpustate, ea, v);
    ea_post_1(cpustate, r);
}

/***************************************************************************
    INIT / RESET
***************************************************************************/

static CPU_INIT( mb86233 )
{
    mb86233_state *cpustate = get_safe_token(device);
    mb86233_cpu_core *_config = (mb86233_cpu_core *)device->baseconfig().static_config();
    (void)irqcallback;

    memset(cpustate, 0, sizeof(*cpustate));
    cpustate->device  = device;
    cpustate->program = device->space(AS_PROGRAM);

    /* Cache a direct pointer to the instruction RAM at program-space
     * offset 0 if the host driver mapped it as plain RAM via AM_BASE -
     * the model2 driver does, so we skip memory_decrypted_read_dword and
     * its address-space lookup on every TGP cycle.  If the lookup fails
     * we leave program_base NULL and ROPCODE falls back to the slow
     * path automatically. */
    cpustate->program_base = (uint32_t *)memory_get_read_ptr(cpustate->program, 0);

    /* Same trick for the two external regions GETEXTERNAL hits constantly
     * during gameplay: i960<->TGP bufferram and the math-table ROM.  The
     * model2 driver maps bufferram at byte 0x00400000 (word 0x00100000)
     * for 8K dwords, and the math-table ROM at byte 0xff800000 (word
     * 0x3fe00000).  If either lookup fails we leave the cache pointer
     * NULL and the GETEXTERNAL fallback reads via memory_read_dword_32le
     * as before. */
    cpustate->bufferram_base = (uint32_t *)memory_get_read_ptr(cpustate->program, 0x00400000);
    cpustate->bufferram_word_lo = 0x00100000;
    cpustate->bufferram_word_hi = 0x00102000;
    cpustate->rom_base = (uint32_t *)memory_get_read_ptr(cpustate->program, 0xff800000);
    cpustate->rom_word_lo = 0x3fe00000;
    cpustate->rom_word_hi = 0x3fe80000;

    if (_config)
    {
        cpustate->fifo_read_cb  = _config->fifo_read_cb;
        cpustate->fifo_write_cb = _config->fifo_write_cb;
    }

    /* 2x 256-word internal RAMs */
    cpustate->RAM = auto_alloc_array(device->machine, uint32_t, 2 * 0x200);
    memset(cpustate->RAM, 0, 2 * 0x200 * sizeof(uint32_t));
    cpustate->ARAM = &cpustate->RAM[0];
    cpustate->BRAM = &cpustate->RAM[0x200];

    if (_config && _config->tablergn)
        cpustate->Tables = (uint32_t *)memory_region(device->machine, _config->tablergn);

    state_save_register_global_pointer(device->machine, cpustate->RAM, 2 * 0x200);
    state_save_register_device_item(device, 0, cpustate->pc);
    state_save_register_device_item(device, 0, cpustate->ppc);
    state_save_register_device_item(device, 0, cpustate->st);
    state_save_register_device_item(device, 0, cpustate->a);
    state_save_register_device_item(device, 0, cpustate->b);
    state_save_register_device_item(device, 0, cpustate->d);
    state_save_register_device_item(device, 0, cpustate->p);
    state_save_register_device_item(device, 0, cpustate->eb);
    state_save_register_device_item(device, 0, cpustate->r);
    state_save_register_device_item(device, 0, cpustate->rpc);
    state_save_register_device_item(device, 0, cpustate->c0);
    state_save_register_device_item(device, 0, cpustate->c1);
    state_save_register_device_item(device, 0, cpustate->b0);
    state_save_register_device_item(device, 0, cpustate->b1);
    state_save_register_device_item(device, 0, cpustate->x0);
    state_save_register_device_item(device, 0, cpustate->x1);
    state_save_register_device_item(device, 0, cpustate->i0);
    state_save_register_device_item(device, 0, cpustate->i1);
    state_save_register_device_item(device, 0, cpustate->vsm);
    state_save_register_device_item(device, 0, cpustate->vsmr);
    state_save_register_device_item(device, 0, cpustate->sft);
    state_save_register_device_item(device, 0, cpustate->mask);
    state_save_register_device_item(device, 0, cpustate->m);
    state_save_register_device_item(device, 0, cpustate->sp);
    state_save_register_device_item_array(device, 0, cpustate->pcs);
    state_save_register_device_item_array(device, 0, cpustate->extport);
}

static CPU_RESET( mb86233 )
{
    mb86233_state *cpustate = get_safe_token(device);
    int i;

    cpustate->pc  = 0;
    cpustate->ppc = 0;
    cpustate->st  = F_ZRC | F_ZRD | F_ZX0 | F_ZX1 | F_ZX2 | F_ZC0 | F_ZC1;
    cpustate->sp  = 0;

    cpustate->a = cpustate->b = cpustate->d = cpustate->p = 0;
    cpustate->r = 1;
    cpustate->rpc = 1;
    cpustate->c0 = 1;
    cpustate->c1 = 1;
    cpustate->b0 = cpustate->b1 = cpustate->x0 = cpustate->x1 = 0;
    cpustate->i0 = cpustate->i1 = 0;
    cpustate->sft = 0;
    cpustate->vsm = 0;
    cpustate->vsmr = 7;
    cpustate->mask = 0;
    cpustate->m = 1;
    cpustate->eb = 0;

    cpustate->alu_stmask = 0;
    cpustate->alu_stset  = 0;
    cpustate->alu_r1 = 0;
    cpustate->alu_r2 = 0;

    cpustate->stall = 0;

    for (i = 0; i < 4; i++)
        cpustate->pcs[i] = 0;
}

/***************************************************************************
    EXECUTE
***************************************************************************/

static CPU_EXECUTE( mb86233 )
{
    mb86233_state *cpustate = get_safe_token(device);

    while (cpustate->icount > 0)
    {
        uint32_t opcode;
        int do_stall = 0;

        cpustate->ppc = cpustate->pc;
        debugger_instruction_hook(device, cpustate->ppc);

        opcode = ROPCODE(cpustate->pc);
        cpustate->pc++;

        switch ((opcode >> 26) & 0x3f)
        {
        case 0x00:
        {
            /* lab - dual load */
            uint32_t r1  = opcode & 0x1ff;
            uint32_t r2  = (opcode >> 9) & 0x1ff;
            uint32_t alu = (opcode >> 21) & 0x1f;
            uint32_t op  = (opcode >> 18) & 0x7;

            alu_pre(cpustate, alu);

            switch (op)
            {
            case 0: case 1:
            {
                uint16_t ea1 = ea_pre_0(cpustate, r1);
                uint32_t v1  = data_read(cpustate, ea1);
                uint16_t ea2; uint32_t v2;
                if (cpustate->stall) { do_stall = 1; break; }
                ea2 = ea_pre_1(cpustate, r2);
                v2  = io_read(cpustate, ea2);
                if (cpustate->stall) { do_stall = 1; break; }
                ea_post_0(cpustate, r1);
                ea_post_1(cpustate, r2);
                cpustate->a = v1;
                cpustate->b = v2;
                break;
            }

            case 3:
            {
                uint16_t ea1 = ea_pre_0(cpustate, r1);
                uint32_t v1  = data_read(cpustate, ea1);
                uint16_t ea2; uint32_t v2;
                if (cpustate->stall) { do_stall = 1; break; }
                ea2 = ea_pre_1(cpustate, r2) + 0x200;
                v2  = data_read(cpustate, ea2);
                if (cpustate->stall) { do_stall = 1; break; }
                ea_post_0(cpustate, r1);
                ea_post_1(cpustate, r2);
                cpustate->a = v1;
                cpustate->b = v2;
                break;
            }

            case 4:
            {
                uint16_t ea1 = ea_pre_0(cpustate, r1) + 0x200;
                uint32_t v1  = data_read(cpustate, ea1);
                uint16_t ea2; uint32_t v2;
                if (cpustate->stall) { do_stall = 1; break; }
                ea2 = ea_pre_1(cpustate, r2);
                v2  = data_read(cpustate, ea2);
                if (cpustate->stall) { do_stall = 1; break; }
                ea_post_0(cpustate, r1);
                ea_post_1(cpustate, r2);
                cpustate->a = v1;
                cpustate->b = v2;
                break;
            }

            default:
                logerror("mb86233: unhandled lab subop %x at PC:%04x\n", op, cpustate->ppc);
                break;
            }

            if (do_stall) break;
            alu_post_1(cpustate, alu);
            alu_post_2(cpustate, alu);
            break;
        }

        case 0x07:
        {
            /* ld / mov */
            uint32_t r1  = opcode & 0x1ff;
            uint32_t r2  = (opcode >> 9) & 0x1ff;
            uint32_t alu = (opcode >> 21) & 0x1f;
            uint32_t op  = (opcode >> 18) & 0x7;

            alu_pre(cpustate, alu);

            switch (op)
            {
            case 0:
            case 1:
            {
                /* mov mem, mem (e) */
                uint16_t ea = ea_pre_0(cpustate, r1);
                uint32_t v  = data_read(cpustate, ea);
                if (cpustate->stall) { do_stall = 1; break; }
                ea_post_0(cpustate, r1);
                alu_post_1(cpustate, alu);
                write_mem_io_1(cpustate, r2, v);
                break;
            }

            case 2:
            {
                /* mov mem (e), mem */
                uint16_t ea = ea_pre_0(cpustate, r1);
                uint32_t v  = io_read(cpustate, ea);
                if (cpustate->stall) { do_stall = 1; break; }
                ea_post_0(cpustate, r1);
                alu_post_1(cpustate, alu);
                write_mem_internal_1(cpustate, r2, v, 0);
                break;
            }

            case 3:
            {
                /* mov mem, mem + 0x200 */
                uint16_t ea = ea_pre_0(cpustate, r1);
                uint32_t v  = data_read(cpustate, ea);
                if (cpustate->stall) { do_stall = 1; break; }
                ea_post_0(cpustate, r1);
                alu_post_1(cpustate, alu);
                write_mem_internal_1(cpustate, r2, v, 1);
                break;
            }

            case 4:
            {
                /* mov mem + 0x200, mem */
                uint16_t ea = ea_pre_0(cpustate, r1) + 0x200;
                uint32_t v  = data_read(cpustate, ea);
                if (cpustate->stall) { do_stall = 1; break; }
                ea_post_0(cpustate, r1);
                alu_post_1(cpustate, alu);
                write_mem_internal_1(cpustate, r2, v, 0);
                break;
            }

            case 5:
            {
                /* mov mem (o), mem - source is program-as-data */
                uint16_t ea = ea_pre_0(cpustate, r1);
                uint32_t v  = prog_read_as_data(cpustate, ea);
                if (cpustate->stall) { do_stall = 1; break; }
                ea_post_0(cpustate, r1);
                alu_post_1(cpustate, alu);
                write_mem_internal_1(cpustate, r2, v, 0);
                break;
            }

            case 7:
            {
                switch (r2 >> 6)
                {
                case 0:
                {
                    /* mov reg, mem */
                    uint32_t v = read_reg(cpustate, r2);
                    if (cpustate->stall) { do_stall = 1; break; }
                    alu_post_1(cpustate, alu);
                    write_mem_internal_1(cpustate, r1, v, 0);
                    break;
                }

                case 1:
                {
                    /* mov reg, mem (e) */
                    uint32_t v = read_reg(cpustate, r2);
                    if (cpustate->stall) { do_stall = 1; break; }
                    alu_post_1(cpustate, alu);
                    write_mem_io_1(cpustate, r1, v);
                    break;
                }

                case 2:
                {
                    /* mov mem + 0x200, reg */
                    uint16_t ea = ea_pre_1(cpustate, r1) + 0x200;
                    uint32_t v  = data_read(cpustate, ea);
                    if (cpustate->stall) { do_stall = 1; break; }
                    ea_post_1(cpustate, r1);
                    alu_post_1(cpustate, alu);
                    write_reg(cpustate, r2, v);
                    break;
                }

                case 3:
                {
                    /* mov mem, reg */
                    uint16_t ea = ea_pre_1(cpustate, r1);
                    uint32_t v  = data_read(cpustate, ea);
                    if (cpustate->stall) { do_stall = 1; break; }
                    ea_post_1(cpustate, r1);
                    alu_post_1(cpustate, alu);
                    write_reg(cpustate, r2, v);
                    break;
                }

                case 4:
                {
                    /* mov mem (e), reg */
                    uint16_t ea = ea_pre_1(cpustate, r1);
                    uint32_t v  = io_read(cpustate, ea);
                    if (cpustate->stall) { do_stall = 1; break; }
                    ea_post_1(cpustate, r1);
                    alu_post_1(cpustate, alu);
                    write_reg(cpustate, r2, v);
                    break;
                }

                case 5:
                {
                    /* mov mem (o), reg */
                    uint16_t ea = ea_pre_0(cpustate, r1);
                    uint32_t v  = prog_read_as_data(cpustate, ea);
                    if (cpustate->stall) { do_stall = 1; break; }
                    ea_post_0(cpustate, r1);
                    alu_post_1(cpustate, alu);
                    write_reg(cpustate, r2, v);
                    break;
                }

                case 6:
                {
                    /* mov reg, reg */
                    uint32_t v = read_reg(cpustate, r1);
                    if (cpustate->stall) { do_stall = 1; break; }
                    alu_post_1(cpustate, alu);
                    write_reg(cpustate, r2, v);
                    break;
                }

                default:
                    alu_post_1(cpustate, alu);
                    logerror("mb86233: unhandled ld/mov subop 7/%x at PC:%04x\n",
                             (r2 >> 6), cpustate->ppc);
                    break;
                }
                break;
            }

            default:
                alu_post_1(cpustate, alu);
                logerror("mb86233: unhandled ld/mov subop %x at PC:%04x\n", op, cpustate->ppc);
                break;
            }

            if (do_stall) break;
            alu_post_2(cpustate, alu);
            break;
        }

        case 0x0d:
        {
            /* stm / clm */
            uint32_t sub2 = (opcode >> 17) & 7;
            switch (sub2)
            {
            case 5:
                /* stmh - mode register update */
                cpustate->m = opcode;
                break;
            default:
                logerror("mb86233: unimplemented opcode 0d/%x at PC:%04x\n", sub2, cpustate->ppc);
                break;
            }
            break;
        }

        case 0x0e:
        {
            /* lipl / lia / lib / lid */
            switch ((opcode >> 24) & 0x3)
            {
            case 0:
                /* lipl - low 24 bits of P, zero-extended */
                cpustate->p = (cpustate->p & 0xff000000u) | (opcode & 0x00ffffffu);
                break;
            case 1:
                cpustate->a = (uint32_t)sext(opcode, 24);
                break;
            case 2:
                cpustate->b = (uint32_t)sext(opcode, 24);
                break;
            case 3:
                cpustate->d = (uint32_t)sext(opcode, 24);
                break;
            }
            break;
        }

        case 0x0f:
        {
            /* rep / clr0 / clr1 / set */
            uint32_t alu  = (opcode >> 20) & 0x1f;
            uint32_t sub2 = (opcode >> 17) & 7;

            alu_pre(cpustate, alu);

            switch (sub2)
            {
            case 0:
                /* clr0 */
                if (opcode & 0x0004) cpustate->a = 0;
                if (opcode & 0x0008) cpustate->b = 0;
                if (opcode & 0x0010) cpustate->d = 0;
                break;
            case 1:
                /* clr1 - flag mapping not fully known */
                break;
            case 2:
            {
                /* rep */
                uint8_t r = (opcode & 0x8000) ? (uint8_t)read_reg(cpustate, opcode) : (uint8_t)opcode;
                if (cpustate->stall) { do_stall = 1; break; }
                cpustate->r = r;
                /* skip the trailing repeat-decrement logic for this instruction
                 * - we'll continue with r != 1 and the outer loop will rewind */
                alu_post_1(cpustate, alu);
                /* do NOT touch m_pc / m_r decrement */
                cpustate->icount--;
                continue;
            }
            case 3:
                /* set - flag mapping not fully known */
                break;
            default:
                logerror("mb86233: unimplemented opcode 0f/%x at PC:%04x\n", sub2, cpustate->ppc);
                break;
            }
            if (do_stall) break;
            alu_post_1(cpustate, alu);
            break;
        }

        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x18: case 0x19: case 0x1a: case 0x1b:
        case 0x1c: case 0x1d: case 0x1e: case 0x1f:
        {
            /* ldi #v, r */
            write_reg(cpustate, opcode >> 24, (uint32_t)sext(opcode, 24));
            break;
        }

        case 0x2f:
        case 0x3f:
        {
            /* conditional branch family */
            uint32_t cond    = (opcode >> 20) & 0x1f;
            uint32_t subtype = (opcode >> 17) & 7;
            uint32_t data    = opcode & 0xffff;
            int invert       = (opcode & 0x40000000) ? 1 : 0;
            int cond_passed  = 0;

            switch (cond)
            {
            case 0x00: cond_passed = (cpustate->st & F_ZRD) ? 1 : 0; break;
            case 0x01: cond_passed = !(cpustate->st & F_SGD) ? 1 : 0; break;
            case 0x02: cond_passed = (cpustate->st & (F_ZRD | F_SGD)) ? 1 : 0; break;
            case 0x0a: cond_passed = cpustate->gpio0; break;
            case 0x0b: cond_passed = cpustate->gpio1; break;
            case 0x0c: cond_passed = cpustate->gpio2; break;
            case 0x10: cond_passed = !(cpustate->st & F_ZC0) ? 1 : 0; break;
            case 0x11: cond_passed = !(cpustate->st & F_ZC1) ? 1 : 0; break;
            case 0x12: cond_passed = cpustate->gpio3; break;
            case 0x16: cond_passed = 1; break;
            default:
                logerror("mb86233: unimplemented branch cond %x at PC:%04x\n", cond, cpustate->ppc);
                break;
            }
            if (invert)
                cond_passed = !cond_passed;

            if (cond_passed)
            {
                switch (subtype)
                {
                case 0:
                    /* brif #adr */
                    cpustate->pc = data;
                    break;

                case 1:
                    /* brul */
                    if (opcode & 0x4000)
                    {
                        uint32_t v = read_reg(cpustate, opcode);
                        if (cpustate->stall) { do_stall = 1; break; }
                        cpustate->pc = v;
                    }
                    else
                    {
                        uint16_t ea = ea_pre_0(cpustate, opcode);
                        uint32_t v  = data_read(cpustate, ea);
                        if (cpustate->stall) { do_stall = 1; break; }
                        ea_post_0(cpustate, opcode);
                        cpustate->pc = v;
                    }
                    break;

                case 2:
                    /* bsif #adr */
                    pcs_push(cpustate);
                    cpustate->pc = data;
                    break;

                case 3:
                    /* bsul */
                    if (opcode & 0x4000)
                    {
                        uint32_t v = read_reg(cpustate, opcode);
                        if (cpustate->stall) { do_stall = 1; break; }
                        pcs_push(cpustate);
                        cpustate->pc = v;
                    }
                    else
                    {
                        uint16_t ea = ea_pre_0(cpustate, opcode);
                        uint32_t v  = data_read(cpustate, ea);
                        if (cpustate->stall) { do_stall = 1; break; }
                        ea_post_0(cpustate, opcode);
                        pcs_push(cpustate);
                        cpustate->pc = v;
                    }
                    break;

                case 5:
                    /* rtif */
                    pcs_pop(cpustate);
                    break;

                case 6:
                {
                    /* ldif adr, rn */
                    uint16_t ea = ea_pre_0(cpustate, opcode);
                    uint32_t v  = data_read(cpustate, ea);
                    if (cpustate->stall) { do_stall = 1; break; }
                    ea_post_0(cpustate, opcode);
                    write_reg(cpustate, opcode >> 9, v);
                    break;
                }

                default:
                    logerror("mb86233: unimplemented branch subtype %x at PC:%04x\n",
                             subtype, cpustate->ppc);
                    break;
                }
            }

            if (subtype < 2)
            {
                /* auto-decrement c0/c1 for non-subroutine forms */
                switch (cond)
                {
                case 0x10:
                    if (cpustate->c0 != 1)
                    {
                        cpustate->c0--;
                        if (cpustate->c0 == 1)
                            cpustate->st |= F_ZC0;
                    }
                    break;
                case 0x11:
                    if (cpustate->c1 != 1)
                    {
                        cpustate->c1--;
                        if (cpustate->c1 == 1)
                            cpustate->st |= F_ZC1;
                    }
                    break;
                }
            }

            break;
        }

        default:
            logerror("mb86233: unimplemented opcode type %02x (%08x) at PC:%04x\n",
                     (opcode >> 26) & 0x3f, opcode, cpustate->ppc);
            break;
        }

        if (cpustate->r != 1)
        {
            cpustate->pc = cpustate->ppc;
            cpustate->r--;
        }

        if (do_stall)
        {
            /* re-execute on next slice */
            cpustate->pc = cpustate->ppc;
            cpustate->stall = 0;
        }

        cpustate->icount--;
    }
}

/***************************************************************************
    INFO INTERFACE
***************************************************************************/

static CPU_SET_INFO( mb86233 )
{
    mb86233_state *cpustate = get_safe_token(device);

    switch (state)
    {
    case CPUINFO_INT_PC:
    case CPUINFO_INT_REGISTER + MB86233_PC:    cpustate->pc    = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_A:     cpustate->a     = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_B:     cpustate->b     = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_P:     cpustate->p     = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_D:     cpustate->d     = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_REP:   cpustate->r     = info->i; break;
    case CPUINFO_INT_SP:
    case CPUINFO_INT_REGISTER + MB86233_SP:    cpustate->sp    = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_EB:    cpustate->eb    = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_SHIFT: cpustate->sft   = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_FLAGS: cpustate->st    = info->i; break;
    /* MB86233_R0..R15: mapped to upstream-style registers for compatibility
     * with debugger expectations.  The 2010 driver's gpr[] array is gone. */
    case CPUINFO_INT_REGISTER + MB86233_R0:    cpustate->b0    = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R1:    cpustate->b1    = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R2:    cpustate->x0    = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R3:    cpustate->x1    = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R4:    cpustate->i0    = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R5:    cpustate->i1    = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R6:    cpustate->c0    = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R7:    cpustate->c1    = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R8:    cpustate->pcs[0]= info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R9:    cpustate->pcs[1]= info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R10:   cpustate->pcs[2]= info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R11:   cpustate->pcs[3]= info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R12:   cpustate->vsm   = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R13:   cpustate->mask  = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R14:   cpustate->m     = info->i; break;
    case CPUINFO_INT_REGISTER + MB86233_R15:   cpustate->rpc   = info->i; break;
    }
}

CPU_GET_INFO( mb86233 )
{
    mb86233_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

    switch (state)
    {
    case CPUINFO_INT_CONTEXT_SIZE:                                 info->i = sizeof(mb86233_state); break;
    case CPUINFO_INT_INPUT_LINES:                                  info->i = 0; break;
    case CPUINFO_INT_DEFAULT_IRQ_VECTOR:                           info->i = 0; break;
    case DEVINFO_INT_ENDIANNESS:                                   info->i = ENDIANNESS_LITTLE; break;
    case CPUINFO_INT_CLOCK_MULTIPLIER:                             info->i = 1; break;
    case CPUINFO_INT_CLOCK_DIVIDER:                                info->i = 1; break;
    case CPUINFO_INT_MIN_INSTRUCTION_BYTES:                        info->i = 4; break;
    case CPUINFO_INT_MAX_INSTRUCTION_BYTES:                        info->i = 4; break;
    case CPUINFO_INT_MIN_CYCLES:                                   info->i = 1; break;
    case CPUINFO_INT_MAX_CYCLES:                                   info->i = 2; break;

    case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:        info->i = 32; break;
    case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM:        info->i = 32; break;
    case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM:        info->i = -2; break;
    case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:           info->i = 0;  break;
    case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:           info->i = 0;  break;
    case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:           info->i = 0;  break;
    case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:             info->i = 0;  break;
    case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:             info->i = 0;  break;
    case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:             info->i = 0;  break;

    case CPUINFO_INT_PREVIOUSPC:                                                  break;

    case CPUINFO_INT_PC:
    case CPUINFO_INT_REGISTER + MB86233_PC:    info->i = cpustate->pc;    break;
    case CPUINFO_INT_REGISTER + MB86233_A:     info->i = cpustate->a;     break;
    case CPUINFO_INT_REGISTER + MB86233_B:     info->i = cpustate->b;     break;
    case CPUINFO_INT_REGISTER + MB86233_P:     info->i = cpustate->p;     break;
    case CPUINFO_INT_REGISTER + MB86233_D:     info->i = cpustate->d;     break;
    case CPUINFO_INT_REGISTER + MB86233_REP:   info->i = cpustate->r;     break;
    case CPUINFO_INT_SP:
    case CPUINFO_INT_REGISTER + MB86233_SP:    info->i = cpustate->sp;    break;
    case CPUINFO_INT_REGISTER + MB86233_EB:    info->i = cpustate->eb;    break;
    case CPUINFO_INT_REGISTER + MB86233_SHIFT: info->i = cpustate->sft;   break;
    case CPUINFO_INT_REGISTER + MB86233_FLAGS: info->i = cpustate->st;    break;
    case CPUINFO_INT_REGISTER + MB86233_R0:    info->i = cpustate->b0;    break;
    case CPUINFO_INT_REGISTER + MB86233_R1:    info->i = cpustate->b1;    break;
    case CPUINFO_INT_REGISTER + MB86233_R2:    info->i = cpustate->x0;    break;
    case CPUINFO_INT_REGISTER + MB86233_R3:    info->i = cpustate->x1;    break;
    case CPUINFO_INT_REGISTER + MB86233_R4:    info->i = cpustate->i0;    break;
    case CPUINFO_INT_REGISTER + MB86233_R5:    info->i = cpustate->i1;    break;
    case CPUINFO_INT_REGISTER + MB86233_R6:    info->i = cpustate->c0;    break;
    case CPUINFO_INT_REGISTER + MB86233_R7:    info->i = cpustate->c1;    break;
    case CPUINFO_INT_REGISTER + MB86233_R8:    info->i = cpustate->pcs[0];break;
    case CPUINFO_INT_REGISTER + MB86233_R9:    info->i = cpustate->pcs[1];break;
    case CPUINFO_INT_REGISTER + MB86233_R10:   info->i = cpustate->pcs[2];break;
    case CPUINFO_INT_REGISTER + MB86233_R11:   info->i = cpustate->pcs[3];break;
    case CPUINFO_INT_REGISTER + MB86233_R12:   info->i = cpustate->vsm;   break;
    case CPUINFO_INT_REGISTER + MB86233_R13:   info->i = cpustate->mask;  break;
    case CPUINFO_INT_REGISTER + MB86233_R14:   info->i = cpustate->m;     break;
    case CPUINFO_INT_REGISTER + MB86233_R15:   info->i = cpustate->rpc;   break;

    case CPUINFO_FCT_SET_INFO:           info->setinfo = CPU_SET_INFO_NAME(mb86233);    break;
    case CPUINFO_FCT_INIT:               info->init    = CPU_INIT_NAME(mb86233);        break;
    case CPUINFO_FCT_RESET:              info->reset   = CPU_RESET_NAME(mb86233);       break;
    case CPUINFO_FCT_EXIT:               info->exit    = NULL;                          break;
    case CPUINFO_FCT_EXECUTE:            info->execute = CPU_EXECUTE_NAME(mb86233);     break;
    case CPUINFO_FCT_BURN:               info->burn    = NULL;                          break;
    case CPUINFO_FCT_DISASSEMBLE:        info->disassemble = CPU_DISASSEMBLE_NAME(mb86233); break;
    case CPUINFO_PTR_INSTRUCTION_COUNTER: info->icount = &cpustate->icount;             break;

    case DEVINFO_STR_NAME:               strcpy(info->s, "MB86233");                    break;
    case DEVINFO_STR_FAMILY:             strcpy(info->s, "Fujitsu MB86233");            break;
    case DEVINFO_STR_VERSION:            strcpy(info->s, "2.0");                        break;
    case DEVINFO_STR_SOURCE_FILE:        strcpy(info->s, __FILE__);                     break;
    case DEVINFO_STR_CREDITS:            strcpy(info->s, "Copyright Miguel Angel Horna and Ernesto Corvi"); break;

    case CPUINFO_STR_FLAGS:
        sprintf(info->s, "%c%c", (cpustate->st & F_SGD) ? 'N' : 'n',
                                  (cpustate->st & F_ZRD) ? 'Z' : 'z');
        break;

    case CPUINFO_STR_REGISTER + MB86233_FLAGS:
        sprintf(info->s, "FL:%c%c", (cpustate->st & F_SGD) ? 'N' : 'n',
                                     (cpustate->st & F_ZRD) ? 'Z' : 'z');
        break;

    case CPUINFO_STR_REGISTER + MB86233_PC:    sprintf(info->s, "PC:%04X", cpustate->pc);  break;
    case CPUINFO_STR_REGISTER + MB86233_A:     sprintf(info->s, "PA:%08X (%f)", cpustate->a, u2f(cpustate->a)); break;
    case CPUINFO_STR_REGISTER + MB86233_B:     sprintf(info->s, "PB:%08X (%f)", cpustate->b, u2f(cpustate->b)); break;
    case CPUINFO_STR_REGISTER + MB86233_P:     sprintf(info->s, "PP:%08X (%f)", cpustate->p, u2f(cpustate->p)); break;
    case CPUINFO_STR_REGISTER + MB86233_D:     sprintf(info->s, "PD:%08X (%f)", cpustate->d, u2f(cpustate->d)); break;
    case CPUINFO_STR_REGISTER + MB86233_REP:   sprintf(info->s, "REPS:%02X",   cpustate->r);   break;
    case CPUINFO_STR_REGISTER + MB86233_SP:    sprintf(info->s, "SP:%04X",     cpustate->sp);  break;
    case CPUINFO_STR_REGISTER + MB86233_EB:    sprintf(info->s, "EB:%08X",     cpustate->eb);  break;
    case CPUINFO_STR_REGISTER + MB86233_SHIFT: sprintf(info->s, "SHIFT:%02X",  cpustate->sft); break;
    case CPUINFO_STR_REGISTER + MB86233_R0:    sprintf(info->s, "B0:%04X",     cpustate->b0);  break;
    case CPUINFO_STR_REGISTER + MB86233_R1:    sprintf(info->s, "B1:%04X",     cpustate->b1);  break;
    case CPUINFO_STR_REGISTER + MB86233_R2:    sprintf(info->s, "X0:%04X",     cpustate->x0);  break;
    case CPUINFO_STR_REGISTER + MB86233_R3:    sprintf(info->s, "X1:%04X",     cpustate->x1);  break;
    case CPUINFO_STR_REGISTER + MB86233_R4:    sprintf(info->s, "I0:%04X",     cpustate->i0);  break;
    case CPUINFO_STR_REGISTER + MB86233_R5:    sprintf(info->s, "I1:%04X",     cpustate->i1);  break;
    case CPUINFO_STR_REGISTER + MB86233_R6:    sprintf(info->s, "C0:%02X",     cpustate->c0);  break;
    case CPUINFO_STR_REGISTER + MB86233_R7:    sprintf(info->s, "C1:%02X",     cpustate->c1);  break;
    case CPUINFO_STR_REGISTER + MB86233_R8:    sprintf(info->s, "PCS0:%04X",   cpustate->pcs[0]); break;
    case CPUINFO_STR_REGISTER + MB86233_R9:    sprintf(info->s, "PCS1:%04X",   cpustate->pcs[1]); break;
    case CPUINFO_STR_REGISTER + MB86233_R10:   sprintf(info->s, "PCS2:%04X",   cpustate->pcs[2]); break;
    case CPUINFO_STR_REGISTER + MB86233_R11:   sprintf(info->s, "PCS3:%04X",   cpustate->pcs[3]); break;
    case CPUINFO_STR_REGISTER + MB86233_R12:   sprintf(info->s, "VSM:%02X",    cpustate->vsm); break;
    case CPUINFO_STR_REGISTER + MB86233_R13:   sprintf(info->s, "MASK:%04X",   cpustate->mask); break;
    case CPUINFO_STR_REGISTER + MB86233_R14:   sprintf(info->s, "M:%04X",      cpustate->m);   break;
    case CPUINFO_STR_REGISTER + MB86233_R15:   sprintf(info->s, "RPC:%02X",    cpustate->rpc); break;
    }
}

DEFINE_LEGACY_CPU_DEVICE(MB86233, mb86233);
