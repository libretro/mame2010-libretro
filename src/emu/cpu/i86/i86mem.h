typedef struct _memory_interface memory_interface;
struct _memory_interface
{
	offs_t	fetch_xor;

	uint8_t	(*rbyte)(const address_space *, offs_t);
	uint16_t	(*rword)(const address_space *, offs_t);
	void	(*wbyte)(const address_space *, offs_t, uint8_t);
	void	(*wword)(const address_space *, offs_t, uint16_t);
};
