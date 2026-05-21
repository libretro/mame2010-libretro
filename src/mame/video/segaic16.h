/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/

#include "devcb.h"

/* globals */
extern uint8_t segaic16_display_enable;
extern uint16_t *segaic16_tileram_0;
extern uint16_t *segaic16_textram_0;
extern uint16_t *segaic16_spriteram_0;
extern uint16_t *segaic16_spriteram_1;
extern uint16_t *segaic16_roadram_0;
extern uint16_t *segaic16_rotateram_0;
extern uint16_t *segaic16_paletteram;

/* misc functions */
void segaic16_set_display_enable(running_machine *machine, int enable);

/* palette handling */
void segaic16_palette_init(int entries);
WRITE16_HANDLER( segaic16_paletteram_w );

/* tilemap systems */
#define SEGAIC16_MAX_TILEMAPS		1

#define SEGAIC16_TILEMAP_HANGON		0
#define SEGAIC16_TILEMAP_16A		1
#define SEGAIC16_TILEMAP_16B		2
#define SEGAIC16_TILEMAP_16B_ALT	3

#define SEGAIC16_TILEMAP_FOREGROUND	0
#define SEGAIC16_TILEMAP_BACKGROUND	1
#define SEGAIC16_TILEMAP_TEXT		2

void segaic16_tilemap_init(running_machine *machine, int which, int type, int colorbase, int xoffs, int numbanks);
void segaic16_tilemap_reset(running_machine *machine, int which);
void segaic16_tilemap_draw(running_device *screen, bitmap_t *bitmap, const rectangle *cliprect, int which, int map, int priority, int priority_mark);
void segaic16_tilemap_set_bank(running_machine *machine, int which, int banknum, int offset);
void segaic16_tilemap_set_flip(running_machine *machine, int which, int flip);
void segaic16_tilemap_set_rowscroll(running_machine *machine, int which, int enable);
void segaic16_tilemap_set_colscroll(running_machine *machine, int which, int enable);

WRITE16_HANDLER( segaic16_tileram_0_w );
WRITE16_HANDLER( segaic16_textram_0_w );

/* sprite systems */
#define SEGAIC16_MAX_SPRITES		2

#define SEGAIC16_SPRITES_OUTRUN		4
#define SEGAIC16_SPRITES_XBOARD		5

void segaic16_sprites_draw(running_device *screen, bitmap_t *bitmap, const rectangle *cliprect, int which);
void segaic16_sprites_set_bank(running_machine *machine, int which, int banknum, int offset);
void segaic16_sprites_set_flip(running_machine *machine, int which, int flip);
void segaic16_sprites_set_shadow(running_machine *machine, int which, int shadow);
WRITE16_HANDLER( segaic16_sprites_draw_0_w );
WRITE16_HANDLER( segaic16_sprites_draw_1_w );

/* road systems */
#define SEGAIC16_MAX_ROADS			1

#define SEGAIC16_ROAD_HANGON		0
#define SEGAIC16_ROAD_SHARRIER		1
#define SEGAIC16_ROAD_OUTRUN		2
#define SEGAIC16_ROAD_XBOARD		3

#define SEGAIC16_ROAD_BACKGROUND	0
#define SEGAIC16_ROAD_FOREGROUND	1

void segaic16_road_init(running_machine *machine, int which, int type, int colorbase1, int colorbase2, int colorbase3, int xoffs);
void segaic16_road_draw(int which, bitmap_t *bitmap, const rectangle *cliprect, int priority);
READ16_HANDLER( segaic16_road_control_0_r );
WRITE16_HANDLER( segaic16_road_control_0_w );

/* rotation systems */
#define SEGAIC16_MAX_ROTATE			1

#define SEGAIC16_ROTATE_YBOARD		0

void segaic16_rotate_init(running_machine *machine, int which, int type, int colorbase);
void segaic16_rotate_draw(running_machine *machine, int which, bitmap_t *bitmap, const rectangle *cliprect, bitmap_t *srcbitmap);
READ16_HANDLER( segaic16_rotate_control_0_r );

/*************************************
 *
 *  Type definitions
 *
 *************************************/



struct tilemap_callback_info
{
	uint16_t *		rambase;						/* base of RAM for this tilemap page */
	const uint8_t *	bank;							/* pointer to bank array */
	uint16_t			banksize;						/* size of banks */
};


struct tilemap_info
{
	uint8_t			index;							/* index of this structure */
	uint8_t			type;							/* type of tilemap (see segaic16.h for details) */
	uint8_t			numpages;						/* number of allocated pages */
	uint8_t			flip;							/* screen flip? */
	uint8_t			rowscroll, colscroll;			/* are rowscroll/colscroll enabled (if external enables are used) */
	uint8_t			bank[8];						/* indexes of the tile banks */
	uint16_t			banksize;						/* number of tiles per bank */
	uint16_t			latched_xscroll[4];				/* latched X scroll values */
	uint16_t			latched_yscroll[4];				/* latched Y scroll values */
	uint16_t			latched_pageselect[4];			/* latched page select values */
	int32_t			xoffs;							/* X scroll offset */
	tilemap_t *		tilemaps[16];					/* up to 16 tilemap pages */
	tilemap_t *		textmap;						/* a single text tilemap */
	struct tilemap_callback_info tmap_info[16];		/* callback info for 16 tilemap pages */
	struct tilemap_callback_info textmap_info;		/* callback info for a single textmap page */
	void			(*reset)(running_machine *machine, struct tilemap_info *info);/* reset callback */
	void			(*draw_layer)(running_machine *machine, struct tilemap_info *info, bitmap_t *bitmap, const rectangle *cliprect, int which, int flags, int priority);
	uint16_t *		textram;						/* pointer to textram pointer */
	uint16_t *		tileram;						/* pointer to tileram pointer */
};

struct road_info
{
	uint8_t			index;							/* index of this structure */
	uint8_t			type;							/* type of road system (see segaic16.h for details) */
	uint8_t			control;						/* control register value */
	uint16_t			colorbase1;						/* color base for road ROM data */
	uint16_t			colorbase2;						/* color base for road background data */
	uint16_t			colorbase3;						/* color base for sky data */
	int32_t			xoffs;							/* X scroll offset */
	void			(*draw)(struct road_info *info, bitmap_t *bitmap, const rectangle *cliprect, int priority);
	uint16_t *		roadram;						/* pointer to roadram pointer */
	uint16_t *		buffer;							/* buffered roadram pointer */
	uint8_t *			gfx;							/* expanded road graphics */
};

struct palette_info
{
	int32_t			entries;						/* number of entries (not counting shadows) */
	uint8_t			normal[32];						/* RGB translations for normal pixels */
	uint8_t			shadow[32];						/* RGB translations for shadowed pixels */
	uint8_t			hilight[32];					/* RGB translations for hilighted pixels */
};

struct rotate_info
{
	uint8_t			index;							/* index of this structure */
	uint8_t			type;							/* type of rotate system (see segaic16.h for details) */
	uint16_t			colorbase;						/* base color index */
	int32_t			ramsize;						/* size of rotate RAM */
	uint16_t *		rotateram;						/* pointer to rotateram pointer */
	uint16_t *		buffer;							/* buffered data */
};

/* interface */
typedef struct _sega16sp_interface sega16sp_interface;
struct _sega16sp_interface
{
	uint8_t			which;							/* which sprite RAM */
	uint16_t			colorbase;						/* base color index */
	int32_t			ramsize;						/* size of sprite RAM */
	int32_t			xoffs;							/* X scroll offset */
	void			(*draw)(running_machine *machine, running_device* device, bitmap_t *bitmap, const rectangle *cliprect);
	int				buffer;							/* should ram be buffered? */
};




/* state */
typedef struct _sega16sp_state sega16sp_state;
struct _sega16sp_state
{
	uint8_t			which;							/* which sprite RAM */
	uint8_t			flip;							/* screen flip? */
	uint8_t			shadow;							/* shadow or hilight? */
	uint8_t			bank[16];						/* banking redirection */
	uint16_t			colorbase;						/* base color index */
	int32_t			ramsize;						/* size of sprite RAM */
	int32_t			xoffs;							/* X scroll offset */
	void			(*draw)(running_machine *machine, running_device* device, bitmap_t *bitmap, const rectangle *cliprect);
	uint16_t *		spriteram;						/* pointer to spriteram pointer */
	uint16_t *		buffer;							/* buffered spriteram for those that use it */

};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DECLARE_LEGACY_DEVICE(SEGA16SP, sega16sp);

void segaic16_sprites_hangon_draw(running_machine *machine, running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void segaic16_sprites_sharrier_draw(running_machine *machine, running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void segaic16_sprites_16a_draw(running_machine *machine, running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void segaic16_sprites_16b_draw(running_machine *machine, running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void segaic16_sprites_yboard_16b_draw(running_machine *machine, running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void segaic16_sprites_yboard_draw(running_machine *machine, running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void segaic16_sprites_outrun_draw(running_machine *machine, running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void segaic16_sprites_xboard_draw(running_machine *machine, running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void segaic16_sprites_16a_bootleg_wb3bl_draw(running_machine *machine, running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void segaic16_sprites_16a_bootleg_passhtb_draw(running_machine *machine, running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void segaic16_sprites_16a_bootleg_shinobld_draw(running_machine *machine, running_device *device, bitmap_t *bitmap, const rectangle *cliprect);

/* the various sprite configs */
static const sega16sp_interface hangon_sega16sp_intf =
{
	0,	   // which spriteram
	1024,  // colorbase
	0x800, // ramsize
	0,     // xoffs
	segaic16_sprites_hangon_draw, // draw function
	0, // use buffer
};

static const sega16sp_interface sharrier_sega16sp_intf =
{
	0,	   // which spriteram
	1024,  // colorbase
	0x1000, // ramsize
	0,     // xoffs
	segaic16_sprites_sharrier_draw, // draw function
	0, // use buffer
};

static const sega16sp_interface yboard_16b_sega16sp_intf =
{
	0,	   // which spriteram
	2048,  // colorbase
	0x800, // ramsize
	0,     // xoffs
	segaic16_sprites_yboard_16b_draw, // draw function
	0, // use buffer
};

static const sega16sp_interface yboard_sega16sp_intf =
{
	1,	   // which spriteram
	4096,  // colorbase
	0x10000, // ramsize
	0,     // xoffs
	segaic16_sprites_yboard_draw, // draw function
	0, // use buffer
};

static const sega16sp_interface s16a_sega16sp_intf =
{
	0,	   // which spriteram
	1024,  // colorbase
	0x800, // ramsize
	0,     // xoffs
	segaic16_sprites_16a_draw, // draw function
	0, // use buffer
};

static const sega16sp_interface s16b_sega16sp_intf =
{
	0,	   // which spriteram
	1024,  // colorbase
	0x800, // ramsize
	0,     // xoffs
	segaic16_sprites_16b_draw, // draw function
	0, // use buffer
};

static const sega16sp_interface outrun_sega16sp_intf =
{
	0,	   // which spriteram
	2048,  // colorbase
	0x1000, // ramsize
	0,     // xoffs
	segaic16_sprites_outrun_draw, // draw function
	1, // use buffer
};


static const sega16sp_interface xboard_sega16sp_intf =
{
	0,	   // which spriteram
	0,  // colorbase
	0x1000, // ramsize
	0,     // xoffs
	segaic16_sprites_xboard_draw, // draw function
	1, // use buffer
};



#define MDRV_SEGA16SP_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, SEGA16SP, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_SEGA16SP_ADD_HANGON(_tag) \
	MDRV_DEVICE_ADD(_tag, SEGA16SP, 0) \
	MDRV_DEVICE_CONFIG(hangon_sega16sp_intf)

#define MDRV_SEGA16SP_ADD_SHARRIER(_tag) \
	MDRV_DEVICE_ADD(_tag, SEGA16SP, 0) \
	MDRV_DEVICE_CONFIG(sharrier_sega16sp_intf)

#define MDRV_SEGA16SP_ADD_YBOARD(_tag) \
	MDRV_DEVICE_ADD(_tag, SEGA16SP, 0) \
	MDRV_DEVICE_CONFIG(yboard_sega16sp_intf)

#define MDRV_SEGA16SP_ADD_YBOARD_16B(_tag) \
	MDRV_DEVICE_ADD(_tag, SEGA16SP, 0) \
	MDRV_DEVICE_CONFIG(yboard_16b_sega16sp_intf)

#define MDRV_SEGA16SP_ADD_16A(_tag) \
	MDRV_DEVICE_ADD(_tag, SEGA16SP, 0) \
	MDRV_DEVICE_CONFIG(s16a_sega16sp_intf)

#define MDRV_SEGA16SP_ADD_16B(_tag) \
	MDRV_DEVICE_ADD(_tag, SEGA16SP, 0) \
	MDRV_DEVICE_CONFIG(s16b_sega16sp_intf)

#define MDRV_SEGA16SP_ADD_OUTRUN(_tag) \
	MDRV_DEVICE_ADD(_tag, SEGA16SP, 0) \
	MDRV_DEVICE_CONFIG(outrun_sega16sp_intf)

#define MDRV_SEGA16SP_ADD_XBOARD(_tag) \
	MDRV_DEVICE_ADD(_tag, SEGA16SP, 0) \
	MDRV_DEVICE_CONFIG(xboard_sega16sp_intf)


extern struct palette_info segaic16_palette;
extern struct rotate_info segaic16_rotate[SEGAIC16_MAX_ROTATE];
extern struct road_info segaic16_road[SEGAIC16_MAX_ROADS];

