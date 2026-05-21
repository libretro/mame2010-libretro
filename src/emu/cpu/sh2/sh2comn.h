/*****************************************************************************
 *
 *   sh2common.h
 *
 *   SH-2 non-specific components
 *
 *****************************************************************************/

#pragma once

#ifndef __SH2COMN_H__
#define __SH2COMN_H__

#if !defined(DISABLE_SH2DRC)
	#define USE_SH2DRC
#endif

#ifdef USE_SH2DRC
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"
#endif

#define SH2_CODE_XOR(a)		((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(2,0))

typedef struct _irq_entry irq_entry;
struct _irq_entry
{
	int irq_vector;
	int irq_priority;
};

enum
{
	ICF  = 0x00800000,
	OCFA = 0x00080000,
	OCFB = 0x00040000,
	OVF  = 0x00020000
};

#define T	0x00000001
#define S	0x00000002
#define I	0x000000f0
#define Q	0x00000100
#define M	0x00000200

#define AM	0xc7ffffff

#define FLAGS	(M|Q|I|S|T)

#define Rn	((opcode>>8)&15)
#define Rm	((opcode>>4)&15)

#define CPU_TYPE_SH1	(0)
#define CPU_TYPE_SH2	(1)

#define REGFLAG_R(n)                                        (1 << (n))

/* register flags 1 */
#define REGFLAG_PR						(1 << 0)
#define REGFLAG_MACL						(1 << 1)
#define REGFLAG_MACH						(1 << 2)
#define REGFLAG_GBR						(1 << 3)
#define REGFLAG_VBR						(1 << 4)
#define REGFLAG_SR						(1 << 5)

#define CHECK_PENDING_IRQ(message)				\
do {											\
	int irq = -1;								\
	if (sh2->pending_irq & (1 <<  0)) irq =	0;	\
	if (sh2->pending_irq & (1 <<  1)) irq =	1;	\
	if (sh2->pending_irq & (1 <<  2)) irq =	2;	\
	if (sh2->pending_irq & (1 <<  3)) irq =	3;	\
	if (sh2->pending_irq & (1 <<  4)) irq =	4;	\
	if (sh2->pending_irq & (1 <<  5)) irq =	5;	\
	if (sh2->pending_irq & (1 <<  6)) irq =	6;	\
	if (sh2->pending_irq & (1 <<  7)) irq =	7;	\
	if (sh2->pending_irq & (1 <<  8)) irq =	8;	\
	if (sh2->pending_irq & (1 <<  9)) irq =	9;	\
	if (sh2->pending_irq & (1 << 10)) irq = 10;	\
	if (sh2->pending_irq & (1 << 11)) irq = 11;	\
	if (sh2->pending_irq & (1 << 12)) irq = 12;	\
	if (sh2->pending_irq & (1 << 13)) irq = 13;	\
	if (sh2->pending_irq & (1 << 14)) irq = 14;	\
	if (sh2->pending_irq & (1 << 15)) irq = 15;	\
	if ((sh2->internal_irq_level != -1) && (sh2->internal_irq_level > irq)) irq = sh2->internal_irq_level; \
	if (irq >= 0)								\
		sh2_exception(sh2,message,irq);			\
} while(0)

typedef struct
{
	uint32_t	ppc;
	uint32_t	pc;
	uint32_t	pr;
	uint32_t	sr;
	uint32_t	gbr, vbr;
	uint32_t	mach, macl;
	uint32_t	r[16];
	uint32_t	ea;
	uint32_t	delay;
	uint32_t	cpu_off;
	uint32_t	dvsr, dvdnth, dvdntl, dvcr;
	uint32_t	pending_irq;
	uint32_t	test_irq;
	uint32_t	pending_nmi;
	int32_t  irqline;
	uint32_t	evec;				// exception vector for DRC
	uint32_t  irqsr;				// IRQ-time old SR for DRC
	uint32_t target;				// target for jmp/jsr/etc so the delay slot can't kill it
	irq_entry     irq_queue[16];

	int pcfsel;	    			// last pcflush entry set
	int maxpcfsel;				// highest valid pcflush entry
	uint32_t pcflushes[16];			// pcflush entries

	int8_t	irq_line_state[17];
	device_irq_callback irq_callback;
	legacy_cpu_device *device;
	const address_space *program;
	const address_space *internal;
	uint32_t	*m;
	int8_t  nmi_line_state;

	uint16_t	frc;
	uint16_t	ocra, ocrb, icr;
	uint64_t	frc_base;

	int		frt_input;
	int 	internal_irq_level;
	int 	internal_irq_vector;
	int				icount;

	emu_timer *timer;
	emu_timer *dma_timer[2];
	int     dma_timer_active[2];

	int     is_slave, cpu_type;
	int  (*dma_callback_kludge)(uint32_t src, uint32_t dst, uint32_t data, int size);

	void	(*ftcsr_read_callback)(uint32_t data);

#ifdef USE_SH2DRC
	drccache *			cache;			    	/* pointer to the DRC code cache */
	drcuml_state *		drcuml;					/* DRC UML generator state */
	drcfe_state *		drcfe;					/* pointer to the DRC front-end state */
	uint32_t				drcoptions;			/* configurable DRC options */

	/* internal stuff */
	uint8_t				cache_dirty;		    	/* true if we need to flush the cache */

	/* parameters for subroutines */
	uint64_t				numcycles;		    	/* return value from gettotalcycles */
	uint32_t				arg0;			    	/* print_debug argument 1 */
	uint32_t				arg1;			    	/* print_debug argument 2 */
	uint32_t				irq;				/* irq we're taking */

	/* register mappings */
	drcuml_parameter	regmap[16];		    		/* parameter to register mappings for all 16 integer registers */

	drcuml_codehandle *	entry;			    		/* entry point */
	drcuml_codehandle *	read8;					/* read byte */
	drcuml_codehandle *	write8;					/* write byte */
	drcuml_codehandle *	read16;					/* read half */
	drcuml_codehandle *	write16;		    		/* write half */
	drcuml_codehandle *	read32;					/* read word */
	drcuml_codehandle *	write32;		    		/* write word */

	drcuml_codehandle *	interrupt;				/* interrupt */
	drcuml_codehandle *	nocode;					/* nocode */
	drcuml_codehandle *	out_of_cycles;				/* out of cycles exception handler */
#endif
} sh2_state;

void sh2_common_init(sh2_state *sh2, legacy_cpu_device *device, device_irq_callback irqcallback);
void sh2_recalc_irq(sh2_state *sh2);
void sh2_set_irq_line(sh2_state *sh2, int irqline, int state);
void sh2_exception(sh2_state *sh2, const char *message, int irqline);

#endif /* __SH2COMN_H__ */
