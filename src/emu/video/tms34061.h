/****************************************************************************
 *                                                                          *
 *  Function prototypes and constants used by the TMS34061 emulator         *
 *                                                                          *
 *  Created by Zsolt Vasvari on 5/26/1998.                                  *
 *  Updated by Aaron Giles on 11/21/2000.                                   *
 *                                                                          *
 ****************************************************************************/


/* register constants */
enum
{
	TMS34061_HORENDSYNC = 0,
	TMS34061_HORENDBLNK,
	TMS34061_HORSTARTBLNK,
	TMS34061_HORTOTAL,
	TMS34061_VERENDSYNC,
	TMS34061_VERENDBLNK,
	TMS34061_VERSTARTBLNK,
	TMS34061_VERTOTAL,
	TMS34061_DISPUPDATE,
	TMS34061_DISPSTART,
	TMS34061_VERINT,
	TMS34061_CONTROL1,
	TMS34061_CONTROL2,
	TMS34061_STATUS,
	TMS34061_XYOFFSET,
	TMS34061_XYADDRESS,
	TMS34061_DISPADDRESS,
	TMS34061_VERCOUNTER,
	TMS34061_REGCOUNT
};



/* interface structure */
struct tms34061_interface
{
	const char	*screen_tag;	/* the screen we are acting on */
	uint8_t		rowshift;		/* VRAM address is (row << rowshift) | col */
	uint32_t		vramsize;		/* size of video RAM */
	void		(*interrupt)(running_machine *machine, int state);	/* interrupt gen callback */
};


/* display state structure */
struct tms34061_display
{
	uint8_t	blanked;		/* true if blanked */
	uint8_t	*vram;			/* base of VRAM */
	uint8_t	*latchram;		/* base of latch RAM */
	uint16_t	*regs;			/* pointer to array of registers */
	offs_t	dispstart;		/* display start */
};


/* starts/stops the emulator */
void tms34061_start(running_machine *machine, const struct tms34061_interface *interface);

/* reads/writes to the 34061 */
uint8_t tms34061_r(const address_space *space, int col, int row, int func);
void tms34061_w(const address_space *space, int col, int row, int func, uint8_t data);

/* latch settings */
READ8_HANDLER( tms34061_latch_r );
WRITE8_HANDLER( tms34061_latch_w );

/* video update handling */
void tms34061_get_display_state(struct tms34061_display *state);
