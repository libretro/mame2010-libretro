/***************************************************************************

    crsshair.c

    Crosshair handling.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "rendutil.h"
#include "config.h"
#include "xmlfile.h"
#include "crsshair.h"

#include "retromain.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CROSSHAIR_RAW_SIZE		100
#define CROSSHAIR_RAW_ROWBYTES	((CROSSHAIR_RAW_SIZE + 7) / 8)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _crosshair_global crosshair_global;
struct _crosshair_global
{
	UINT8				usage;					/* true if any crosshairs are used */
	UINT8				used[MAX_PLAYERS];		/* usage per player */
	UINT8				mode[MAX_PLAYERS];		/* visibility mode per player */
	UINT8				visible[MAX_PLAYERS];	/* visibility per player */
	bitmap_t *			bitmap[MAX_PLAYERS];	/* bitmap per player */
	render_texture *	texture[MAX_PLAYERS];	/* texture per player */
	device_t *screen[MAX_PLAYERS];	/* the screen on which this player's crosshair is drawn */
	float				x[MAX_PLAYERS];			/* current X position */
	float				y[MAX_PLAYERS];			/* current Y position */
	float				last_x[MAX_PLAYERS];	/* last X position */
	float				last_y[MAX_PLAYERS];	/* last Y position */
	UINT8				fade;					/* color fading factor */
	UINT8				animation_counter;		/* animation frame index */
	UINT16				auto_time;				/* time in seconds to turn invisible */
	UINT16				time[MAX_PLAYERS];		/* time since last movement */
	char				name[MAX_PLAYERS][CROSSHAIR_PIC_NAME_LENGTH + 1];	/* name of crosshair png file */
};


/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* global state */
static crosshair_global global;

/* raw bitmap */
static const UINT8 crosshair_raw_top[] =
{
	0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,
	0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xe0,0x00,
	0x00,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xf0,0x00,
	0x01,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xf8,0x00,
	0x03,0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xfc,0x00,
	0x07,0xfe,0x00,0x00,0x00,0x0f,0xfe,0x00,0x00,0x00,0x07,0xfe,0x00,
	0x0f,0xff,0x00,0x00,0x01,0xff,0xff,0xf0,0x00,0x00,0x0f,0xff,0x00,
	0x1f,0xff,0x80,0x00,0x1f,0xff,0xff,0xff,0x00,0x00,0x1f,0xff,0x80,
	0x3f,0xff,0x80,0x00,0xff,0xff,0xff,0xff,0xe0,0x00,0x1f,0xff,0xc0,
	0x7f,0xff,0xc0,0x03,0xff,0xff,0xff,0xff,0xf8,0x00,0x3f,0xff,0xe0,
	0xff,0xff,0xe0,0x07,0xff,0xff,0xff,0xff,0xfc,0x00,0x7f,0xff,0xf0,
	0x7f,0xff,0xf0,0x1f,0xff,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xe0,
	0x3f,0xff,0xf8,0x7f,0xff,0xff,0xff,0xff,0xff,0xc1,0xff,0xff,0xc0,
	0x0f,0xff,0xf8,0xff,0xff,0xff,0xff,0xff,0xff,0xe1,0xff,0xff,0x00,
	0x07,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfb,0xff,0xfe,0x00,
	0x03,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0x00,
	0x01,0xff,0xff,0xff,0xff,0xf0,0x01,0xff,0xff,0xff,0xff,0xf8,0x00,
	0x00,0x7f,0xff,0xff,0xff,0x00,0x00,0x1f,0xff,0xff,0xff,0xe0,0x00,
	0x00,0x3f,0xff,0xff,0xf8,0x00,0x00,0x03,0xff,0xff,0xff,0xc0,0x00,
	0x00,0x1f,0xff,0xff,0xe0,0x00,0x00,0x00,0xff,0xff,0xff,0x80,0x00,
	0x00,0x0f,0xff,0xff,0x80,0x00,0x00,0x00,0x3f,0xff,0xff,0x00,0x00,
	0x00,0x03,0xff,0xfe,0x00,0x00,0x00,0x00,0x0f,0xff,0xfc,0x00,0x00,
	0x00,0x01,0xff,0xfc,0x00,0x00,0x00,0x00,0x07,0xff,0xf8,0x00,0x00,
	0x00,0x03,0xff,0xf8,0x00,0x00,0x00,0x00,0x01,0xff,0xf8,0x00,0x00,
	0x00,0x07,0xff,0xfc,0x00,0x00,0x00,0x00,0x03,0xff,0xfc,0x00,0x00,
	0x00,0x0f,0xff,0xfe,0x00,0x00,0x00,0x00,0x07,0xff,0xfe,0x00,0x00,
	0x00,0x0f,0xff,0xff,0x00,0x00,0x00,0x00,0x0f,0xff,0xfe,0x00,0x00,
	0x00,0x1f,0xff,0xff,0x80,0x00,0x00,0x00,0x1f,0xff,0xff,0x00,0x00,
	0x00,0x1f,0xff,0xff,0x80,0x00,0x00,0x00,0x1f,0xff,0xff,0x00,0x00,
	0x00,0x3f,0xfe,0xff,0xc0,0x00,0x00,0x00,0x3f,0xff,0xff,0x80,0x00,
	0x00,0x7f,0xfc,0x7f,0xe0,0x00,0x00,0x00,0x7f,0xe7,0xff,0xc0,0x00,
	0x00,0x7f,0xf8,0x3f,0xf0,0x00,0x00,0x00,0xff,0xc3,0xff,0xc0,0x00,
	0x00,0xff,0xf8,0x1f,0xf8,0x00,0x00,0x01,0xff,0x83,0xff,0xe0,0x00,
	0x00,0xff,0xf0,0x07,0xf8,0x00,0x00,0x01,0xfe,0x01,0xff,0xe0,0x00,
	0x00,0xff,0xf0,0x03,0xfc,0x00,0x00,0x03,0xfc,0x01,0xff,0xe0,0x00,
	0x01,0xff,0xe0,0x01,0xfe,0x00,0x00,0x07,0xf8,0x00,0xff,0xf0,0x00,
	0x01,0xff,0xe0,0x00,0xff,0x00,0x00,0x0f,0xf0,0x00,0xff,0xf0,0x00,
	0x01,0xff,0xc0,0x00,0x3f,0x80,0x00,0x1f,0xc0,0x00,0x7f,0xf0,0x00,
	0x01,0xff,0xc0,0x00,0x1f,0x80,0x00,0x1f,0x80,0x00,0x7f,0xf0,0x00,
	0x03,0xff,0xc0,0x00,0x0f,0xc0,0x00,0x3f,0x00,0x00,0x7f,0xf8,0x00,
	0x03,0xff,0x80,0x00,0x07,0xe0,0x00,0x7e,0x00,0x00,0x3f,0xf8,0x00,
	0x03,0xff,0x80,0x00,0x01,0xf0,0x00,0xf8,0x00,0x00,0x3f,0xf8,0x00,
	0x03,0xff,0x80,0x00,0x00,0xf8,0x01,0xf0,0x00,0x00,0x3f,0xf8,0x00,
	0x03,0xff,0x80,0x00,0x00,0x78,0x01,0xe0,0x00,0x00,0x3f,0xf8,0x00,
	0x07,0xff,0x00,0x00,0x00,0x3c,0x03,0xc0,0x00,0x00,0x3f,0xfc,0x00,
	0x07,0xff,0x00,0x00,0x00,0x0e,0x07,0x00,0x00,0x00,0x1f,0xfc,0x00,
	0x07,0xff,0x00,0x00,0x00,0x07,0x0e,0x00,0x00,0x00,0x1f,0xfc,0x00,
	0x07,0xff,0x00,0x00,0x00,0x03,0x9c,0x00,0x00,0x00,0x1f,0xfc,0x00,
	0x07,0xff,0x00,0x00,0x00,0x01,0x98,0x00,0x00,0x00,0x1f,0xfc,0x00,
	0x07,0xff,0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x1f,0xfc,0x00
};

/* per-player colors */
static const rgb_t crosshair_colors[] =
{
	MAKE_RGB(0x40,0x40,0xff),
	MAKE_RGB(0xff,0x40,0x40),
	MAKE_RGB(0x40,0xff,0x40),
	MAKE_RGB(0xff,0xff,0x40),
	MAKE_RGB(0xff,0x40,0xff),
	MAKE_RGB(0x40,0xff,0xff),
	MAKE_RGB(0xff,0xff,0xff)
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void crosshair_exit(running_machine &machine);
static void crosshair_load(running_machine *machine, int config_type, xml_data_node *parentnode);
static void crosshair_save(running_machine *machine, int config_type, xml_data_node *parentnode);

static void animate(screen_device &device, void *param, bool vblank_state);


/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/


/*-------------------------------------------------
    create_bitmap - create the rendering
    structures for the given player
-------------------------------------------------*/

static void create_bitmap(running_machine *machine, int player)
{
	int x, y;
	char filename[20];
	rgb_t color = crosshair_colors[player];

	/* if we have a bitmap for this player, kill it */
	global_free(global.bitmap[player]);

	if (global.name[player][0] != 0)
	{
		/* look for user specified file */
		sprintf(filename, "%s.png", global.name[player]);
		global.bitmap[player] = render_load_png(crosshairpath, NULL, filename, NULL, NULL);
	}
	else
	{
		/* look for default cross?.png in crsshair\game dir */
		sprintf(filename, "cross%d.png", player + 1);
		global.bitmap[player] = render_load_png(crosshairpath, machine->gamedrv->name, filename, NULL, NULL);

		/* look for default cross?.png in crsshair dir */
		if (global.bitmap[player] == NULL)
			global.bitmap[player] = render_load_png(crosshairpath, NULL, filename, NULL, NULL);
	}

	/* if that didn't work, use the built-in one */
	if (global.bitmap[player] == NULL)
	{
		/* allocate a blank bitmap to start with */
		global.bitmap[player] = global_alloc(bitmap_t(CROSSHAIR_RAW_SIZE, CROSSHAIR_RAW_SIZE, BITMAP_FORMAT_ARGB32));
		bitmap_fill(global.bitmap[player], NULL, MAKE_ARGB(0x00,0xff,0xff,0xff));

		/* extract the raw source data to it */
		for (y = 0; y < CROSSHAIR_RAW_SIZE / 2; y++)
		{
			/* assume it is mirrored vertically */
			UINT32 *dest0 = BITMAP_ADDR32(global.bitmap[player], y, 0);
			UINT32 *dest1 = BITMAP_ADDR32(global.bitmap[player], CROSSHAIR_RAW_SIZE - 1 - y, 0);

			/* extract to two rows simultaneously */
			for (x = 0; x < CROSSHAIR_RAW_SIZE; x++)
				if ((crosshair_raw_top[y * CROSSHAIR_RAW_ROWBYTES + x / 8] << (x % 8)) & 0x80)
					dest0[x] = dest1[x] = MAKE_ARGB(0xff,0x00,0x00,0x00) | color;
		}
	}

	/* create a texture to reference the bitmap */
	global.texture[player] = render_texture_alloc(render_texture_hq_scale, NULL);
	render_texture_set_bitmap(global.texture[player], global.bitmap[player], NULL, TEXFORMAT_ARGB32, NULL);
}


/*-------------------------------------------------
    crosshair_init - initialize the crosshair
    bitmaps and such
-------------------------------------------------*/

void crosshair_init(running_machine *machine)
{
	const input_port_config *port;
	const input_field_config *field;

	/* request a callback upon exiting */
	machine->add_notifier(MACHINE_NOTIFY_EXIT, crosshair_exit);

	/* clear all the globals */
	memset(&global, 0, sizeof(global));

	/* setup the default auto visibility time */
	global.auto_time = CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT;

	/* determine who needs crosshairs */
	for (port = machine->m_portlist.first(); port != NULL; port = port->next())
		for (field = port->fieldlist; field != NULL; field = field->next)
			if (field->crossaxis != CROSSHAIR_AXIS_NONE)
			{
				int player = field->player;

				assert(player < MAX_PLAYERS);

				/* mark as used and set the default visibility and mode */
				global.usage = TRUE;
				global.used[player] = TRUE;
				global.mode[player] = CROSSHAIR_VISIBILITY_DEFAULT;
				global.visible[player] = (CROSSHAIR_VISIBILITY_DEFAULT == CROSSHAIR_VISIBILITY_OFF) ? FALSE : TRUE;

				/* for now, use the main screen */
				global.screen[player] = machine->primary_screen;

				create_bitmap(machine, player);
			}

	/* register callbacks for when we load/save configurations */
	if (global.usage)
		config_register(machine, "crosshairs", crosshair_load, crosshair_save);

	/* register the animation callback */
	if (machine->primary_screen != NULL)
		machine->primary_screen->register_vblank_callback(animate, NULL);
}


/*-------------------------------------------------
    crosshair_exit - free memory allocated for
    the crosshairs
-------------------------------------------------*/

static void crosshair_exit(running_machine &machine)
{
	int player;

	/* free bitmaps and textures for each player */
	for (player = 0; player < MAX_PLAYERS; player++)
	{
		if (global.texture[player] != NULL)
			render_texture_free(global.texture[player]);
		global.texture[player] = NULL;

		global_free(global.bitmap[player]);
		global.bitmap[player] = NULL;
	}
}


/*-------------------------------------------------
    crosshair_get_usage - return TRUE
    if any crosshairs are used
-------------------------------------------------*/

int crosshair_get_usage(running_machine *machine)
{
	return global.usage;
}


/*-------------------------------------------------
    crosshair_get_user_settings - return the
    current crosshair settings for a player
    Note: auto_time is common for all players
-------------------------------------------------*/

void crosshair_get_user_settings(running_machine *machine, UINT8 player, crosshair_user_settings *settings)
{
	settings->auto_time = global.auto_time;
	settings->used = global.used[player];
	settings->mode = global.mode[player];
	strcpy(settings->name, global.name[player]);
}


/*-------------------------------------------------
    crosshair_set_user_settings - modify the
    current crosshair settings for a player
    Note: auto_time is common for all players
-------------------------------------------------*/

void crosshair_set_user_settings(running_machine *machine, UINT8 player, crosshair_user_settings *settings)
{
	int changed = FALSE;

	global.auto_time = settings->auto_time;
	global.used[player] = settings->used;
	global.mode[player] = settings->mode;

	/* set visibility as specified by mode */
	/* auto mode starts with visibility off */
	global.visible[player] = (settings->mode == CROSSHAIR_VISIBILITY_ON) ? TRUE : FALSE;

	/* update bitmap if name has changed */
	changed = strcmp(settings->name, global.name[player]);
	strcpy(global.name[player], settings->name);
	if (changed)
		create_bitmap(machine, player);
}


/*-------------------------------------------------
    animate - animates the crosshair once a frame
-------------------------------------------------*/

static void animate(screen_device &device, void *param, bool vblank_state)
{
	int player;

	/* only animate once per frame, when vblank_state is 1 */
	if (!vblank_state)
		return;

	/* increment animation counter */
	global.animation_counter += 0x08;

	/* compute a fade factor from the current animation value */
	if (global.animation_counter < 0x80)
		global.fade = 0xa0 + (0x60 * ( global.animation_counter & 0x7f) / 0x80);
	else
		global.fade = 0xa0 + (0x60 * (~global.animation_counter & 0x7f) / 0x80);

	for (player = 0; player < MAX_PLAYERS; player++)
	{
		/* read all the lightgun values */
		if (global.used[player])
			input_port_get_crosshair_position(device.machine, player, &global.x[player], &global.y[player]);

		/* auto visibility */
		if (global.mode[player] == CROSSHAIR_VISIBILITY_AUTO)
		{
			if ((global.x[player] != global.last_x[player]) || (global.y[player] != global.last_y[player]))
			{
				/* crosshair has moved, keep crosshair visible */
				global.visible[player] = TRUE;
				global.last_x[player] = global.x[player];
				global.last_y[player] = global.y[player];
				global.time[player] = 0;
			}
			else
			{
				/* see if the player has been motionless for time specified */
				/* slightly confusing formula, but the effect is: */
				/* auto_time = 0 makes the crosshair barely visible while moved */
				/* every increment in auto_time is about .2s at 60Hz */
				if (global.time[player] > global.auto_time * 12 + 2)
					/* time exceeded so turn crosshair invisible */
					global.visible[player] = FALSE;

				/* increment player visibility time */
				global.time[player]++;
			}
		}
	}
}


/*-------------------------------------------------
    crosshair_render - render the crosshairs
    for the given screen
-------------------------------------------------*/

void crosshair_render(screen_device &screen)
{
	int player;

	for (player = 0; player < MAX_PLAYERS; player++)
		/* draw if visible and the right screen */
		if (global.visible[player] &&
			((global.screen[player] == &screen) || (global.screen[player] == CROSSHAIR_SCREEN_ALL)))
		{
			/* add a quad assuming a 4:3 screen (this is not perfect) */
			render_screen_add_quad(&screen,
						global.x[player] - 0.03f, global.y[player] - 0.04f,
						global.x[player] + 0.03f, global.y[player] + 0.04f,
						MAKE_ARGB(0xc0, global.fade, global.fade, global.fade),
						global.texture[player], PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
}


/*-------------------------------------------------
    crosshair_set_screen - sets the screen(s) for a
    given player's crosshair
-------------------------------------------------*/

void crosshair_set_screen(running_machine *machine, int player, device_t *screen)
{
	global.screen[player] = screen;
}


/*-------------------------------------------------
    crosshair_load - read and apply data from the
    configuration file
-------------------------------------------------*/

static void crosshair_load(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	/* Note: crosshair_load() is only registered if croshairs are used */

	xml_data_node *crosshairnode;
	int auto_time;

	/* we only care about game files */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	/* might not have any data */
	if (parentnode == NULL)
		return;

	/* loop and get player crosshair info */
	for (crosshairnode = xml_get_sibling(parentnode->child, "crosshair"); crosshairnode; crosshairnode = xml_get_sibling(crosshairnode->next, "crosshair"))
	{
		int  player, mode;

		player = xml_get_attribute_int(crosshairnode, "player", -1);

		/* check to make sure we have a valid player */
		/* also check if the player really uses a crosshair */
		if (player >=0 && player < MAX_PLAYERS && global.used[player])
		{
			/* get, check, and store visibility mode */
			mode = xml_get_attribute_int(crosshairnode, "mode", CROSSHAIR_VISIBILITY_DEFAULT);
			if (mode >= CROSSHAIR_VISIBILITY_OFF && mode <= CROSSHAIR_VISIBILITY_AUTO)
			{
				global.mode[player] = (UINT8)mode;
				/* set visibility as specified by mode */
				/* auto mode starts with visibility off */
				global.visible[player] = (mode == CROSSHAIR_VISIBILITY_ON) ? TRUE : FALSE;
			}

			/* get and store crosshair pic name, truncate name to max length */
			strncpy(global.name[player], xml_get_attribute_string(crosshairnode, "pic", ""), CROSSHAIR_PIC_NAME_LENGTH);
			/* update bitmap */
			create_bitmap(machine, player);
		}
	}

	/* get, check, and store auto visibility time */
	crosshairnode = xml_get_sibling(parentnode->child, "autotime");
	if (crosshairnode != NULL)
	{
		auto_time = xml_get_attribute_int(crosshairnode, "val", CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT);
		if ((auto_time >= CROSSHAIR_VISIBILITY_AUTOTIME_MIN) && (auto_time <= CROSSHAIR_VISIBILITY_AUTOTIME_MAX))
			global.auto_time = (UINT8)auto_time;
	}
}


/*-------------------------------------------------
    crosshair_save -  save data to the
    configuration file
-------------------------------------------------*/

static void crosshair_save(running_machine *machine, int config_type, xml_data_node *parentnode)
{
	/* Note: crosshair_save() is only registered if crosshairs are used */

	xml_data_node *crosshairnode;
	int player;

	/* we only care about game files */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	for (player = 0; player < MAX_PLAYERS; player++)
	{
		if (global.used[player])
		{
			/* create a node */
			crosshairnode = xml_add_child(parentnode, "crosshair", NULL);

			if (crosshairnode != NULL)
			{
				int changed = FALSE;

				xml_set_attribute_int(crosshairnode, "player", player);

				if (global.visible[player] != CROSSHAIR_VISIBILITY_DEFAULT)
				{
					xml_set_attribute_int(crosshairnode, "mode", global.mode[player]);
					changed = TRUE;
				}

				/* the default graphic name is "", so only save if not */
				if (strlen(global.name[player]) > 0)
				{
					xml_set_attribute(crosshairnode, "pic", global.name[player]);
					changed = TRUE;
				}

				/* if nothing changed, kill the node */
				if (!changed)
					xml_delete_node(crosshairnode);
			}
		}
	}

	/* always store autotime so that it stays at the user value if it is needed */
	if (global.auto_time != CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT)
	{
		/* create a node */
		crosshairnode = xml_add_child(parentnode, "autotime", NULL);

		if (crosshairnode != NULL)
			xml_set_attribute_int(crosshairnode, "val", global.auto_time);
	}

}
