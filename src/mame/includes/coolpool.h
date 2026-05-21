class coolpool_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, coolpool_state(machine)); }

	coolpool_state(running_machine &machine) { }

	uint16_t *vram_base;

	uint8_t cmd_pending;
	uint16_t iop_cmd;
	uint16_t iop_answer;
	int iop_romaddr;

	uint8_t newx[3];
	uint8_t newy[3];
	uint8_t oldx[3];
	uint8_t oldy[3];
	int dx[3];
	int dy[3];

	uint16_t result;
	uint16_t lastresult;

	running_device *maincpu;
	running_device *dsp;
};
