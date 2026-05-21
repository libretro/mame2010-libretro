/*************************************************************************

    Sega Z80-3D system

*************************************************************************/

#include "sound/discrete.h"

/* sprites are scaled in the analog domain; to give a better */
/* rendition of this, we scale in the X direction by this factor */
#define TURBO_X_SCALE		2


struct i8279_state
{
	uint8_t		command;
	uint8_t		mode;
	uint8_t		prescale;
	uint8_t		inhibit;
	uint8_t		clear;
	uint8_t		ram[16];
};


class turbo_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, turbo_state(machine)); }

	turbo_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *		videoram;
	uint8_t *		spriteram;
	uint8_t *		sprite_position;
	uint8_t *		buckrog_bitmap_ram;

	/* machine states */
	i8279_state	i8279;

	/* sound state */
	uint8_t		turbo_osel, turbo_bsel;
	uint8_t		sound_state[3];

	/* video state */
	tilemap_t *	fg_tilemap;

	/* Turbo-specific states */
	uint8_t		turbo_opa, turbo_opb, turbo_opc;
	uint8_t		turbo_ipa, turbo_ipb, turbo_ipc;
	uint8_t		turbo_fbpla, turbo_fbcol;
	uint8_t		turbo_speed;
	uint8_t		turbo_collision;
	uint8_t		turbo_last_analog;
	uint8_t		turbo_accel;

	/* Subroc-specific states */
	uint8_t		subroc3d_col, subroc3d_ply, subroc3d_flip;
	uint8_t		subroc3d_mdis, subroc3d_mdir;
	uint8_t		subroc3d_tdis, subroc3d_tdir;
	uint8_t		subroc3d_fdis, subroc3d_fdir;
	uint8_t		subroc3d_hdis, subroc3d_hdir;

	/* Buck Rogers-specific states */
	uint8_t		buckrog_fchg, buckrog_mov, buckrog_obch;
	uint8_t		buckrog_command;
	uint8_t		buckrog_myship;
};


/*----------- defined in audio/turbo.c -----------*/

MACHINE_DRIVER_EXTERN( turbo_samples );
MACHINE_DRIVER_EXTERN( subroc3d_samples );
MACHINE_DRIVER_EXTERN( buckrog_samples );

WRITE8_DEVICE_HANDLER( turbo_sound_a_w );
WRITE8_DEVICE_HANDLER( turbo_sound_b_w );
WRITE8_DEVICE_HANDLER( turbo_sound_c_w );

WRITE8_DEVICE_HANDLER( subroc3d_sound_a_w );
WRITE8_DEVICE_HANDLER( subroc3d_sound_b_w );
WRITE8_DEVICE_HANDLER( subroc3d_sound_c_w );

WRITE8_DEVICE_HANDLER( buckrog_sound_a_w );
WRITE8_DEVICE_HANDLER( buckrog_sound_b_w );


/*----------- defined in video/turbo.c -----------*/

PALETTE_INIT( turbo );
VIDEO_START( turbo );
VIDEO_UPDATE( turbo );

PALETTE_INIT( subroc3d );
VIDEO_UPDATE( subroc3d );

PALETTE_INIT( buckrog );
VIDEO_START( buckrog );
VIDEO_UPDATE( buckrog );

WRITE8_HANDLER( turbo_videoram_w );
WRITE8_HANDLER( buckrog_bitmap_w );
