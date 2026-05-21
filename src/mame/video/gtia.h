/***************************************************************************

    Atari 400/800

    GTIA  graphics television interface adapter

    Juergen Buchmueller, June 1998

***************************************************************************/

#ifndef __GTIA_H__
#define __GTIA_H__

#include "includes/atari.h"

typedef struct _gtia_interface gtia_interface;
struct _gtia_interface
{
	uint8_t (*console_read)(const address_space *space);
	void (*console_write)(const address_space *space, uint8_t data);
};




/* reading registers */
typedef struct _gtia_readregs gtia_readregs;
struct _gtia_readregs
{
	uint8_t	m0pf;		/* d000 missile 0 playfield collisions */
	uint8_t	m1pf;		/* d001 missile 1 playfield collisions */
	uint8_t	m2pf;		/* d002 missile 2 playfield collisions */
	uint8_t	m3pf;		/* d003 missile 3 playfield collisions */
	uint8_t	p0pf;		/* d004 player 0 playfield collisions */
	uint8_t	p1pf;		/* d005 player 1 playfield collisions */
	uint8_t	p2pf;		/* d006 player 2 playfield collisions */
	uint8_t	p3pf;		/* d007 player 3 playfield collisions */
	uint8_t	m0pl;		/* d008 missile 0 player collisions */
	uint8_t	m1pl;		/* d009 missile 1 player collisions */
	uint8_t	m2pl;		/* d00a missile 2 player collisions */
	uint8_t	m3pl;		/* d00b missile 3 player collisions */
	uint8_t	p0pl;		/* d00c player 0 player collisions */
	uint8_t	p1pl;		/* d00d player 1 player collisions */
	uint8_t	p2pl;		/* d00e player 2 player collisions */
	uint8_t	p3pl;		/* d00f player 3 player collisions */
	uint8_t	but[4];		/* d010-d013 button stick 0-3 */
	uint8_t	pal;		/* d014 PAL/NTSC config (D3,2,1 0=PAL, 1=NTSC */
	uint8_t	gtia15; 	/* d015 nothing */
	uint8_t	gtia16; 	/* d016 nothing */
	uint8_t	gtia17; 	/* d017 nothing */
	uint8_t	gtia18; 	/* d018 nothing */
	uint8_t	gtia19; 	/* d019 nothing */
	uint8_t	gtia1a; 	/* d01a nothing */
	uint8_t	gtia1b; 	/* d01b nothing */
	uint8_t	gtia1c; 	/* d01c nothing */
	uint8_t	gtia1d; 	/* d01d nothing */
	uint8_t	gtia1e; 	/* d01e nothing */
	uint8_t	cons;		/* d01f console keys */
};

/* writing registers */
typedef struct _gtia_writeregs gtia_writeregs;
struct _gtia_writeregs
{
	uint8_t	hposp0; 	/* d000 player 0 horz position */
	uint8_t	hposp1; 	/* d001 player 1 horz position */
	uint8_t	hposp2; 	/* d002 player 2 horz position */
	uint8_t	hposp3; 	/* d003 player 3 horz position */
	uint8_t	hposm0; 	/* d004 missile 0 horz position */
	uint8_t	hposm1; 	/* d005 missile 1 horz position */
	uint8_t	hposm2; 	/* d006 missile 2 horz position */
	uint8_t	hposm3; 	/* d007 missile 3 horz position */
	uint8_t	sizep0; 	/* d008 size player 0 */
	uint8_t	sizep1; 	/* d009 size player 1 */
	uint8_t	sizep2; 	/* d00a size player 2 */
	uint8_t	sizep3; 	/* d00b size player 3 */
    uint8_t   sizem;      /* d00c size missiles */
	uint8_t	grafp0[2];	/* d00d graphics data for player 0 */
	uint8_t	grafp1[2];	/* d00e graphics data for player 1 */
	uint8_t	grafp2[2];	/* d00f graphics data for player 2 */
	uint8_t	grafp3[2];	/* d010 graphics data for player 3 */
	uint8_t	grafm[2];	/* d011 graphics data for missiles */
	uint8_t	colpm0; 	/* d012 color for player/missile 0 */
	uint8_t	colpm1; 	/* d013 color for player/missile 1 */
	uint8_t	colpm2; 	/* d014 color for player/missile 2 */
	uint8_t	colpm3; 	/* d015 color for player/missile 3 */
	uint8_t	colpf0; 	/* d016 playfield color 0 */
	uint8_t	colpf1; 	/* d017 playfield color 1 */
	uint8_t	colpf2; 	/* d018 playfield color 2 */
	uint8_t	colpf3; 	/* d019 playfield color 3 */
	uint8_t	colbk;		/* d01a background playfield */
	uint8_t	prior;		/* d01b priority select */
	uint8_t	vdelay; 	/* d01c delay until vertical retrace */
	uint8_t	gractl; 	/* d01d graphics control */
	uint8_t	hitclr; 	/* d01e clear collisions */
	uint8_t	cons;		/* d01f write console (speaker) */
};

/* helpers */
typedef struct _gtia_helpervars gtia_helpervars;
struct _gtia_helpervars
{
	uint8_t	grafp0; 	/* optimized graphics data player 0 */
	uint8_t	grafp1; 	/* optimized graphics data player 1 */
	uint8_t	grafp2; 	/* optimized graphics data player 2 */
	uint8_t	grafp3; 	/* optimized graphics data player 3 */
	uint8_t	grafm0; 	/* optimized graphics data missile 0 */
	uint8_t	grafm1; 	/* optimized graphics data missile 1 */
	uint8_t	grafm2; 	/* optimized graphics data missile 2 */
	uint8_t	grafm3; 	/* optimized graphics data missile 3 */
	uint32_t	hitclr_frames;/* frames gone since last hitclr */
	uint8_t	sizem;		/* optimized size missiles */
	uint8_t	usedp;		/* mask for used player colors */
	uint8_t	usedm0; 	/* mask for used missile 0 color */
	uint8_t	usedm1; 	/* mask for used missile 1 color */
	uint8_t	usedm2; 	/* mask for used missile 2 color */
	uint8_t	usedm3; 	/* mask for used missile 3 color */
	uint8_t	vdelay_m0;	/* vertical delay for missile 0 */
	uint8_t	vdelay_m1;	/* vertical delay for missile 1 */
	uint8_t	vdelay_m2;	/* vertical delay for missile 2 */
	uint8_t	vdelay_m3;	/* vertical delay for missile 3 */
	uint8_t	vdelay_p0;	/* vertical delay for player 0 */
	uint8_t	vdelay_p1;	/* vertical delay for player 1 */
	uint8_t	vdelay_p2;	/* vertical delay for player 2 */
	uint8_t	vdelay_p3;	/* vertical delay for player 3 */
};

typedef struct _gtia_struct gtia_struct;
struct _gtia_struct
{
	gtia_interface intf;
	gtia_readregs	r;			/* read registers */
	gtia_writeregs	w;			/* write registers */
	gtia_helpervars	h;			/* helper variables */
};



extern gtia_struct gtia;

void gtia_init(running_machine *machine, const gtia_interface *intf);
READ8_HANDLER( atari_gtia_r );
WRITE8_HANDLER( atari_gtia_w );

ANTIC_RENDERER( gtia_mode_1_32 );
ANTIC_RENDERER( gtia_mode_1_40 );
ANTIC_RENDERER( gtia_mode_1_48 );
ANTIC_RENDERER( gtia_mode_2_32 );
ANTIC_RENDERER( gtia_mode_2_40 );
ANTIC_RENDERER( gtia_mode_2_48 );
ANTIC_RENDERER( gtia_mode_3_32 );
ANTIC_RENDERER( gtia_mode_3_40 );
ANTIC_RENDERER( gtia_mode_3_48 );
void gtia_render(VIDEO *video);

#endif /* __GTIA_H__ */
