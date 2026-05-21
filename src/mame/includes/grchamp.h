/*************************************************************************

    Taito Grand Champ hardware

*************************************************************************/

#include "sound/discrete.h"

class grchamp_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, grchamp_state(machine)); }

	grchamp_state(running_machine &machine) { }

	uint8_t		cpu0_out[16];
	uint8_t		cpu1_out[16];

	uint8_t		comm_latch;
	uint8_t		comm_latch2[4];

	uint16_t		ledlatch;
	uint8_t		ledaddr;
	uint16_t		ledram[8];

	uint16_t		collide;
	uint8_t		collmode;

	uint8_t *		radarram;
	uint8_t *		videoram;
	uint8_t *		leftram;
	uint8_t *		centerram;
	uint8_t *		rightram;
	uint8_t *		spriteram;

	bitmap_t *	work_bitmap;
	tilemap_t *	text_tilemap;
	tilemap_t *	left_tilemap;
	tilemap_t *	center_tilemap;
	tilemap_t *	right_tilemap;

	rgb_t		bgcolor[0x20];
};

/* Discrete Sound Input Nodes */
#define GRCHAMP_ENGINE_CS_EN				NODE_01
#define GRCHAMP_SIFT_DATA					NODE_02
#define GRCHAMP_ATTACK_UP_DATA				NODE_03
#define GRCHAMP_IDLING_EN					NODE_04
#define GRCHAMP_FOG_EN						NODE_05
#define GRCHAMP_PLAYER_SPEED_DATA			NODE_06
#define GRCHAMP_ATTACK_SPEED_DATA			NODE_07
#define GRCHAMP_A_DATA						NODE_08
#define GRCHAMP_B_DATA						NODE_09

/*----------- defined in audio/grchamp.c -----------*/

DISCRETE_SOUND_EXTERN( grchamp );

/*----------- defined in video/grchamp.c -----------*/

PALETTE_INIT( grchamp );
VIDEO_START( grchamp );
VIDEO_UPDATE( grchamp );
WRITE8_HANDLER( grchamp_left_w );
WRITE8_HANDLER( grchamp_center_w );
WRITE8_HANDLER( grchamp_right_w );
