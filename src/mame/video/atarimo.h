/***************************************************************************

    atarimo.h

    Common motion object management functions for Atari raster games.

***************************************************************************/

#ifndef __ATARIMO__
#define __ATARIMO__


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* maximum number of motion object processors */
#define ATARIMO_MAX				2

/* maximum objects per bank */
#define ATARIMO_MAXPERBANK		1024

/* shift to get to priority in raw data */
#define ATARIMO_PRIORITY_SHIFT	12
#define ATARIMO_PRIORITY_MASK	((~0 << ATARIMO_PRIORITY_SHIFT) & 0xffff)
#define ATARIMO_DATA_MASK		(ATARIMO_PRIORITY_MASK ^ 0xffff)



/***************************************************************************
    TYPES & STRUCTURES
***************************************************************************/

/* callback for special processing */
typedef int (*atarimo_special_func)(bitmap_t *bitmap, const rectangle *clip, int code, int color, int xpos, int ypos, rectangle *mobounds);

/* description for a four-word mask */
typedef struct _atarimo_entry atarimo_entry;
struct _atarimo_entry
{
	uint16_t			data[4];
};

/* description of the motion objects */
typedef struct _atarimo_desc atarimo_desc;
struct _atarimo_desc
{
	uint8_t				gfxindex;			/* index to which gfx system */
	uint8_t				banks;				/* number of motion object banks */
	uint8_t				linked;				/* are the entries linked? */
	uint8_t				split;				/* are the entries split? */
	uint8_t				reverse;			/* render in reverse order? */
	uint8_t				swapxy;				/* render in swapped X/Y order? */
	uint8_t				nextneighbor;		/* does the neighbor bit affect the next object? */
	uint16_t				slipheight;			/* pixels per SLIP entry (0 for no-slip) */
	uint8_t				slipoffset;			/* pixel offset for SLIPs */
	uint16_t				maxlinks;			/* maximum number of links to visit/scanline (0=all) */

	uint16_t				palettebase;		/* base palette entry */
	uint16_t				maxcolors;			/* maximum number of colors */
	uint8_t				transpen;			/* transparent pen index */

	atarimo_entry		linkmask;			/* mask for the link */
	atarimo_entry		gfxmask;			/* mask for the graphics bank */
	atarimo_entry		codemask;			/* mask for the code index */
	atarimo_entry		codehighmask;		/* mask for the upper code index */
	atarimo_entry		colormask;			/* mask for the color */
	atarimo_entry		xposmask;			/* mask for the X position */
	atarimo_entry		yposmask;			/* mask for the Y position */
	atarimo_entry		widthmask;			/* mask for the width, in tiles*/
	atarimo_entry		heightmask;			/* mask for the height, in tiles */
	atarimo_entry		hflipmask;			/* mask for the horizontal flip */
	atarimo_entry		vflipmask;			/* mask for the vertical flip */
	atarimo_entry		prioritymask;		/* mask for the priority */
	atarimo_entry		neighbormask;		/* mask for the neighbor */
	atarimo_entry		absolutemask;		/* mask for absolute coordinates */

	atarimo_entry		specialmask;		/* mask for the special value */
	uint16_t				specialvalue;		/* resulting value to indicate "special" */
	atarimo_special_func specialcb;			/* callback routine for special entries */
};

/* rectangle list */
typedef struct _atarimo_rect_list atarimo_rect_list;
struct _atarimo_rect_list
{
	int					numrects;
	rectangle *			rect;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* setup/shutdown */
void atarimo_init(running_machine *machine, int map, const atarimo_desc *desc);
uint16_t *atarimo_get_code_lookup(int map, int *size);
uint8_t *atarimo_get_color_lookup(int map, int *size);
uint8_t *atarimo_get_gfx_lookup(int map, int *size);

/* core processing */
bitmap_t *atarimo_render(int map, const rectangle *cliprect, atarimo_rect_list *rectlist);

/* atrribute setters */
void atarimo_set_bank(int map, int bank);
void atarimo_set_xscroll(int map, int xscroll);
void atarimo_set_yscroll(int map, int yscroll);

/* atrribute getters */
int atarimo_get_bank(int map);
int atarimo_get_xscroll(int map);
int atarimo_get_yscroll(int map);

/* write handlers */
WRITE16_HANDLER( atarimo_0_spriteram_w );
WRITE16_HANDLER( atarimo_0_spriteram_expanded_w );
WRITE16_HANDLER( atarimo_0_slipram_w );

WRITE16_HANDLER( atarimo_1_spriteram_w );
WRITE16_HANDLER( atarimo_1_slipram_w );



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern uint16_t *atarimo_0_spriteram;
extern uint16_t *atarimo_0_slipram;

extern uint16_t *atarimo_1_spriteram;
extern uint16_t *atarimo_1_slipram;


#endif
