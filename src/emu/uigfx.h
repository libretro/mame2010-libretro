/***************************************************************************

    uigfx.h

    Internal graphics viewer.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UIGFX_H__
#define __UIGFX_H__



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* initialization */
void ui_gfx_init(running_machine *machine);

/* master handler */
uint32_t ui_gfx_ui_handler(running_machine *machine, render_container *container, uint32_t state);


#endif	/* __UIGFX_H__ */
