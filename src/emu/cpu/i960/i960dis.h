#ifndef __I960DIS_H__
#define __I960DIS_H__

typedef struct _disassemble_t disassemble_t;
struct _disassemble_t
{
	char		*buffer;	// output buffer
	unsigned long	IP;
	unsigned long	IPinc;
	const uint8_t *oprom;
	uint32_t disflags;
};

#endif /* __I960DIS_H__ */
