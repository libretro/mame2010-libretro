#define FD1094_STATE_RESET	0x0100
#define FD1094_STATE_IRQ	0x0200
#define FD1094_STATE_RTE	0x0300

int fd1094_set_state(uint8_t *key,int state);
int fd1094_decode(int address,int val,uint8_t *key,int vector_fetch);

typedef struct _fd1094_constraint fd1094_constraint;
struct _fd1094_constraint
{
	offs_t	pc;
	uint16_t	state;
	uint16_t	value;
	uint16_t	mask;
};
