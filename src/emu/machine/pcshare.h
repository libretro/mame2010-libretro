/* flags for init_pc_common */
#define PCCOMMON_KEYBOARD_PC	0
#define PCCOMMON_KEYBOARD_AT	1

void init_pc_common(running_machine *machine, uint32_t flags, void (*set_keyb_int_func)(running_machine *, int));

void pc_keyboard(void);
uint8_t pc_keyb_read(void);
void pc_keyb_set_clock(int on);
void pc_keyb_clear(void);

extern IRQ_CALLBACK(pcat_irq_callback);
ADDRESS_MAP_EXTERN(pcat32_io_common, 32);
MACHINE_DRIVER_EXTERN(pcat_common);
