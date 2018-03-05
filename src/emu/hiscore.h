/***************************************************************************

    hiscore.h

    Manages the hiscore system.

    Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __HISCORE_H__
#define __HISCORE_H__

void hiscore_init( running_machine *machine );
extern const char hiscoredat[];
extern const int hiscoredat_length;

#endif	/* __HISCORE_H__ */