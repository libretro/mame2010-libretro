/***************************************************************************

    driver.c

    Driver construction helpers.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define DRIVER_LRU_SIZE			10



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static int driver_lru[DRIVER_LRU_SIZE];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int penalty_compare(const char *source, const char *target);



/***************************************************************************
    MISC FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    driver_get_name - return a pointer to a
    driver given its name
-------------------------------------------------*/

const game_driver *driver_get_name(const char *name)
{
	int lurnum, drvnum;

	/* scan the LRU list first */
	for (lurnum = 0; lurnum < DRIVER_LRU_SIZE; lurnum++)
		if (mame_stricmp(drivers[driver_lru[lurnum]]->name, name) == 0)
		{
			/* if not first, swap with the head */
			if (lurnum != 0)
			{
				int temp = driver_lru[0];
				driver_lru[0] = driver_lru[lurnum];
				driver_lru[lurnum] = temp;
			}
			return drivers[driver_lru[0]];
		}

	/* scan for a match in the drivers -- slow! */
	for (drvnum = 0; drivers[drvnum] != NULL; drvnum++)
		if (mame_stricmp(drivers[drvnum]->name, name) == 0)
		{
			memmove((void *)&driver_lru[1], (void *)&driver_lru[0], sizeof(driver_lru[0]) * (DRIVER_LRU_SIZE - 1));
			driver_lru[0] = drvnum;
			return drivers[drvnum];
		}

	return NULL;
}


/*-------------------------------------------------
    driver_get_clone - return a pointer to the
    clone of a game driver.
-------------------------------------------------*/

const game_driver *driver_get_clone(const game_driver *driver)
{
	/* if no clone, easy out */
	if (driver->parent == NULL || (driver->parent[0] == '0' && driver->parent[1] == 0))
		return NULL;

	/* convert the name to a game_driver */
	return driver_get_name(driver->parent);
}


/*-------------------------------------------------
    driver_list_get_approx_matches - find the best
    n matches to a driver name.
-------------------------------------------------*/

void driver_list_get_approx_matches(const game_driver * const driverlist[], const char *name, int matches, const game_driver **list)
{
	int matchnum, drvnum;
	int *penalty;

	/* if no name, return the first N valid (non-NO_STANDALONE) entries.
	   This branch is reachable from the in-game game-select menu when the
	   search box is empty. The historical implementation shuffled the
	   list via libc rand() seeded from osd_ticks(), so each invocation
	   would surface a different sampling of drivers. That has been
	   removed because seeding libc's *global* PRNG state from wall-clock
	   ticks is the only place anything emu-side touches libc rand at
	   all, and it pollutes a global that's read elsewhere in the tree
	   (validity.c's math primitive tests, video.c's burn-in test
	   pattern). The shuffle existed for cosmetic 'fresh look' value in
	   the standalone-MAME menu; in the libretro core that menu is not
	   surfaced by the frontend, so the visible behaviour is unaffected. */
	if (name == NULL || name[0] == 0)
	{
		const game_driver **templist;
		int driver_count;

		/* allocate a temporary list */
		templist = global_alloc_array(const game_driver *, driver_list_get_count(driverlist));

		/* build up a list of valid entries */
		for (drvnum = driver_count = 0; driverlist[drvnum] != NULL; drvnum++)
			if ((driverlist[drvnum]->flags & GAME_NO_STANDALONE) == 0)
				templist[driver_count++] = driverlist[drvnum];

		/* copy out the first few entries */
		for (matchnum = 0; matchnum < matches; matchnum++)
			list[matchnum] = (driver_count > 0) ? templist[matchnum % driver_count] : NULL;

		global_free(templist);
		return;
	}

	/* allocate some temp memory */
	penalty = global_alloc_array(int, matches);

	/* initialize everyone's states */
	for (matchnum = 0; matchnum < matches; matchnum++)
	{
		penalty[matchnum] = 9999;
		list[matchnum] = NULL;
	}

	/* scan the entire drivers array */
	for (drvnum = 0; driverlist[drvnum] != NULL; drvnum++)
	{
		int curpenalty, tmp;

		/* skip things that can't run */
		if ((driverlist[drvnum]->flags & GAME_NO_STANDALONE) != 0)
			continue;

		/* pick the best match between driver name and description */
		curpenalty = penalty_compare(name, driverlist[drvnum]->description);
		tmp = penalty_compare(name, driverlist[drvnum]->name);
		curpenalty = MIN(curpenalty, tmp);

		/* insert into the sorted table of matches */
		for (matchnum = matches - 1; matchnum >= 0; matchnum--)
		{
			/* stop if we're worse than the current entry */
			if (curpenalty >= penalty[matchnum])
				break;

			/* as lng as this isn't the last entry, bump this one down */
			if (matchnum < matches - 1)
			{
				penalty[matchnum + 1] = penalty[matchnum];
				list[matchnum + 1] = list[matchnum];
			}
			list[matchnum] = driverlist[drvnum];
			penalty[matchnum] = curpenalty;
		}
	}

	/* free our temp memory */
	global_free(penalty);
}


/*-------------------------------------------------
    penalty_compare - compare two strings for
    closeness and assign a score.
-------------------------------------------------*/

static int penalty_compare(const char *source, const char *target)
{
	int gaps = 1;
	int last = TRUE;

	/* scan the strings */
	for ( ; *source && *target; target++)
	{
		/* do a case insensitive match */
		int match = (tolower((uint8_t)*source) == tolower((uint8_t)*target));

		/* if we matched, advance the source */
		if (match)
			source++;

		/* if the match state changed, count gaps */
		if (match != last)
		{
			last = match;
			if (!match)
				gaps++;
		}
	}

	/* penalty if short string does not completely fit in */
	for ( ; *source; source++)
		gaps++;

	/* if we matched perfectly, gaps == 0 */
	if (gaps == 1 && *source == 0 && *target == 0)
		gaps = 0;

	return gaps;
}


/*-------------------------------------------------
    driver_list_get_count - returns the amount of
    drivers
-------------------------------------------------*/

int driver_list_get_count(const game_driver * const driverlist[])
{
	int count;

	for (count = 0; driverlist[count] != NULL; count++) ;
	return count;
}

/*-------------------------------------------------
    driver_get_compatible - return a pointer to the
    compatible driver.
-------------------------------------------------*/

const game_driver *driver_get_compatible(const game_driver *drv)
{
	if (driver_get_clone(drv))
		drv = driver_get_clone(drv);
	else if (drv->compatible_with)
		drv = driver_get_name(drv->compatible_with);
	else
		drv = NULL;
	return drv;
}
