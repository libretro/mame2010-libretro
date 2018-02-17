/*********************************************************************

    debugcmd.c

    Debugger command interface engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "debugcmd.h"
#include "debugcmt.h"
#include "debugcon.h"
#include "debugcpu.h"
#include "express.h"
#include "debughlp.h"
#include "debugvw.h"
#include "render.h"
#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_GLOBALS		1000



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _global_entry global_entry;
struct _global_entry
{
	void *		base;
	UINT32		size;
};


typedef struct _cheat_map cheat_map;
struct _cheat_map
{
	UINT64		offset;
	UINT64		first_value;
	UINT64		previous_value;
	UINT8		state:1;
	UINT8		undo:7;
};


typedef struct _cheat_system cheat_system;
struct _cheat_system
{
	char		cpu;
	UINT64		length;
	UINT8		width;
	cheat_map *	cheatmap;
	UINT8		undo;
	UINT8		signed_cheat;
	UINT8		swapped_cheat;
};


typedef struct _cheat_region_map cheat_region_map;
struct _cheat_region_map
{
	UINT64		offset;
	UINT64		endoffset;
	const char *share;
	UINT8		disabled;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static global_entry global_array[MAX_GLOBALS];
static cheat_system cheat;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void debug_command_exit(running_machine &machine);

static UINT64 execute_min(void *globalref, void *ref, UINT32 params, const UINT64 *param);
static UINT64 execute_max(void *globalref, void *ref, UINT32 params, const UINT64 *param);
static UINT64 execute_if(void *globalref, void *ref, UINT32 params, const UINT64 *param);

static UINT64 global_get(void *globalref, void *ref);
static void global_set(void *globalref, void *ref, UINT64 value);

static void execute_help(running_machine *machine, int ref, int params, const char **param);
static void execute_print(running_machine *machine, int ref, int params, const char **param);
static void execute_printf(running_machine *machine, int ref, int params, const char **param);
static void execute_logerror(running_machine *machine, int ref, int params, const char **param);
static void execute_tracelog(running_machine *machine, int ref, int params, const char **param);
static void execute_quit(running_machine *machine, int ref, int params, const char **param);
static void execute_do(running_machine *machine, int ref, int params, const char **param);
static void execute_step(running_machine *machine, int ref, int params, const char **param);
static void execute_over(running_machine *machine, int ref, int params, const char **param);
static void execute_out(running_machine *machine, int ref, int params, const char **param);
static void execute_go(running_machine *machine, int ref, int params, const char **param);
static void execute_go_vblank(running_machine *machine, int ref, int params, const char **param);
static void execute_go_interrupt(running_machine *machine, int ref, int params, const char **param);
static void execute_go_time(running_machine *machine, int ref, int params, const char *param[]);
static void execute_focus(running_machine *machine, int ref, int params, const char **param);
static void execute_ignore(running_machine *machine, int ref, int params, const char **param);
static void execute_observe(running_machine *machine, int ref, int params, const char **param);
static void execute_next(running_machine *machine, int ref, int params, const char **param);
static void execute_comment(running_machine *machine, int ref, int params, const char **param);
static void execute_comment_del(running_machine *machine, int ref, int params, const char **param);
static void execute_comment_save(running_machine *machine, int ref, int params, const char **param);
static void execute_bpset(running_machine *machine, int ref, int params, const char **param);
static void execute_bpclear(running_machine *machine, int ref, int params, const char **param);
static void execute_bpdisenable(running_machine *machine, int ref, int params, const char **param);
static void execute_bplist(running_machine *machine, int ref, int params, const char **param);
static void execute_wpset(running_machine *machine, int ref, int params, const char **param);
static void execute_wpclear(running_machine *machine, int ref, int params, const char **param);
static void execute_wpdisenable(running_machine *machine, int ref, int params, const char **param);
static void execute_wplist(running_machine *machine, int ref, int params, const char **param);
static void execute_hotspot(running_machine *machine, int ref, int params, const char **param);
static void execute_save(running_machine *machine, int ref, int params, const char **param);
static void execute_dump(running_machine *machine, int ref, int params, const char **param);
static void execute_cheatinit(running_machine *machine, int ref, int params, const char **param);
static void execute_cheatnext(running_machine *machine, int ref, int params, const char **param);
static void execute_cheatlist(running_machine *machine, int ref, int params, const char **param);
static void execute_cheatundo(running_machine *machine, int ref, int params, const char **param);
static void execute_dasm(running_machine *machine, int ref, int params, const char **param);
static void execute_find(running_machine *machine, int ref, int params, const char **param);
static void execute_trace(running_machine *machine, int ref, int params, const char **param);
static void execute_traceover(running_machine *machine, int ref, int params, const char **param);
static void execute_traceflush(running_machine *machine, int ref, int params, const char **param);
static void execute_history(running_machine *machine, int ref, int params, const char **param);
static void execute_snap(running_machine *machine, int ref, int params, const char **param);
static void execute_source(running_machine *machine, int ref, int params, const char **param);
static void execute_map(running_machine *machine, int ref, int params, const char **param);
static void execute_memdump(running_machine *machine, int ref, int params, const char **param);
static void execute_symlist(running_machine *machine, int ref, int params, const char **param);
static void execute_softreset(running_machine *machine, int ref, int params, const char **param);
static void execute_hardreset(running_machine *machine, int ref, int params, const char **param);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    cheat_address_is_valid - return TRUE if the
    given address is valid for cheating
-------------------------------------------------*/

INLINE int cheat_address_is_valid(const address_space *space, offs_t address)
{
	return debug_cpu_translate(space, TRANSLATE_READ, &address) && (memory_get_write_ptr(space, address) != NULL);
}


/*-------------------------------------------------
    cheat_sign_extend - sign-extend a value to
    the current cheat width, if signed
-------------------------------------------------*/

INLINE UINT64 cheat_sign_extend(const cheat_system *cheatsys, UINT64 value)
{
	if (cheatsys->signed_cheat)
	{
		switch (cheatsys->width)
		{
			case 1:	value = (INT8)value;	break;
			case 2:	value = (INT16)value;	break;
			case 4:	value = (INT32)value;	break;
		}
	}
	return value;
}
/*-------------------------------------------------
    cheat_byte_swap - swap a value
-------------------------------------------------*/

INLINE UINT64 cheat_byte_swap(const cheat_system *cheatsys, UINT64 value)
{
	if (cheatsys->swapped_cheat)
	{
		switch (cheatsys->width)
		{
			case 2:	value = ((value >> 8) & 0x00ff) | ((value << 8) & 0xff00);	break;
			case 4:	value = ((value >> 24) & 0x000000ff) | ((value >> 8) & 0x0000ff00) | ((value << 8) & 0x00ff0000) | ((value << 24) & 0xff000000);	break;
			case 8:	value = ((value >> 56) & U64(0x00000000000000ff)) | ((value >> 40) & U64(0x000000000000ff00)) | ((value >> 24) & U64(0x0000000000ff0000)) | ((value >> 8) & U64(0x00000000ff000000)) |
							((value << 8) & U64(0x000000ff00000000)) | ((value << 24) & U64(0x0000ff0000000000)) | ((value << 40) & U64(0x00ff000000000000)) | ((value << 56) & U64(0xff00000000000000));	break;
		}
	}
	return value;
}

/*-------------------------------------------------
    cheat_read_extended - read a value from memory
    in the given address space, sign-extending
    and swapping if necessary
-------------------------------------------------*/

INLINE UINT64 cheat_read_extended(const cheat_system *cheatsys, const address_space *space, offs_t address)
{
	return cheat_sign_extend(cheatsys, cheat_byte_swap(cheatsys, debug_read_memory(space, address, cheatsys->width, TRUE)));
}



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    debug_command_init - initializes the command
    system
-------------------------------------------------*/

void debug_command_init(running_machine *machine)
{
	symbol_table *symtable = debug_cpu_get_global_symtable(machine);
	const char *name;
	int itemnum;

	/* add a few simple global functions */
	symtable_add_function(symtable, "min", NULL, 2, 2, execute_min);
	symtable_add_function(symtable, "max", NULL, 2, 2, execute_max);
	symtable_add_function(symtable, "if", NULL, 3, 3, execute_if);

	/* add all single-entry save state globals */
	for (itemnum = 0; itemnum < MAX_GLOBALS; itemnum++)
	{
		UINT32 valsize, valcount;
		void *base;

		/* stop when we run out of items */
		name = state_save_get_indexed_item(machine, itemnum, &base, &valsize, &valcount);
		if (name == NULL)
			break;

		/* if this is a single-entry global, add it */
		if (valcount == 1 && strstr(name, "/globals/"))
		{
			char symname[100];
			sprintf(symname, ".%s", strrchr(name, '/') + 1);
			global_array[itemnum].base = base;
			global_array[itemnum].size = valsize;
			symtable_add_register(symtable, symname, &global_array, global_get, global_set);
		}
	}

	/* add all the commands */
	debug_console_register_command(machine, "help",      CMDFLAG_NONE, 0, 0, 1, execute_help);
	debug_console_register_command(machine, "print",     CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_print);
	debug_console_register_command(machine, "printf",    CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_printf);
	debug_console_register_command(machine, "logerror",  CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_logerror);
	debug_console_register_command(machine, "tracelog",  CMDFLAG_NONE, 0, 1, MAX_COMMAND_PARAMS, execute_tracelog);
	debug_console_register_command(machine, "quit",      CMDFLAG_NONE, 0, 0, 0, execute_quit);
	debug_console_register_command(machine, "do",        CMDFLAG_NONE, 0, 1, 1, execute_do);
	debug_console_register_command(machine, "step",      CMDFLAG_NONE, 0, 0, 1, execute_step);
	debug_console_register_command(machine, "s",         CMDFLAG_NONE, 0, 0, 1, execute_step);
	debug_console_register_command(machine, "over",      CMDFLAG_NONE, 0, 0, 1, execute_over);
	debug_console_register_command(machine, "o",         CMDFLAG_NONE, 0, 0, 1, execute_over);
	debug_console_register_command(machine, "out" ,      CMDFLAG_NONE, 0, 0, 0, execute_out);
	debug_console_register_command(machine, "go",        CMDFLAG_NONE, 0, 0, 1, execute_go);
	debug_console_register_command(machine, "g",         CMDFLAG_NONE, 0, 0, 1, execute_go);
	debug_console_register_command(machine, "gvblank",   CMDFLAG_NONE, 0, 0, 0, execute_go_vblank);
	debug_console_register_command(machine, "gv",        CMDFLAG_NONE, 0, 0, 0, execute_go_vblank);
	debug_console_register_command(machine, "gint",      CMDFLAG_NONE, 0, 0, 1, execute_go_interrupt);
	debug_console_register_command(machine, "gi",        CMDFLAG_NONE, 0, 0, 1, execute_go_interrupt);
	debug_console_register_command(machine, "gtime",     CMDFLAG_NONE, 0, 0, 1, execute_go_time);
	debug_console_register_command(machine, "gt",        CMDFLAG_NONE, 0, 0, 1, execute_go_time);
	debug_console_register_command(machine, "next",      CMDFLAG_NONE, 0, 0, 0, execute_next);
	debug_console_register_command(machine, "n",         CMDFLAG_NONE, 0, 0, 0, execute_next);
	debug_console_register_command(machine, "focus",     CMDFLAG_NONE, 0, 1, 1, execute_focus);
	debug_console_register_command(machine, "ignore",    CMDFLAG_NONE, 0, 0, MAX_COMMAND_PARAMS, execute_ignore);
	debug_console_register_command(machine, "observe",   CMDFLAG_NONE, 0, 0, MAX_COMMAND_PARAMS, execute_observe);

	debug_console_register_command(machine, "comadd",	CMDFLAG_NONE, 0, 1, 2, execute_comment);
	debug_console_register_command(machine, "//",        CMDFLAG_NONE, 0, 1, 2, execute_comment);
	debug_console_register_command(machine, "comdelete",	CMDFLAG_NONE, 0, 1, 1, execute_comment_del);
	debug_console_register_command(machine, "comsave",	CMDFLAG_NONE, 0, 0, 0, execute_comment_save);

	debug_console_register_command(machine, "bpset",     CMDFLAG_NONE, 0, 1, 3, execute_bpset);
	debug_console_register_command(machine, "bp",        CMDFLAG_NONE, 0, 1, 3, execute_bpset);
	debug_console_register_command(machine, "bpclear",   CMDFLAG_NONE, 0, 0, 1, execute_bpclear);
	debug_console_register_command(machine, "bpdisable", CMDFLAG_NONE, 0, 0, 1, execute_bpdisenable);
	debug_console_register_command(machine, "bpenable",  CMDFLAG_NONE, 1, 0, 1, execute_bpdisenable);
	debug_console_register_command(machine, "bplist",    CMDFLAG_NONE, 0, 0, 0, execute_bplist);

	debug_console_register_command(machine, "wpset",     CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wp",        CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpdset",    CMDFLAG_NONE, ADDRESS_SPACE_DATA, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpd",       CMDFLAG_NONE, ADDRESS_SPACE_DATA, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpiset",    CMDFLAG_NONE, ADDRESS_SPACE_IO, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpi",       CMDFLAG_NONE, ADDRESS_SPACE_IO, 3, 5, execute_wpset);
	debug_console_register_command(machine, "wpclear",   CMDFLAG_NONE, 0, 0, 1, execute_wpclear);
	debug_console_register_command(machine, "wpdisable", CMDFLAG_NONE, 0, 0, 1, execute_wpdisenable);
	debug_console_register_command(machine, "wpenable",  CMDFLAG_NONE, 1, 0, 1, execute_wpdisenable);
	debug_console_register_command(machine, "wplist",    CMDFLAG_NONE, 0, 0, 0, execute_wplist);

	debug_console_register_command(machine, "hotspot",   CMDFLAG_NONE, 0, 0, 3, execute_hotspot);

	debug_console_register_command(machine, "save",      CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 3, 4, execute_save);
	debug_console_register_command(machine, "saved",     CMDFLAG_NONE, ADDRESS_SPACE_DATA, 3, 4, execute_save);
	debug_console_register_command(machine, "savei",     CMDFLAG_NONE, ADDRESS_SPACE_IO, 3, 4, execute_save);

	debug_console_register_command(machine, "dump",      CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 3, 6, execute_dump);
	debug_console_register_command(machine, "dumpd",     CMDFLAG_NONE, ADDRESS_SPACE_DATA, 3, 6, execute_dump);
	debug_console_register_command(machine, "dumpi",     CMDFLAG_NONE, ADDRESS_SPACE_IO, 3, 6, execute_dump);

	debug_console_register_command(machine, "cheatinit", CMDFLAG_NONE, 0, 0, 4, execute_cheatinit);
	debug_console_register_command(machine, "ci",        CMDFLAG_NONE, 0, 0, 4, execute_cheatinit);

	debug_console_register_command(machine, "cheatrange",CMDFLAG_NONE, 1, 2, 2, execute_cheatinit);
	debug_console_register_command(machine, "cr",        CMDFLAG_NONE, 1, 2, 2, execute_cheatinit);

	debug_console_register_command(machine, "cheatnext", CMDFLAG_NONE, 0, 1, 2, execute_cheatnext);
	debug_console_register_command(machine, "cn",        CMDFLAG_NONE, 0, 1, 2, execute_cheatnext);
	debug_console_register_command(machine, "cheatnextf",CMDFLAG_NONE, 1, 1, 2, execute_cheatnext);
	debug_console_register_command(machine, "cnf",       CMDFLAG_NONE, 1, 1, 2, execute_cheatnext);

	debug_console_register_command(machine, "cheatlist", CMDFLAG_NONE, 0, 0, 1, execute_cheatlist);
	debug_console_register_command(machine, "cl",        CMDFLAG_NONE, 0, 0, 1, execute_cheatlist);

	debug_console_register_command(machine, "cheatundo", CMDFLAG_NONE, 0, 0, 0, execute_cheatundo);
	debug_console_register_command(machine, "cu",        CMDFLAG_NONE, 0, 0, 0, execute_cheatundo);

	debug_console_register_command(machine, "f",         CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_PROGRAM, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "find",      CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_PROGRAM, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "fd",        CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_DATA, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "findd",     CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_DATA, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "fi",        CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_IO, 3, MAX_COMMAND_PARAMS, execute_find);
	debug_console_register_command(machine, "findi",     CMDFLAG_KEEP_QUOTES, ADDRESS_SPACE_IO, 3, MAX_COMMAND_PARAMS, execute_find);

	debug_console_register_command(machine, "dasm",      CMDFLAG_NONE, 0, 3, 5, execute_dasm);

	debug_console_register_command(machine, "trace",     CMDFLAG_NONE, 0, 1, 3, execute_trace);
	debug_console_register_command(machine, "traceover", CMDFLAG_NONE, 0, 1, 3, execute_traceover);
	debug_console_register_command(machine, "traceflush",CMDFLAG_NONE, 0, 0, 0, execute_traceflush);

	debug_console_register_command(machine, "history",   CMDFLAG_NONE, 0, 0, 2, execute_history);

	debug_console_register_command(machine, "snap",      CMDFLAG_NONE, 0, 0, 1, execute_snap);

	debug_console_register_command(machine, "source",    CMDFLAG_NONE, 0, 1, 1, execute_source);

	debug_console_register_command(machine, "map",		CMDFLAG_NONE, ADDRESS_SPACE_PROGRAM, 1, 1, execute_map);
	debug_console_register_command(machine, "mapd",		CMDFLAG_NONE, ADDRESS_SPACE_DATA, 1, 1, execute_map);
	debug_console_register_command(machine, "mapi",		CMDFLAG_NONE, ADDRESS_SPACE_IO, 1, 1, execute_map);
	debug_console_register_command(machine, "memdump",	CMDFLAG_NONE, 0, 0, 1, execute_memdump);

	debug_console_register_command(machine, "symlist",	CMDFLAG_NONE, 0, 0, 1, execute_symlist);

	debug_console_register_command(machine, "softreset",	CMDFLAG_NONE, 0, 0, 1, execute_softreset);
	debug_console_register_command(machine, "hardreset",	CMDFLAG_NONE, 0, 0, 1, execute_hardreset);

	/* ask all the devices if they would like to register functions or symbols */
	machine->m_devicelist.debug_setup_all();

	machine->add_notifier(MACHINE_NOTIFY_EXIT, debug_command_exit);

	/* set up the initial debugscript if specified */
	name = options_get_string(machine->options(), OPTION_DEBUGSCRIPT);
	if (name[0] != 0)
		debug_cpu_source_script(machine, name);
}


/*-------------------------------------------------
    debug_command_exit - exit-time cleanup
-------------------------------------------------*/

static void debug_command_exit(running_machine &machine)
{
	/* turn off all traces */
	for (device_t *device = machine.m_devicelist.first(); device != NULL; device = device->next())
		device->debug()->trace(NULL, 0, NULL);

	if (cheat.length)
		auto_free(&machine, cheat.cheatmap);
}



/***************************************************************************
    GLOBAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    execute_min - return the minimum of two values
-------------------------------------------------*/

static UINT64 execute_min(void *globalref, void *ref, UINT32 params, const UINT64 *param)
{
	return (param[0] < param[1]) ? param[0] : param[1];
}


/*-------------------------------------------------
    execute_max - return the maximum of two values
-------------------------------------------------*/

static UINT64 execute_max(void *globalref, void *ref, UINT32 params, const UINT64 *param)
{
	return (param[0] > param[1]) ? param[0] : param[1];
}


/*-------------------------------------------------
    execute_if - if (a) return b; else return c;
-------------------------------------------------*/

static UINT64 execute_if(void *globalref, void *ref, UINT32 params, const UINT64 *param)
{
	return param[0] ? param[1] : param[2];
}



/***************************************************************************
    GLOBAL ACCESSORS
***************************************************************************/

/*-------------------------------------------------
    global_get - symbol table getter for globals
-------------------------------------------------*/

static UINT64 global_get(void *globalref, void *ref)
{
	global_entry *global = (global_entry *)ref;
	switch (global->size)
	{
		case 1:		return *(UINT8 *)global->base;
		case 2:		return *(UINT16 *)global->base;
		case 4:		return *(UINT32 *)global->base;
		case 8:		return *(UINT64 *)global->base;
	}
	return ~0;
}


/*-------------------------------------------------
    global_set - symbol table setter for globals
-------------------------------------------------*/

static void global_set(void *globalref, void *ref, UINT64 value)
{
	global_entry *global = (global_entry *)ref;
	switch (global->size)
	{
		case 1:		*(UINT8 *)global->base = value;	break;
		case 2:		*(UINT16 *)global->base = value;	break;
		case 4:		*(UINT32 *)global->base = value;	break;
		case 8:		*(UINT64 *)global->base = value;	break;
	}
}



/***************************************************************************
    PARAMETER VALIDATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    debug_command_parameter_number - validates a
    number parameter
-------------------------------------------------*/

int debug_command_parameter_number(running_machine *machine, const char *param, UINT64 *result)
{
	EXPRERR err;

	/* NULL parameter does nothing and returns no error */
	if (param == NULL)
		return TRUE;

	/* evaluate the expression; success if no error */
	err = expression_evaluate(param, debug_cpu_get_visible_symtable(machine), &debug_expression_callbacks, machine, result);
	if (err == EXPRERR_NONE)
		return TRUE;

	/* print an error pointing to the character that caused it */
	debug_console_printf(machine, "Error in expression: %s\n", param);
	debug_console_printf(machine, "                     %*s^", EXPRERR_ERROR_OFFSET(err), "");
	debug_console_printf(machine, "%s\n", exprerr_to_string(err));
	return FALSE;
}


/*-------------------------------------------------
    debug_command_parameter_cpu - validates a
    parameter as a cpu
-------------------------------------------------*/

int debug_command_parameter_cpu(running_machine *machine, const char *param, device_t **result)
{
	UINT64 cpunum;
	EXPRERR err;

	/* if no parameter, use the visible CPU */
	if (param == NULL)
	{
		*result = debug_cpu_get_visible_cpu(machine);
		if (*result == NULL)
		{
			debug_console_printf(machine, "No valid CPU is currently selected\n");
			return FALSE;
		}
		return TRUE;
	}

	/* first look for a tag match */
	*result = machine->device(param);
	if (*result != NULL)
		return TRUE;

	/* then evaluate as an expression; on an error assume it was a tag */
	err = expression_evaluate(param, debug_cpu_get_visible_symtable(machine), &debug_expression_callbacks, machine, &cpunum);
	if (err != EXPRERR_NONE)
	{
		debug_console_printf(machine, "Unable to find CPU '%s'\n", param);
		return FALSE;
	}

	/* if we got a valid one, return */
	device_execute_interface *exec = NULL;
	for (bool gotone = machine->m_devicelist.first(exec); gotone; gotone = exec->next(exec))
		if (cpunum-- == 0)
		{
			*result = &exec->device();
			return TRUE;
		}

	/* if out of range, complain */
	debug_console_printf(machine, "Invalid CPU index %d\n", (UINT32)cpunum);
	return FALSE;
}


/*-------------------------------------------------
    debug_command_parameter_cpu_space - validates
    a parameter as a cpu and retrieves the given
    address space
-------------------------------------------------*/

int debug_command_parameter_cpu_space(running_machine *machine, const char *param, int spacenum, const address_space **result)
{
	device_t *cpu;

	/* first do the standard CPU thing */
	if (!debug_command_parameter_cpu(machine, param, &cpu))
		return FALSE;

	/* fetch the space pointer */
	*result = cpu_get_address_space(cpu, spacenum);
	if (*result == NULL)
	{
		debug_console_printf(machine, "No matching memory space found for CPU '%s'\n", cpu->tag());
		return FALSE;
	}
	return TRUE;
}


/*-------------------------------------------------
    debug_command_parameter_expression - validates
    an expression parameter
-------------------------------------------------*/

static int debug_command_parameter_expression(running_machine *machine, const char *param, parsed_expression **result)
{
	EXPRERR err;

	/* NULL parameter does nothing and returns no error */
	if (param == NULL)
	{
		*result = NULL;
		return TRUE;
	}

	/* parse the expression; success if no error */
	err = expression_parse(param, debug_cpu_get_visible_symtable(machine), &debug_expression_callbacks, machine, result);
	if (err == EXPRERR_NONE)
		return TRUE;

	/* output an error */
	debug_console_printf(machine, "Error in expression: %s\n", param);
	debug_console_printf(machine, "                     %*s^", EXPRERR_ERROR_OFFSET(err), "");
	debug_console_printf(machine, "%s\n", exprerr_to_string(err));
	return FALSE;
}


/*-------------------------------------------------
    debug_command_parameter_command - validates a
    command parameter
-------------------------------------------------*/

static int debug_command_parameter_command(running_machine *machine, const char *param)
{
	CMDERR err;

	/* NULL parameter does nothing and returns no error */
	if (param == NULL)
		return TRUE;

	/* validate the comment; success if no error */
	err = debug_console_validate_command(machine, param);
	if (err == CMDERR_NONE)
		return TRUE;

	/* output an error */
	debug_console_printf(machine, "Error in command: %s\n", param);
	debug_console_printf(machine, "                  %*s^", CMDERR_ERROR_OFFSET(err), "");
	debug_console_printf(machine, "%s\n", debug_cmderr_to_string(err));
	return 0;
}



/***************************************************************************
    COMMAND HELPERS
***************************************************************************/

/*-------------------------------------------------
    execute_help - execute the help command
-------------------------------------------------*/

static void execute_help(running_machine *machine, int ref, int params, const char *param[])
{
	if (params == 0)
		debug_console_printf_wrap(machine, 80, "%s\n", debug_get_help(""));
	else
		debug_console_printf_wrap(machine, 80, "%s\n", debug_get_help(param[0]));
}


/*-------------------------------------------------
    execute_print - execute the print command
-------------------------------------------------*/

static void execute_print(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 values[MAX_COMMAND_PARAMS];
	int i;

	/* validate the other parameters */
	for (i = 0; i < params; i++)
		if (!debug_command_parameter_number(machine, param[i], &values[i]))
			return;

	/* then print each one */
	for (i = 0; i < params; i++)
		debug_console_printf(machine, "%s", core_i64_hex_format(values[i], 0));
	debug_console_printf(machine, "\n");
}


/*-------------------------------------------------
    mini_printf - safe printf to a buffer
-------------------------------------------------*/

static int mini_printf(running_machine *machine, char *buffer, const char *format, int params, UINT64 *param)
{
	const char *f = format;
	char *p = buffer;

	/* parse the string looking for % signs */
	for (;;)
	{
		char c = *f++;
		if (!c) break;

		/* escape sequences */
		if (c == '\\')
		{
			c = *f++;
			if (!c) break;
			switch (c)
			{
				case '\\':	*p++ = c;		break;
				case 'n':	*p++ = '\n';	break;
				default:					break;
			}
			continue;
		}

		/* formatting */
		else if (c == '%')
		{
			int width = 0;
			int zerofill = 0;

			/* parse out the width */
			for (;;)
			{
				c = *f++;
				if (!c || c < '0' || c > '9') break;
				if (c == '0' && width == 0)
					zerofill = 1;
				width = width * 10 + (c - '0');
			}
			if (!c) break;

			/* get the format */
			switch (c)
			{
				case '%':
					*p++ = c;
					break;

				case 'X':
				case 'x':
					if (params == 0)
					{
						debug_console_printf(machine, "Not enough parameters for format!\n");
						return 0;
					}
					if ((UINT32)(*param >> 32) != 0)
						p += sprintf(p, zerofill ? "%0*X" : "%*X", (width <= 8) ? 1 : width - 8, (UINT32)(*param >> 32));
					else if (width > 8)
						p += sprintf(p, zerofill ? "%0*X" : "%*X", width - 8, 0);
					p += sprintf(p, zerofill ? "%0*X" : "%*X", (width < 8) ? width : 8, (UINT32)*param);
					param++;
					params--;
					break;

				case 'D':
				case 'd':
					if (params == 0)
					{
						debug_console_printf(machine, "Not enough parameters for format!\n");
						return 0;
					}
					p += sprintf(p, zerofill ? "%0*d" : "%*d", width, (UINT32)*param);
					param++;
					params--;
					break;
			}
		}

		/* normal stuff */
		else
			*p++ = c;
	}

	/* NULL-terminate and exit */
	*p = 0;
	return 1;
}


/*-------------------------------------------------
    execute_printf - execute the printf command
-------------------------------------------------*/

static void execute_printf(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 values[MAX_COMMAND_PARAMS];
	char buffer[1024];
	int i;

	/* validate the other parameters */
	for (i = 1; i < params; i++)
		if (!debug_command_parameter_number(machine, param[i], &values[i]))
			return;

	/* then do a printf */
	if (mini_printf(machine, buffer, param[0], params - 1, &values[1]))
		debug_console_printf(machine, "%s\n", buffer);
}


/*-------------------------------------------------
    execute_logerror - execute the logerror command
-------------------------------------------------*/

static void execute_logerror(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 values[MAX_COMMAND_PARAMS];
	char buffer[1024];
	int i;

	/* validate the other parameters */
	for (i = 1; i < params; i++)
		if (!debug_command_parameter_number(machine, param[i], &values[i]))
			return;

	/* then do a printf */
	if (mini_printf(machine, buffer, param[0], params - 1, &values[1]))
		logerror("%s", buffer);
}


/*-------------------------------------------------
    execute_tracelog - execute the tracelog command
-------------------------------------------------*/

static void execute_tracelog(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 values[MAX_COMMAND_PARAMS];
	char buffer[1024];
	int i;

	/* validate the other parameters */
	for (i = 1; i < params; i++)
		if (!debug_command_parameter_number(machine, param[i], &values[i]))
			return;

	/* then do a printf */
	if (mini_printf(machine, buffer, param[0], params - 1, &values[1]))
		debug_cpu_get_visible_cpu(machine)->debug()->trace_printf("%s", buffer);
}


/*-------------------------------------------------
    execute_quit - execute the quit command
-------------------------------------------------*/

static void execute_quit(running_machine *machine, int ref, int params, const char *param[])
{
	mame_printf_error("Exited via the debugger\n");
	machine->schedule_exit();
}


/*-------------------------------------------------
    execute_do - execute the do command
-------------------------------------------------*/

static void execute_do(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 dummy;
	debug_command_parameter_number(machine, param[0], &dummy);
}


/*-------------------------------------------------
    execute_step - execute the step command
-------------------------------------------------*/

static void execute_step(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 steps = 1;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &steps))
		return;

	debug_cpu_get_visible_cpu(machine)->debug()->single_step(steps);
}


/*-------------------------------------------------
    execute_over - execute the over command
-------------------------------------------------*/

static void execute_over(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 steps = 1;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &steps))
		return;

	debug_cpu_get_visible_cpu(machine)->debug()->single_step_over(steps);
}


/*-------------------------------------------------
    execute_out - execute the out command
-------------------------------------------------*/

static void execute_out(running_machine *machine, int ref, int params, const char *param[])
{
	debug_cpu_get_visible_cpu(machine)->debug()->single_step_out();
}


/*-------------------------------------------------
    execute_go - execute the go command
-------------------------------------------------*/

static void execute_go(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 addr = ~0;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &addr))
		return;

	debug_cpu_get_visible_cpu(machine)->debug()->go(addr);
}


/*-------------------------------------------------
    execute_go_vblank - execute the govblank
    command
-------------------------------------------------*/

static void execute_go_vblank(running_machine *machine, int ref, int params, const char *param[])
{
	debug_cpu_get_visible_cpu(machine)->debug()->go_vblank();
}


/*-------------------------------------------------
    execute_go_interrupt - execute the goint command
-------------------------------------------------*/

static void execute_go_interrupt(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 irqline = -1;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &irqline))
		return;

	debug_cpu_get_visible_cpu(machine)->debug()->go_interrupt(irqline);
}


/*-------------------------------------------------
    execute_go_time - execute the gtime command
-------------------------------------------------*/

static void execute_go_time(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 milliseconds = -1;

	/* if we have a parameter, use it instead */
	if (!debug_command_parameter_number(machine, param[0], &milliseconds))
		return;

	debug_cpu_get_visible_cpu(machine)->debug()->go_milliseconds(milliseconds);
}


/*-------------------------------------------------
    execute_next - execute the next command
-------------------------------------------------*/

static void execute_next(running_machine *machine, int ref, int params, const char *param[])
{
	debug_cpu_get_visible_cpu(machine)->debug()->go_next_device();
}


/*-------------------------------------------------
    execute_focus - execute the focus command
-------------------------------------------------*/

static void execute_focus(running_machine *machine, int ref, int params, const char *param[])
{
	/* validate params */
	device_t *cpu;
	if (!debug_command_parameter_cpu(machine, param[0], &cpu))
		return;

	/* first clear the ignore flag on the focused CPU */
	cpu->debug()->ignore(false);

	/* then loop over CPUs and set the ignore flags on all other CPUs */
	device_execute_interface *exec = NULL;
	for (bool gotone = machine->m_devicelist.first(exec); gotone; gotone = exec->next(exec))
		if (&exec->device() != cpu)
			exec->device().debug()->ignore(true);
	debug_console_printf(machine, "Now focused on CPU '%s'\n", cpu->tag());
}


/*-------------------------------------------------
    execute_ignore - execute the ignore command
-------------------------------------------------*/

static void execute_ignore(running_machine *machine, int ref, int params, const char *param[])
{
	/* if there are no parameters, dump the ignore list */
	if (params == 0)
	{
		astring buffer;

		/* loop over all executable devices */
		device_execute_interface *exec = NULL;
		for (bool gotone = machine->m_devicelist.first(exec); gotone; gotone = exec->next(exec))

			/* build up a comma-separated list */
			if (!exec->device().debug()->observing())
			{
				if (buffer.len() == 0)
					buffer.printf("Currently ignoring device '%s'", exec->device().tag());
				else
					buffer.catprintf(", '%s'", exec->device().tag());
			}

		/* special message for none */
		if (buffer.len() == 0)
			buffer.printf("Not currently ignoring any devices");
		debug_console_printf(machine, "%s\n", buffer.cstr());
	}

	/* otherwise clear the ignore flag on all requested CPUs */
	else
	{
		device_t *devicelist[MAX_COMMAND_PARAMS];

		/* validate parameters */
		for (int paramnum = 0; paramnum < params; paramnum++)
			if (!debug_command_parameter_cpu(machine, param[paramnum], &devicelist[paramnum]))
				return;

		/* set the ignore flags */
		for (int paramnum = 0; paramnum < params; paramnum++)
		{
			/* make sure this isn't the last live CPU */
			device_execute_interface *exec = NULL;
			bool gotone;
			for (gotone = machine->m_devicelist.first(exec); gotone; gotone = exec->next(exec))
				if (&exec->device() != devicelist[paramnum] && exec->device().debug()->observing())
					break;
			if (!gotone)
			{
				debug_console_printf(machine, "Can't ignore all devices!\n");
				return;
			}

			devicelist[paramnum]->debug()->ignore(true);
			debug_console_printf(machine, "Now ignoring device '%s'\n", devicelist[paramnum]->tag());
		}
	}
}


/*-------------------------------------------------
    execute_observe - execute the observe command
-------------------------------------------------*/

static void execute_observe(running_machine *machine, int ref, int params, const char *param[])
{
	/* if there are no parameters, dump the ignore list */
	if (params == 0)
	{
		astring buffer;

		/* loop over all executable devices */
		device_execute_interface *exec = NULL;
		for (bool gotone = machine->m_devicelist.first(exec); gotone; gotone = exec->next(exec))

			/* build up a comma-separated list */
			if (exec->device().debug()->observing())
			{
				if (buffer.len() == 0)
					buffer.printf("Currently observing CPU '%s'", exec->device().tag());
				else
					buffer.catprintf(", '%s'", exec->device().tag());
			}

		/* special message for none */
		if (buffer.len() == 0)
			buffer.printf("Not currently observing any devices");
		debug_console_printf(machine, "%s\n", buffer.cstr());
	}

	/* otherwise set the ignore flag on all requested CPUs */
	else
	{
		device_t *devicelist[MAX_COMMAND_PARAMS];

		/* validate parameters */
		for (int paramnum = 0; paramnum < params; paramnum++)
			if (!debug_command_parameter_cpu(machine, param[paramnum], &devicelist[paramnum]))
				return;

		/* clear the ignore flags */
		for (int paramnum = 0; paramnum < params; paramnum++)
		{
			devicelist[paramnum]->debug()->ignore(false);
			debug_console_printf(machine, "Now observing device '%s'\n", devicelist[paramnum]->tag());
		}
	}
}


/*-------------------------------------------------
    execute_comment - add a comment to a line
-------------------------------------------------*/

static void execute_comment(running_machine *machine, int ref, int params, const char *param[])
{
	device_t *cpu;
	UINT64 address;

	/* param 1 is the address for the comment */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* CPU parameter is implicit */
	if (!debug_command_parameter_cpu(machine, NULL, &cpu))
		return;

	/* make sure param 2 exists */
	if (strlen(param[1]) == 0)
	{
		debug_console_printf(machine, "Error : comment text empty\n");
		return;
	}

	/* Now try adding the comment */
	debug_comment_add(cpu, address, param[1], 0x00ff0000, debug_comment_get_opcode_crc32(cpu, address));
	cpu->machine->m_debug_view->update_all(DVT_DISASSEMBLY);
}


/*------------------------------------------------------
    execute_comment_del - remove a comment from an addr
--------------------------------------------------------*/

static void execute_comment_del(running_machine *machine, int ref, int params, const char *param[])
{
	device_t *cpu;
	UINT64 address;

	/* param 1 can either be a command or the address for the comment */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* CPU parameter is implicit */
	if (!debug_command_parameter_cpu(machine, NULL, &cpu))
		return;

	/* If it's a number, it must be an address */
	/* The bankoff and cbn will be pulled from what's currently active */
	debug_comment_remove(cpu, address, debug_comment_get_opcode_crc32(cpu, address));
	cpu->machine->m_debug_view->update_all(DVT_DISASSEMBLY);
}


/*-------------------------------------------------
    execute_comment - add a comment to a line
-------------------------------------------------*/

static void execute_comment_save(running_machine *machine, int ref, int params, const char *param[])
{
	if (debug_comment_save(machine))
		debug_console_printf(machine, "Comments successfully saved\n");
}


/*-------------------------------------------------
    execute_bpset - execute the breakpoint set
    command
-------------------------------------------------*/

static void execute_bpset(running_machine *machine, int ref, int params, const char *param[])
{
	parsed_expression *condition = NULL;
	device_t *cpu;
	const char *action = NULL;
	UINT64 address;
	int bpnum;

	/* param 1 is the address */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* param 2 is the condition */
	if (!debug_command_parameter_expression(machine, param[1], &condition))
		return;

	/* param 3 is the action */
	if (!debug_command_parameter_command(machine, action = param[2]))
		return;

	/* CPU is implicit */
	if (!debug_command_parameter_cpu(machine, NULL, &cpu))
		return;

	/* set the breakpoint */
	bpnum = cpu->debug()->breakpoint_set(address, condition, action);
	debug_console_printf(machine, "Breakpoint %X set\n", bpnum);
}


/*-------------------------------------------------
    execute_bpclear - execute the breakpoint
    clear command
-------------------------------------------------*/

static void execute_bpclear(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 bpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
			device->debug()->breakpoint_clear_all();
		debug_console_printf(machine, "Cleared all breakpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(machine, param[0], &bpindex))
		return;
	else
	{
		bool found = false;
		for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
			if (device->debug()->breakpoint_clear(bpindex))
				found = true;
		if (found)
			debug_console_printf(machine, "Breakpoint %X cleared\n", (UINT32)bpindex);
		else
			debug_console_printf(machine, "Invalid breakpoint number %X\n", (UINT32)bpindex);
	}
}


/*-------------------------------------------------
    execute_bpdisenable - execute the breakpoint
    disable/enable commands
-------------------------------------------------*/

static void execute_bpdisenable(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 bpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
			device->debug()->breakpoint_enable_all(ref);
		if (ref == 0)
			debug_console_printf(machine, "Disabled all breakpoints\n");
		else
			debug_console_printf(machine, "Enabled all breakpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(machine, param[0], &bpindex))
		return;
	else
	{
		bool found = false;
		for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
			if (device->debug()->breakpoint_enable(bpindex, ref))
				found = true;
		if (found)
			debug_console_printf(machine, "Breakpoint %X %s\n", (UINT32)bpindex, ref ? "enabled" : "disabled");
		else
			debug_console_printf(machine, "Invalid breakpoint number %X\n", (UINT32)bpindex);
	}
}


/*-------------------------------------------------
    execute_bplist - execute the breakpoint list
    command
-------------------------------------------------*/

static void execute_bplist(running_machine *machine, int ref, int params, const char *param[])
{
	int printed = 0;
	astring buffer;

	/* loop over all CPUs */
	for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
		if (device->debug()->breakpoint_first() != NULL)
		{
			debug_console_printf(machine, "Device '%s' breakpoints:\n", device->tag());

			/* loop over the breakpoints */
			for (device_debug::breakpoint *bp = device->debug()->breakpoint_first(); bp != NULL; bp = bp->next())
			{
				buffer.printf("%c%4X @ %s", bp->enabled() ? ' ' : 'D', bp->index(), core_i64_hex_format(bp->address(), device->debug()->logaddrchars()));
				if (bp->condition() != NULL)
					buffer.catprintf(" if %s", bp->condition());
				if (bp->action() != NULL)
					buffer.catprintf(" do %s", bp->action());
				debug_console_printf(machine, "%s\n", buffer.cstr());
				printed++;
			}
		}

	if (printed == 0)
		debug_console_printf(machine, "No breakpoints currently installed\n");
}


/*-------------------------------------------------
    execute_wpset - execute the watchpoint set
    command
-------------------------------------------------*/

static void execute_wpset(running_machine *machine, int ref, int params, const char *param[])
{
	parsed_expression *condition = NULL;
	const address_space *space;
	const char *action = NULL;
	UINT64 address, length;
	int type = 0;
	int wpnum;

	/* param 1 is the address */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* param 2 is the length */
	if (!debug_command_parameter_number(machine, param[1], &length))
		return;

	/* param 3 is the type */
	if (!strcmp(param[2], "r"))
		type = WATCHPOINT_READ;
	else if (!strcmp(param[2], "w"))
		type = WATCHPOINT_WRITE;
	else if (!strcmp(param[2], "rw") || !strcmp(param[2], "wr"))
		type = WATCHPOINT_READWRITE;
	else
	{
		debug_console_printf(machine, "Invalid watchpoint type: expected r, w, or rw\n");
		return;
	}

	/* param 4 is the condition */
	if (!debug_command_parameter_expression(machine, param[3], &condition))
		return;

	/* param 5 is the action */
	if (!debug_command_parameter_command(machine, action = param[4]))
		return;

	/* CPU is implicit */
	if (!debug_command_parameter_cpu_space(machine, NULL, ref, &space))
		return;

	/* set the watchpoint */
	wpnum = space->cpu->debug()->watchpoint_set(*space, type, address, length, condition, action);
	debug_console_printf(machine, "Watchpoint %X set\n", wpnum);
}


/*-------------------------------------------------
    execute_wpclear - execute the watchpoint
    clear command
-------------------------------------------------*/

static void execute_wpclear(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 wpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
			device->debug()->watchpoint_clear_all();
		debug_console_printf(machine, "Cleared all watchpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(machine, param[0], &wpindex))
		return;
	else
	{
		bool found = false;
		for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
			if (device->debug()->watchpoint_clear(wpindex))
				found = true;
		if (found)
			debug_console_printf(machine, "Watchpoint %X cleared\n", (UINT32)wpindex);
		else
			debug_console_printf(machine, "Invalid watchpoint number %X\n", (UINT32)wpindex);
	}
}


/*-------------------------------------------------
    execute_wpdisenable - execute the watchpoint
    disable/enable commands
-------------------------------------------------*/

static void execute_wpdisenable(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 wpindex;

	/* if 0 parameters, clear all */
	if (params == 0)
	{
		for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
			device->debug()->watchpoint_enable_all(ref);
		if (ref == 0)
			debug_console_printf(machine, "Disabled all watchpoints\n");
		else
			debug_console_printf(machine, "Enabled all watchpoints\n");
	}

	/* otherwise, clear the specific one */
	else if (!debug_command_parameter_number(machine, param[0], &wpindex))
		return;
	else
	{
		bool found = false;
		for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
			if (device->debug()->watchpoint_enable(wpindex, ref))
				found = true;
		if (found)
			debug_console_printf(machine, "Watchpoint %X %s\n", (UINT32)wpindex, ref ? "enabled" : "disabled");
		else
			debug_console_printf(machine, "Invalid watchpoint number %X\n", (UINT32)wpindex);
	}
}


/*-------------------------------------------------
    execute_wplist - execute the watchpoint list
    command
-------------------------------------------------*/

static void execute_wplist(running_machine *machine, int ref, int params, const char *param[])
{
	int printed = 0;
	astring buffer;

	/* loop over all CPUs */
	for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
		for (int spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			if (device->debug()->watchpoint_first(spacenum) != NULL)
			{
				static const char *const types[] = { "unkn ", "read ", "write", "r/w  " };

				debug_console_printf(machine, "Device '%s' %s space watchpoints:\n", device->tag(), device->debug()->watchpoint_first(spacenum)->space().name);

				/* loop over the watchpoints */
				for (device_debug::watchpoint *wp = device->debug()->watchpoint_first(spacenum); wp != NULL; wp = wp->next())
				{
					buffer.printf("%c%4X @ %s-%s %s", wp->enabled() ? ' ' : 'D', wp->index(),
							core_i64_hex_format(memory_byte_to_address(&wp->space(), wp->address()), wp->space().addrchars),
							core_i64_hex_format(memory_byte_to_address_end(&wp->space(), wp->address() + wp->length()) - 1, wp->space().addrchars),
							types[wp->type() & 3]);
					if (wp->condition() != NULL)
						buffer.catprintf(" if %s", wp->condition());
					if (wp->action() != NULL)
						buffer.catprintf(" do %s", wp->action());
					debug_console_printf(machine, "%s\n", buffer.cstr());
					printed++;
				}
			}

	if (printed == 0)
		debug_console_printf(machine, "No watchpoints currently installed\n");
}


/*-------------------------------------------------
    execute_hotspot - execute the hotspot
    command
-------------------------------------------------*/

static void execute_hotspot(running_machine *machine, int ref, int params, const char *param[])
{
	/* if no params, and there are live hotspots, clear them */
	if (params == 0)
	{
		bool cleared = false;

		/* loop over CPUs and find live spots */
		for (device_t *device = machine->m_devicelist.first(); device != NULL; device = device->next())
			if (device->debug()->hotspot_tracking_enabled())
			{
				device->debug()->hotspot_track(0, 0);
				debug_console_printf(machine, "Cleared hotspot tracking on CPU '%s'\n", device->tag());
				cleared = true;
			}

		/* if we cleared, we're done */
		if (cleared)
			return;
	}

	/* extract parameters */
	device_t *device = NULL;
	if (!debug_command_parameter_cpu(machine, (params > 0) ? param[0] : NULL, &device))
		return;
	UINT64 count = 64;
	if (!debug_command_parameter_number(machine, param[1], &count))
		return;
	UINT64 threshhold = 250;
	if (!debug_command_parameter_number(machine, param[2], &threshhold))
		return;

	/* attempt to install */
	device->debug()->hotspot_track(count, threshhold);
	debug_console_printf(machine, "Now tracking hotspots on CPU '%s' using %d slots with a threshhold of %d\n", device->tag(), (int)count, (int)threshhold);
}


/*-------------------------------------------------
    execute_save - execute the save command
-------------------------------------------------*/

static void execute_save(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length;
	const address_space *space;
	FILE *f;
	UINT64 i;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[1], &offset))
		return;
	if (!debug_command_parameter_number(machine, param[2], &length))
		return;
	if (!debug_command_parameter_cpu_space(machine, (params > 3) ? param[3] : NULL, ref, &space))
		return;

	/* determine the addresses to write */
	endoffset = memory_address_to_byte(space, offset + length - 1) & space->bytemask;
	offset = memory_address_to_byte(space, offset) & space->bytemask;

	/* open the file */
	f = fopen(param[0], "wb");
	if (!f)
	{
		debug_console_printf(machine, "Error opening file '%s'\n", param[0]);
		return;
	}

	/* now write the data out */
	for (i = offset; i <= endoffset; i++)
	{
		UINT8 byte = debug_read_byte(space, i, TRUE);
		fwrite(&byte, 1, 1, f);
	}

	/* close the file */
	fclose(f);
	debug_console_printf(machine, "Data saved successfully\n");
}


/*-------------------------------------------------
    execute_dump - execute the dump command
-------------------------------------------------*/

static void execute_dump(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length, width = 0, ascii = 1;
	const address_space *space;
	FILE *f = NULL;
	UINT64 i, j;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[1], &offset))
		return;
	if (!debug_command_parameter_number(machine, param[2], &length))
		return;
	if (!debug_command_parameter_number(machine, param[3], &width))
		return;
	if (!debug_command_parameter_number(machine, param[4], &ascii))
		return;
	if (!debug_command_parameter_cpu_space(machine, (params > 5) ? param[5] : NULL, ref, &space))
		return;

	/* further validation */
	if (width == 0)
		width = space->dbits / 8;
	if (width < memory_address_to_byte(space, 1))
		width = memory_address_to_byte(space, 1);
	if (width != 1 && width != 2 && width != 4 && width != 8)
	{
		debug_console_printf(machine, "Invalid width! (must be 1,2,4 or 8)\n");
		return;
	}
	endoffset = memory_address_to_byte(space, offset + length - 1) & space->bytemask;
	offset = memory_address_to_byte(space, offset) & space->bytemask;

	/* open the file */
	f = fopen(param[0], "w");
	if (!f)
	{
		debug_console_printf(machine, "Error opening file '%s'\n", param[0]);
		return;
	}

	/* now write the data out */
	for (i = offset; i <= endoffset; i += 16)
	{
		char output[200];
		int outdex = 0;

		/* print the address */
		outdex += sprintf(&output[outdex], "%s: ", core_i64_hex_format((UINT32)memory_byte_to_address(space, i), space->logaddrchars));

		/* print the bytes */
		for (j = 0; j < 16; j += width)
		{
			if (i + j <= endoffset)
			{
				offs_t curaddr = i + j;
				if (debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &curaddr))
				{
					UINT64 value = debug_read_memory(space, i + j, width, TRUE);
					outdex += sprintf(&output[outdex], " %s", core_i64_hex_format(value, width * 2));
				}
				else
					outdex += sprintf(&output[outdex], " %.*s", (int)width * 2, "****************");
			}
			else
				outdex += sprintf(&output[outdex], " %*s", (int)width * 2, "");
		}

		/* print the ASCII */
		if (ascii)
		{
			outdex += sprintf(&output[outdex], "  ");
			for (j = 0; j < 16 && (i + j) <= endoffset; j++)
			{
				offs_t curaddr = i + j;
				if (debug_cpu_translate(space, TRANSLATE_READ_DEBUG, &curaddr))
				{
					UINT8 byte = debug_read_byte(space, i + j, TRUE);
					outdex += sprintf(&output[outdex], "%c", (byte >= 32 && byte < 128) ? byte : '.');
				}
				else
					outdex += sprintf(&output[outdex], " ");
			}
		}

		/* output the result */
		fprintf(f, "%s\n", output);
	}

	/* close the file */
	fclose(f);
	debug_console_printf(machine, "Data dumped successfully\n");
}


/*-------------------------------------------------
   execute_cheatinit - initialize the cheat system
-------------------------------------------------*/

static void execute_cheatinit(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 offset, length = 0, real_length = 0;
	const address_space *space;
	UINT32 active_cheat = 0;
	UINT64 curaddr;
	UINT8 i, region_count = 0;

	address_map_entry *entry;
	cheat_region_map cheat_region[100];

	memset(cheat_region, 0, sizeof(cheat_region));

	/* validate parameters */
	if (!debug_command_parameter_cpu_space(machine, (params > 3) ? param[3] : NULL, ADDRESS_SPACE_PROGRAM, &space))
		return;

	if (ref == 0)
	{
		cheat.width = 1;
		cheat.signed_cheat = FALSE;
		cheat.swapped_cheat = FALSE;
		if (params > 0)
		{
			char *srtpnt = (char*)param[0];

			if (*srtpnt == 's')
				cheat.signed_cheat = TRUE;
			else if (*srtpnt == 'u')
				cheat.signed_cheat = FALSE;
			else
			{
				debug_console_printf(machine, "Invalid sign: expected s or u\n");
				return;
			}

			if (*(++srtpnt) == 'b')
				cheat.width = 1;
			else if (*srtpnt == 'w')
				cheat.width = 2;
			else if (*srtpnt == 'd')
				cheat.width = 4;
			else if (*srtpnt == 'q')
				cheat.width = 8;
			else
			{
				debug_console_printf(machine, "Invalid width: expected b, w, d or q\n");
				return;
			}

			if (*(++srtpnt) == 's')
				cheat.swapped_cheat = TRUE;
			else
				cheat.swapped_cheat = FALSE;
		}
	}

	/* initialize entire memory by default */
	if (params <= 1)
	{
		offset = 0;
		length = space->bytemask + 1;
		for (entry = space->map->entrylist; entry != NULL; entry = entry->next)
		{
			cheat_region[region_count].offset = memory_address_to_byte(space, entry->addrstart) & space->bytemask;
			cheat_region[region_count].endoffset = memory_address_to_byte(space, entry->addrend) & space->bytemask;
			cheat_region[region_count].share = entry->share;
			cheat_region[region_count].disabled = (entry->write.type == AMH_RAM) ? FALSE : TRUE;

			/* disable double share regions */
			if (entry->share != NULL)
				for (i = 0; i < region_count; i++)
					if (cheat_region[i].share != NULL)
						if (strcmp(cheat_region[i].share, entry->share)==0)
							cheat_region[region_count].disabled = TRUE;

			region_count++;
		}
	}
	else
	{
		/* validate parameters */
		if (!debug_command_parameter_number(machine, param[(ref == 0) ? 1 : 0], &offset))
			return;
		if (!debug_command_parameter_number(machine, param[(ref == 0) ? 2 : 1], &length))
			return;

		/* force region to the specified range */
		cheat_region[region_count].offset = memory_address_to_byte(space, offset) & space->bytemask;;
		cheat_region[region_count].endoffset = memory_address_to_byte(space, offset + length - 1) & space->bytemask;;
		cheat_region[region_count].share = NULL;
		cheat_region[region_count].disabled = FALSE;
		region_count++;
	}

	/* determine the writable extent of each region in total */
	for (i = 0; i <= region_count; i++)
		if (!cheat_region[i].disabled)
			for (curaddr = cheat_region[i].offset; curaddr <= cheat_region[i].endoffset; curaddr += cheat.width)
				if (cheat_address_is_valid(space, curaddr))
					real_length++;

	if (real_length == 0)
	{
		debug_console_printf(machine, "No writable bytes found in this area\n");
		return;
	}

	if (ref == 0)
	{
		/* initialize new cheat system */
		if (cheat.cheatmap != NULL)
			auto_free(machine, cheat.cheatmap);
		cheat.cheatmap = auto_alloc_array(machine, cheat_map, real_length);

		cheat.length = real_length;
		cheat.undo = 0;
		cheat.cpu = (params > 3) ? *param[3] : '0';
	}
	else
	{
		/* add range to cheat system */
		if (cheat.cpu == 0)
		{
			debug_console_printf(machine, "Use cheatinit before cheatrange\n");
			return;
		}

		if (!debug_command_parameter_cpu_space(machine, &cheat.cpu, ADDRESS_SPACE_PROGRAM, &space))
			return;

		cheat_map *newmap = auto_alloc_array(machine, cheat_map, cheat.length + real_length);
		for (int item = 0; item < cheat.length; item++)
			newmap[item] = cheat.cheatmap[item];
		auto_free(machine, cheat.cheatmap);
		cheat.cheatmap = newmap;

		active_cheat = cheat.length;
		cheat.length += real_length;
	}

	/* initialize cheatmap in the selected space */
	for (i = 0; i < region_count; i++)
		if (!cheat_region[i].disabled)
			for (curaddr = cheat_region[i].offset; curaddr <= cheat_region[i].endoffset; curaddr += cheat.width)
				if (cheat_address_is_valid(space, curaddr))
				{
					cheat.cheatmap[active_cheat].previous_value = cheat_read_extended(&cheat, space, curaddr);
					cheat.cheatmap[active_cheat].first_value = cheat.cheatmap[active_cheat].previous_value;
					cheat.cheatmap[active_cheat].offset = curaddr;
					cheat.cheatmap[active_cheat].state = 1;
					cheat.cheatmap[active_cheat].undo = 0;
					active_cheat++;
				}

	debug_console_printf(machine, "%u cheat initialized\n", active_cheat);
}


/*-------------------------------------------------
    execute_cheatnext - execute the search
-------------------------------------------------*/

static void execute_cheatnext(running_machine *machine, int ref, int params, const char *param[])
{
	const address_space *space;
	UINT64 cheatindex;
	UINT32 active_cheat = 0;
	UINT8 condition;
	UINT64 comp_value = 0;

	enum
	{
		CHEAT_ALL = 0,
		CHEAT_EQUAL,
		CHEAT_NOTEQUAL,
		CHEAT_EQUALTO,
		CHEAT_NOTEQUALTO,
		CHEAT_DECREASE,
		CHEAT_INCREASE,
		CHEAT_DECREASE_OR_EQUAL,
		CHEAT_INCREASE_OR_EQUAL,
		CHEAT_DECREASEOF,
		CHEAT_INCREASEOF,
		CHEAT_SMALLEROF,
		CHEAT_GREATEROF
	};

	if (cheat.cpu == 0)
	{
		debug_console_printf(machine, "Use cheatinit before cheatnext\n");
		return;
	}

	if (!debug_command_parameter_cpu_space(machine, &cheat.cpu, ADDRESS_SPACE_PROGRAM, &space))
		return;

	if (params > 1 && !debug_command_parameter_number(machine, param[1], &comp_value))
		return;
	comp_value = cheat_sign_extend(&cheat, comp_value);

	/* decode contidion */
	if (!strcmp(param[0], "all"))
		condition = CHEAT_ALL;
	else if (!strcmp(param[0], "equal") || !strcmp(param[0], "eq"))
		condition = (params > 1) ? CHEAT_EQUALTO : CHEAT_EQUAL;
	else if (!strcmp(param[0], "notequal") || !strcmp(param[0], "ne"))
		condition = (params > 1) ? CHEAT_NOTEQUALTO : CHEAT_NOTEQUAL;
	else if (!strcmp(param[0], "decrease") || !strcmp(param[0], "de") || !strcmp(param[0], "-"))
		condition = (params > 1) ? CHEAT_DECREASEOF : CHEAT_DECREASE;
	else if (!strcmp(param[0], "increase") || !strcmp(param[0], "in") || !strcmp(param[0], "+"))
		condition = (params > 1) ? CHEAT_INCREASEOF : CHEAT_INCREASE;
	else if (!strcmp(param[0], "decreaseorequal") || !strcmp(param[0], "deeq"))
		condition = CHEAT_DECREASE_OR_EQUAL;
	else if (!strcmp(param[0], "increaseorequal") || !strcmp(param[0], "ineq"))
		condition = CHEAT_INCREASE_OR_EQUAL;
	else if (!strcmp(param[0], "smallerof") || !strcmp(param[0], "lt") || !strcmp(param[0], "<"))
		condition = CHEAT_SMALLEROF;
	else if (!strcmp(param[0], "greaterof") || !strcmp(param[0], "gt") || !strcmp(param[0], ">"))
		condition = CHEAT_GREATEROF;
	else
	{
		debug_console_printf(machine, "Invalid condition type\n");
		return;
	}

	cheat.undo++;

	/* execute the search */
	for (cheatindex = 0; cheatindex < cheat.length; cheatindex += 1)
		if (cheat.cheatmap[cheatindex].state == 1)
		{
			UINT64 cheat_value = cheat_read_extended(&cheat, space, cheat.cheatmap[cheatindex].offset);
			UINT64 comp_byte = (ref == 0) ? cheat.cheatmap[cheatindex].previous_value : cheat.cheatmap[cheatindex].first_value;
			UINT8 disable_byte = FALSE;

			switch (condition)
			{
				case CHEAT_ALL:
					break;

				case CHEAT_EQUAL:
					disable_byte = (cheat_value != comp_byte);
					break;

				case CHEAT_NOTEQUAL:
					disable_byte = (cheat_value == comp_byte);
					break;

				case CHEAT_EQUALTO:
					disable_byte = (cheat_value != comp_value);
					break;

				case CHEAT_NOTEQUALTO:
					disable_byte = (cheat_value == comp_value);
					break;

				case CHEAT_DECREASE:
					if (cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value >= (INT64)comp_byte);
					else
						disable_byte = ((UINT64)cheat_value >= (UINT64)comp_byte);
					break;

				case CHEAT_INCREASE:
					if (cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value <= (INT64)comp_byte);
					else
						disable_byte = ((UINT64)cheat_value <= (UINT64)comp_byte);
					break;

				case CHEAT_DECREASE_OR_EQUAL:
					if (cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value > (INT64)comp_byte);
					else
						disable_byte = ((UINT64)cheat_value > (UINT64)comp_byte);
					break;

				case CHEAT_INCREASE_OR_EQUAL:
					if (cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value < (INT64)comp_byte);
					else
						disable_byte = ((UINT64)cheat_value < (UINT64)comp_byte);
					break;

				case CHEAT_DECREASEOF:
					disable_byte = (cheat_value != comp_byte - comp_value);
					break;

				case CHEAT_INCREASEOF:
					disable_byte = (cheat_value != comp_byte + comp_value);
					break;

				case CHEAT_SMALLEROF:
					if (cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value >= (INT64)comp_value);
					else
						disable_byte = ((UINT64)cheat_value >= (UINT64)comp_value);
					break;

				case CHEAT_GREATEROF:
					if (cheat.signed_cheat)
						disable_byte = ((INT64)cheat_value <= (INT64)comp_value);
					else
						disable_byte = ((UINT64)cheat_value <= (UINT64)comp_value);
					break;
			}

			if (disable_byte)
			{
				cheat.cheatmap[cheatindex].state = 0;
				cheat.cheatmap[cheatindex].undo = cheat.undo;
			}
			else
				active_cheat++;

			/* update previous value */
			cheat.cheatmap[cheatindex].previous_value = cheat_value;
		}

	if (active_cheat <= 5)
		execute_cheatlist(machine, 0, 0, NULL);

	debug_console_printf(machine, "%u cheats found\n", active_cheat);
}


/*-------------------------------------------------
    execute_cheatlist - show a list of active cheat
-------------------------------------------------*/

static void execute_cheatlist(running_machine *machine, int ref, int params, const char *param[])
{
	char spaceletter, sizeletter;
	const address_space *space;
	device_t *cpu;
	UINT32 active_cheat = 0;
	UINT64 cheatindex;
	UINT64 sizemask;
	FILE *f = NULL;

	if (!debug_command_parameter_cpu_space(machine, &cheat.cpu, ADDRESS_SPACE_PROGRAM, &space))
		return;

	if (!debug_command_parameter_cpu(machine, &cheat.cpu, &cpu))
		return;

	if (params > 0)
		f = fopen(param[0], "w");

	switch (space->spacenum)
	{
		default:
		case ADDRESS_SPACE_PROGRAM:	spaceletter = 'p';	break;
		case ADDRESS_SPACE_DATA:	spaceletter = 'd';	break;
		case ADDRESS_SPACE_IO:		spaceletter = 'i';	break;
	}

	switch (cheat.width)
	{
		default:
		case 1:						sizeletter = 'b';	sizemask = 0xff;					break;
		case 2:						sizeletter = 'w';	sizemask = 0xffff;					break;
		case 4:						sizeletter = 'd';	sizemask = 0xffffffff;				break;
		case 8:						sizeletter = 'q';	sizemask = U64(0xffffffffffffffff);	break;
	}

	/* write the cheat list */
	for (cheatindex = 0; cheatindex < cheat.length; cheatindex += 1)
	{
		if (cheat.cheatmap[cheatindex].state == 1)
		{
			UINT64 value = cheat_byte_swap(&cheat, cheat_read_extended(&cheat, space, cheat.cheatmap[cheatindex].offset)) & sizemask;
			offs_t address = memory_byte_to_address(space, cheat.cheatmap[cheatindex].offset);

			if (params > 0)
			{
				active_cheat++;
				fprintf(f, "  <cheat desc=\"Possibility %d : %s (%s)\">\n", active_cheat, core_i64_hex_format(address, space->logaddrchars), core_i64_hex_format(value, cheat.width * 2));
				fprintf(f, "    <script state=\"run\">\n");
				fprintf(f, "      <action>%s.p%c%c@%s=%s</action>\n", cpu->tag(), spaceletter, sizeletter, core_i64_hex_format(address, space->logaddrchars), core_i64_hex_format(cheat_byte_swap(&cheat, cheat.cheatmap[cheatindex].first_value) & sizemask, cheat.width * 2));
				fprintf(f, "    </script>\n");
				fprintf(f, "  </cheat>\n\n");
			}
			else
				debug_console_printf(machine, "Address=%s Start=%s Current=%s\n", core_i64_hex_format(address, space->logaddrchars), core_i64_hex_format(cheat_byte_swap(&cheat, cheat.cheatmap[cheatindex].first_value) & sizemask, cheat.width * 2), core_i64_hex_format(value, cheat.width * 2));
		}
	}
	if (params > 0)
		fclose(f);
}


/*-------------------------------------------------
    execute_cheatundo - undo the last search
-------------------------------------------------*/

static void execute_cheatundo(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 cheatindex;
	UINT32 undo_count = 0;

	if (cheat.undo > 0)
	{
		for (cheatindex = 0; cheatindex < cheat.length; cheatindex += 1)
		{
			if (cheat.cheatmap[cheatindex].undo == cheat.undo)
			{
				cheat.cheatmap[cheatindex].state = 1;
				cheat.cheatmap[cheatindex].undo = 0;
				undo_count++;
			}
		}

		cheat.undo--;
		debug_console_printf(machine, "%u cheat reactivated\n", undo_count);
	}
	else
		debug_console_printf(machine, "Maximum undo reached\n");
}


/*-------------------------------------------------
    execute_find - execute the find command
-------------------------------------------------*/

static void execute_find(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 offset, endoffset, length;
	const address_space *space;
	UINT64 data_to_find[256];
	UINT8 data_size[256];
	int cur_data_size;
	int data_count = 0;
	int found = 0;
	UINT64 i, j;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[0], &offset))
		return;
	if (!debug_command_parameter_number(machine, param[1], &length))
		return;
	if (!debug_command_parameter_cpu_space(machine, NULL, ref, &space))
		return;

	/* further validation */
	endoffset = memory_address_to_byte(space, offset + length - 1) & space->bytemask;
	offset = memory_address_to_byte(space, offset) & space->bytemask;
	cur_data_size = memory_address_to_byte(space, 1);
	if (cur_data_size == 0)
		cur_data_size = 1;

	/* parse the data parameters */
	for (i = 2; i < params; i++)
	{
		const char *pdata = param[i];

		/* check for a string */
		if (pdata[0] == '"' && pdata[strlen(pdata) - 1] == '"')
		{
			for (j = 1; j < strlen(pdata) - 1; j++)
			{
				data_to_find[data_count] = pdata[j];
				data_size[data_count++] = 1;
			}
		}

		/* otherwise, validate as a number */
		else
		{
			/* check for a 'b','w','d',or 'q' prefix */
			data_size[data_count] = cur_data_size;
			if (tolower((UINT8)pdata[0]) == 'b' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 1; pdata += 2; }
			if (tolower((UINT8)pdata[0]) == 'w' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 2; pdata += 2; }
			if (tolower((UINT8)pdata[0]) == 'd' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 4; pdata += 2; }
			if (tolower((UINT8)pdata[0]) == 'q' && pdata[1] == '.') { data_size[data_count] = cur_data_size = 8; pdata += 2; }

			/* look for a wildcard */
			if (!strcmp(pdata, "?"))
				data_size[data_count++] |= 0x10;

			/* otherwise, validate as a number */
			else if (!debug_command_parameter_number(machine, pdata, &data_to_find[data_count++]))
				return;
		}
	}

	/* now search */
	for (i = offset; i <= endoffset; i += data_size[0])
	{
		int suboffset = 0;
		int match = 1;

		/* find the entire string */
		for (j = 0; j < data_count && match; j++)
		{
			switch (data_size[j])
			{
				case 1:	match = ((UINT8)debug_read_byte(space, i + suboffset, TRUE) == (UINT8)data_to_find[j]);	break;
				case 2:	match = ((UINT16)debug_read_word(space, i + suboffset, TRUE) == (UINT16)data_to_find[j]);	break;
				case 4:	match = ((UINT32)debug_read_dword(space, i + suboffset, TRUE) == (UINT32)data_to_find[j]);	break;
				case 8:	match = ((UINT64)debug_read_qword(space, i + suboffset, TRUE) == (UINT64)data_to_find[j]);	break;
				default:	/* all other cases are wildcards */		break;
			}
			suboffset += data_size[j] & 0x0f;
		}

		/* did we find it? */
		if (match)
		{
			found++;
			debug_console_printf(machine, "Found at %s\n", core_i64_hex_format((UINT32)memory_byte_to_address(space, i), space->addrchars));
		}
	}

	/* print something if not found */
	if (found == 0)
		debug_console_printf(machine, "Not found\n");
}


/*-------------------------------------------------
    execute_dasm - execute the dasm command
-------------------------------------------------*/

static void execute_dasm(running_machine *machine, int ref, int params, const char *param[])
{
	UINT64 offset, length, bytes = 1;
	int minbytes, maxbytes, byteswidth;
	const address_space *space;
	FILE *f = NULL;
	int i, j;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[1], &offset))
		return;
	if (!debug_command_parameter_number(machine, param[2], &length))
		return;
	if (!debug_command_parameter_number(machine, param[3], &bytes))
		return;
	if (!debug_command_parameter_cpu_space(machine, (params > 4) ? param[4] : NULL, ADDRESS_SPACE_PROGRAM, &space))
		return;

	/* determine the width of the bytes */
	cpu_device *cpudevice = downcast<cpu_device *>(space->cpu);
	minbytes = cpudevice->min_opcode_bytes();
	maxbytes = cpudevice->max_opcode_bytes();
	byteswidth = 0;
	if (bytes)
	{
		byteswidth = (maxbytes + (minbytes - 1)) / minbytes;
		byteswidth *= (2 * minbytes) + 1;
	}

	/* open the file */
	f = fopen(param[0], "w");
	if (!f)
	{
		debug_console_printf(machine, "Error opening file '%s'\n", param[0]);
		return;
	}

	/* now write the data out */
	for (i = 0; i < length; )
	{
		int pcbyte = memory_address_to_byte(space, offset + i) & space->bytemask;
		char output[200+DEBUG_COMMENT_MAX_LINE_LENGTH], disasm[200];
		const char *comment;
		offs_t tempaddr;
		int outdex = 0;
		int numbytes = 0;

		/* print the address */
		outdex += sprintf(&output[outdex], "%s: ", core_i64_hex_format((UINT32)memory_byte_to_address(space, pcbyte), space->logaddrchars));

		/* make sure we can translate the address */
		tempaddr = pcbyte;
		if (debug_cpu_translate(space, TRANSLATE_FETCH_DEBUG, &tempaddr))
		{
			UINT8 opbuf[64], argbuf[64];

			/* fetch the bytes up to the maximum */
			for (numbytes = 0; numbytes < maxbytes; numbytes++)
			{
				opbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1, FALSE);
				argbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1, TRUE);
			}

			/* disassemble the result */
			i += numbytes = space->cpu->debug()->disassemble(disasm, offset + i, opbuf, argbuf) & DASMFLAG_LENGTHMASK;
		}

		/* print the bytes */
		if (bytes)
		{
			int startdex = outdex;
			numbytes = memory_address_to_byte(space, numbytes);
			for (j = 0; j < numbytes; j += minbytes)
				outdex += sprintf(&output[outdex], "%s ", core_i64_hex_format(debug_read_opcode(space, pcbyte + j, minbytes, FALSE), minbytes * 2));
			if (outdex - startdex < byteswidth)
				outdex += sprintf(&output[outdex], "%*s", byteswidth - (outdex - startdex), "");
			outdex += sprintf(&output[outdex], "  ");
		}

		/* add the disassembly */
		sprintf(&output[outdex], "%s", disasm);

		/* attempt to add the comment */
		comment = debug_comment_get_text(space->cpu, tempaddr, debug_comment_get_opcode_crc32(space->cpu, tempaddr));
		if (comment != NULL)
		{
			/* somewhat arbitrary guess as to how long most disassembly lines will be [column 60] */
			if (strlen(output) < 60)
			{
				/* pad the comment space out to 60 characters and null-terminate */
				for (outdex = (int)strlen(output); outdex < 60; outdex++)
					output[outdex] = ' ' ;
				output[outdex] = 0 ;

				sprintf(&output[strlen(output)], "// %s", comment) ;
			}
			else
				sprintf(&output[strlen(output)], "\t// %s", comment) ;
		}

		/* output the result */
		fprintf(f, "%s\n", output);
	}

	/* close the file */
	fclose(f);
	debug_console_printf(machine, "Data dumped successfully\n");
}


/*-------------------------------------------------
    execute_trace_internal - functionality for
    trace over and trace info
-------------------------------------------------*/

static void execute_trace_internal(running_machine *machine, int ref, int params, const char *param[], int trace_over)
{
	const char *action = NULL, *filename = param[0];
	device_t *cpu;
	FILE *f = NULL;
	const char *mode;

	/* validate parameters */
	if (!debug_command_parameter_cpu(machine, (params > 1) ? param[1] : NULL, &cpu))
		return;
	if (!debug_command_parameter_command(machine, action = param[2]))
		return;

	/* further validation */
	if (mame_stricmp(filename, "off") == 0)
		filename = NULL;

	/* open the file */
	if (filename)
	{
		mode = "w";

		/* opening for append? */
		if ((filename[0] == '>') && (filename[1] == '>'))
		{
			mode = "a";
			filename += 2;
		}

		f = fopen(filename, mode);
		if (!f)
		{
			debug_console_printf(machine, "Error opening file '%s'\n", param[0]);
			return;
		}
	}

	/* do it */
	cpu->debug()->trace(f, trace_over, action);
	if (f)
		debug_console_printf(machine, "Tracing CPU '%s' to file %s\n", cpu->tag(), filename);
	else
		debug_console_printf(machine, "Stopped tracing on CPU '%s'\n", cpu->tag());
}


/*-------------------------------------------------
    execute_trace - execute the trace command
-------------------------------------------------*/

static void execute_trace(running_machine *machine, int ref, int params, const char *param[])
{
	execute_trace_internal(machine, ref, params, param, 0);
}


/*-------------------------------------------------
    execute_traceover - execute the trace over command
-------------------------------------------------*/

static void execute_traceover(running_machine *machine, int ref, int params, const char *param[])
{
	execute_trace_internal(machine, ref, params, param, 1);
}


/*-------------------------------------------------
    execute_traceflush - execute the trace flush command
-------------------------------------------------*/

static void execute_traceflush(running_machine *machine, int ref, int params, const char *param[])
{
	debug_cpu_flush_traces(machine);
}


/*-------------------------------------------------
    execute_history - execute the history command
-------------------------------------------------*/

static void execute_history(running_machine *machine, int ref, int params, const char *param[])
{
	/* validate parameters */
	const address_space *space;
	if (!debug_command_parameter_cpu_space(machine, (params > 0) ? param[0] : NULL, ADDRESS_SPACE_PROGRAM, &space))
		return;

	UINT64 count = device_debug::HISTORY_SIZE;
	if (!debug_command_parameter_number(machine, param[1], &count))
		return;

	/* further validation */
	if (count > device_debug::HISTORY_SIZE)
		count = device_debug::HISTORY_SIZE;

	device_debug *debug = space->cpu->debug();

	/* loop over lines */
	int maxbytes = debug->max_opcode_bytes();
	for (int index = 0; index < count; index++)
	{
		offs_t pc = debug->history_pc(-index);

		/* fetch the bytes up to the maximum */
		offs_t pcbyte = memory_address_to_byte(space, pc) & space->bytemask;
		UINT8 opbuf[64], argbuf[64];
		for (int numbytes = 0; numbytes < maxbytes; numbytes++)
		{
			opbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1, false);
			argbuf[numbytes] = debug_read_opcode(space, pcbyte + numbytes, 1, true);
		}

		char buffer[200];
		debug->disassemble(buffer, pc, opbuf, argbuf);

		debug_console_printf(machine, "%s: %s\n", core_i64_hex_format(pc, space->logaddrchars), buffer);
	}
}


/*-------------------------------------------------
    execute_snap - execute the snapshot command
-------------------------------------------------*/

static void execute_snap(running_machine *machine, int ref, int params, const char *param[])
{

}


/*-------------------------------------------------
    execute_source - execute the source command
-------------------------------------------------*/

static void execute_source(running_machine *machine, int ref, int params, const char *param[])
{
	debug_cpu_source_script(machine, param[0]);
}


/*-------------------------------------------------
    execute_map - execute the map command
-------------------------------------------------*/

static void execute_map(running_machine *machine, int ref, int params, const char *param[])
{
	const address_space *space;
	offs_t taddress;
	UINT64 address;
	int intention;

	/* validate parameters */
	if (!debug_command_parameter_number(machine, param[0], &address))
		return;

	/* CPU is implicit */
	if (!debug_command_parameter_cpu_space(machine, NULL, ref, &space))
		return;

	/* do the translation first */
	for (intention = TRANSLATE_READ_DEBUG; intention <= TRANSLATE_FETCH_DEBUG; intention++)
	{
		static const char *const intnames[] = { "Read", "Write", "Fetch" };
		taddress = memory_address_to_byte(space, address) & space->bytemask;
		if (debug_cpu_translate(space, intention, &taddress))
		{
			const char *mapname = memory_get_handler_string(space, intention == TRANSLATE_WRITE_DEBUG, taddress);
			debug_console_printf(machine, "%7s: %s logical == %s physical -> %s\n", intnames[intention & 3], core_i64_hex_format(address, space->logaddrchars), core_i64_hex_format(memory_byte_to_address(space, taddress), space->addrchars), mapname);
		}
		else
			debug_console_printf(machine, "%7s: %s logical is unmapped\n", intnames[intention & 3], core_i64_hex_format(address, space->logaddrchars));
	}
}


/*-------------------------------------------------
    execute_memdump - execute the memdump command
-------------------------------------------------*/

static void execute_memdump(running_machine *machine, int ref, int params, const char **param)
{
	FILE *file;
	const char *filename;

	filename = (params == 0) ? "memdump.log" : param[0];

	debug_console_printf(machine, "Dumping memory to %s\n", filename);

	file = fopen(filename, "w");
	if (file)
	{
		memory_dump(machine, file);
		fclose(file);
	}
}


/*-------------------------------------------------
    execute_symlist - execute the symlist command
-------------------------------------------------*/

static int CLIB_DECL symbol_sort_compare(const void *item1, const void *item2)
{
	const char *str1 = *(const char **)item1;
	const char *str2 = *(const char **)item2;
	return strcmp(str1, str2);
}

static void execute_symlist(running_machine *machine, int ref, int params, const char **param)
{
	device_t *cpu = NULL;
	const char *namelist[1000];
	symbol_table *symtable;
	int symnum, count = 0;

	/* validate parameters */
	if (!debug_command_parameter_cpu(machine, param[0], &cpu))
		return;

	if (cpu != NULL)
	{
		symtable = cpu->debug()->symtable();
		debug_console_printf(machine, "CPU '%s' symbols:\n", cpu->tag());
	}
	else
	{
		symtable = debug_cpu_get_global_symtable(machine);
		debug_console_printf(machine, "Global symbols:\n");
	}

	/* gather names for all symbols */
	for (symnum = 0; symnum < 100000; symnum++)
	{
		const symbol_entry *entry;
		const char *name = symtable_find_indexed(symtable, symnum, &entry);

		/* if we didn't get anything, we're done */
		if (name == NULL)
			break;

		/* only display "register" type symbols */
		if (entry->type == SMT_REGISTER)
		{
			namelist[count++] = name;
			if (count >= ARRAY_LENGTH(namelist))
				break;
		}
	}

	/* sort the symbols */
	if (count > 1)
		qsort((void *)namelist, count, sizeof(namelist[0]), symbol_sort_compare);

	/* iterate over symbols and print out relevant ones */
	for (symnum = 0; symnum < count; symnum++)
	{
		const symbol_entry *entry = symtable_find(symtable, namelist[symnum]);
		UINT64 value = (*entry->info.reg.getter)(symtable_get_globalref(entry->table), entry->ref);
		assert(entry != NULL);

		/* only display "register" type symbols */
		debug_console_printf(machine, "%s = %s", namelist[symnum], core_i64_hex_format(value, 0));
		if (entry->info.reg.setter == NULL)
			debug_console_printf(machine, "  (read-only)");
		debug_console_printf(machine, "\n");
	}
}


/*-------------------------------------------------
    execute_softreset - execute the softreset command
-------------------------------------------------*/

static void execute_softreset(running_machine *machine, int ref, int params, const char **param)
{
	machine->schedule_soft_reset();
}


/*-------------------------------------------------
    execute_hardreset - execute the hardreset command
-------------------------------------------------*/

static void execute_hardreset(running_machine *machine, int ref, int params, const char **param)
{
	machine->schedule_hard_reset();
}
