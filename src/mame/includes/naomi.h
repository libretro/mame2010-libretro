/*

naomi.h -> NAOMI includes

*/

enum {
	JVSBD_DEFAULT = 0,
	JVSBD_ADSTICK,
	JVSBD_LIGHTGUN,
	JVSBD_MAHJONG,
	JVSBD_KEYBOARD
};

/*----------- defined in machine/gdcrypt.c -----------*/

extern void naomi_game_decrypt(running_machine* machine, uint64_t key, uint8_t* region, int length);


/*----------- defined in machine/naomi.c -----------*/

extern DRIVER_INIT( naomi );
extern DRIVER_INIT( naomi_mp );
extern DRIVER_INIT( naomi2 );

extern DRIVER_INIT( sfz3ugd );
extern DRIVER_INIT( ggxxsla );
extern DRIVER_INIT( ggxxrl );
extern DRIVER_INIT( ggxx );

extern DRIVER_INIT( gram2000 );
extern DRIVER_INIT( mvsc2 );
extern DRIVER_INIT( qmegamis );

extern DRIVER_INIT( vf4evoct );

extern DRIVER_INIT( kick4csh );

extern uint64_t *naomi_ram64;

extern int jvsboard_type;
extern uint16_t actel_id;
