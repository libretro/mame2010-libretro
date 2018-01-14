/******************************************************************************

    tiny.c

    mamedriv.c substitute file for "tiny" MAME builds.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    The list of used drivers. Drivers have to be included here to be recognized
    by the executable.

    To save some typing, we use a hack here. This file is recursively #included
    twice, with different definitions of the DRIVER() macro. The first one
    declares external references to the drivers; the second one builds an array
    storing all the drivers.

******************************************************************************/

#include "emu.h"

#ifndef DRIVER_RECURSIVE

#define DRIVER_RECURSIVE

/* step 1: declare all external references */
#define DRIVER(NAME) GAME_EXTERN(NAME);
#include "testdriv.c"

/* step 2: define the drivers[] array */
#undef DRIVER
#define DRIVER(NAME) &GAME_NAME(NAME),
const game_driver * const drivers[] =
{
#include "testdriv.c"
	0	/* end of array */
};

#else	/* DRIVER_RECURSIVE */
/* Neo Geo games */
	/* the four digits number is the game ID stored at address 0x0108 of the program ROM */
	/* info on prototypes taken from http://www.members.tripod.com/fresa/proto/puzzle.htm */
	DRIVER( neogeo )
	DRIVER( nam1975 )	/* 0001 (c) 1990 SNK */
	DRIVER( bstars )	/* 0002 (c) 1990 SNK */
	DRIVER( bstarsh )	/* 0002 (c) 1990 SNK */
	DRIVER( tpgolf )	/* 0003 (c) 1990 SNK */
	DRIVER( mahretsu )	/* 0004 (c) 1990 SNK */
	DRIVER( maglord )	/* 0005 (c) 1990 Alpha Denshi Co. */
	DRIVER( maglordh )	/* 0005 (c) 1990 Alpha Denshi Co. */
	DRIVER( ridhero )	/* 0006 (c) 1990 SNK */
	DRIVER( ridheroh )	/* 0006 (c) 1990 SNK */
	DRIVER( alpham2 )	/* 0007 (c) 1991 SNK */
						/* 0008 Sunshine (prototype) 1990 SNK */
	DRIVER( ncombat )	/* 0009 (c) 1990 Alpha Denshi Co. */
	DRIVER( ncombath )	/* 0009 (c) 1990 Alpha Denshi Co. */
	DRIVER( cyberlip )	/* 0010 (c) 1990 SNK */
	DRIVER( superspy )	/* 0011 (c) 1990 SNK */
						/* 0012 */
						/* 0013 */
	DRIVER( mutnat )	/* 0014 (c) 1992 SNK */
						/* 0015 */
	DRIVER( kotm )		/* 0016 (c) 1991 SNK */
	DRIVER( kotmh )		/* 0016 (c) 1991 SNK */
	DRIVER( sengoku )	/* 0017 (c) 1991 SNK */
	DRIVER( sengokh )	/* 0017 (c) 1991 SNK */
	DRIVER( burningf )	/* 0018 (c) 1991 SNK */
	DRIVER( burningfh )	/* 0018 (c) 1991 SNK */
	DRIVER( lbowling )	/* 0019 (c) 1990 SNK */
	DRIVER( gpilots )	/* 0020 (c) 1991 SNK */
	DRIVER( gpilotsh )	/* 0020 (c) 1991 SNK */
	DRIVER( joyjoy )	/* 0021 (c) 1990 SNK */
	DRIVER( bjourney )	/* 0022 (c) 1990 Alpha Denshi Co. */
	DRIVER( quizdais )	/* 0023 (c) 1991 SNK */

	DRIVER( lresort )	/* 0024 (c) 1992 SNK */
	DRIVER( eightman )	/* 0025 (c) 1991 SNK / Pallas */
						/* 0026 Fun Fun Brothers (prototype) 1991 Alpha */
	DRIVER( minasan )	/* 0027 (c) 1990 Monolith Corp. */
						/* 0028 Dunk Star (prototype) Sammy */
	DRIVER( legendos )	/* 0029 (c) 1991 SNK */
	DRIVER( 2020bb )	/* 0030 (c) 1991 SNK / Pallas */
	DRIVER( 2020bba )	/* 0030 (c) 1991 SNK / Pallas */
	DRIVER( 2020bbh )	/* 0030 (c) 1991 SNK / Pallas */
	DRIVER( socbrawl )	/* 0031 (c) 1991 SNK */

	DRIVER( roboarmy )	/* 0032 (c) 1991 SNK */
	DRIVER( fatfury1 )	/* 0033 (c) 1991 SNK */
	DRIVER( fbfrenzy )	/* 0034 (c) 1992 SNK */
						/* 0035 Mystic Wand (prototype) 1991 Alpha */
	DRIVER( bakatono )	/* 0036 (c) 1991 Monolith Corp. */
	DRIVER( crsword )	/* 0037 (c) 1991 Alpha Denshi Co. */
	DRIVER( trally )	/* 0038 (c) 1991 Alpha Denshi Co. */
	DRIVER( kotm2 )		/* 0039 (c) 1992 SNK */
	DRIVER( sengoku2 )	/* 0040 (c) 1993 SNK */
	DRIVER( bstars2 )	/* 0041 (c) 1992 SNK */
	DRIVER( quizdai2 )	/* 0042 (c) 1992 SNK */
	DRIVER( 3countb )	/* 0043 (c) 1993 SNK */
	DRIVER( aof )		/* 0044 (c) 1992 SNK */
	DRIVER( samsho )	/* 0045 (c) 1993 SNK */
	DRIVER( samshoh )	/* 0045 (c) 1993 SNK */
	DRIVER( tophuntr )	/* 0046 (c) 1994 SNK */
	DRIVER( tophuntrh )	/* 0046 (c) 1994 SNK */
	DRIVER( fatfury2 )	/* 0047 (c) 1992 SNK */
	DRIVER( janshin )	/* 0048 (c) 1994 Aicom */
	DRIVER( androdun )	/* 0049 (c) 1992 Visco */
	DRIVER( ncommand )	/* 0050 (c) 1992 Alpha Denshi Co. */
	DRIVER( viewpoin )	/* 0051 (c) 1992 Sammy */
	DRIVER( ssideki )	/* 0052 (c) 1992 SNK */
	DRIVER( wh1 )		/* 0053 (c) 1992 Alpha Denshi Co. */
	DRIVER( wh1h )		/* 0053 (c) 1992 Alpha Denshi Co. */
	DRIVER( wh1ha )		/* 0053 (c) 1992 Alpha Denshi Co. */
						/* 0054 Crossed Swords 2  (CD only? not confirmed, MVS might exist) */
	DRIVER( kof94 )		/* 0055 (c) 1994 SNK */
	DRIVER( aof2 )		/* 0056 (c) 1994 SNK */
	DRIVER( aof2a )		/* 0056 (c) 1994 SNK */
	DRIVER( wh2 )		/* 0057 (c) 1993 ADK */
	DRIVER( fatfursp )	/* 0058 (c) 1993 SNK */
	DRIVER( fatfursa )	/* 0058 (c) 1993 SNK */
	DRIVER( savagere )	/* 0059 (c) 1995 SNK */
	DRIVER( fightfev )	/* 0060 (c) 1994 Viccom */
	DRIVER( fightfeva )	/* 0060 (c) 1994 Viccom */
	DRIVER( ssideki2 )	/* 0061 (c) 1994 SNK */
	DRIVER( spinmast )	/* 0062 (c) 1993 Data East Corporation */
	DRIVER( samsho2 )	/* 0063 (c) 1994 SNK */
	DRIVER( samsho2k )	/* 0063 (c) 1994 SNK (Korean hack) */
	DRIVER( wh2j )		/* 0064 (c) 1994 ADK / SNK */
	DRIVER( wh2jh )		/* 0064 (c) 1994 ADK / SNK */
	DRIVER( wjammers )	/* 0065 (c) 1994 Data East Corporation */
	DRIVER( karnovr )	/* 0066 (c) 1994 Data East Corporation */
	DRIVER( gururin )	/* 0067 (c) 1994 Face */
	DRIVER( pspikes2 )	/* 0068 (c) 1994 Video System Co. */
						/* Super Volley '94 was once released in Mar.1994, and recalled. Then released as Power Spikes 2 (with some tweaks). */
	DRIVER( fatfury3 )	/* 0069 (c) 1995 SNK */
	DRIVER( zupapa )	/* 0070 Zupapa - released in 2001, 1994 prototype probably exists */
						/* 0071 Bang Bang Busters (prototype) 1994 Visco */
						/* 0072 Last Odyssey Pinball Fantasia (prototype) 1995 Monolith */
	DRIVER( panicbom )	/* 0073 (c) 1994 Eighting / Hudson */
	DRIVER( aodk )		/* 0074 (c) 1994 ADK / SNK */
	DRIVER( sonicwi2 )	/* 0075 (c) 1994 Video System Co. */
	DRIVER( zedblade )	/* 0076 (c) 1994 NMK */
						/* 0077 The Warlocks of the Fates (prototype) 1995 Astec */
	DRIVER( galaxyfg )	/* 0078 (c) 1995 Sunsoft */
	DRIVER( strhoop )	/* 0079 (c) 1994 Data East Corporation */
	DRIVER( quizkof )	/* 0080 (c) 1995 Saurus */

	DRIVER( ssideki3 )	/* 0081 (c) 1995 SNK */
	DRIVER( doubledr )	/* 0082 (c) 1995 Technos */
	DRIVER( pbobblen )	/* 0083 (c) 1994 Taito */
	DRIVER( pbobblenb )	/* bootleg */
	DRIVER( kof95 )		/* 0084 (c) 1995 SNK */
	DRIVER( kof95h )	/* 0084 (c) 1995 SNK */
						/* 0085 Shinsetsu Samurai Spirits Bushidoretsuden / Samurai Shodown RPG (CD only) */
	DRIVER( tws96 )		/* 0086 (c) 1996 Tecmo */
	DRIVER( samsho3 )	/* 0087 (c) 1995 SNK */
	DRIVER( samsho3h )	/* 0087 (c) 1995 SNK */
	DRIVER( fswords )	/* 0187 Korean hack of samsho3 */
	DRIVER( stakwin )	/* 0088 (c) 1995 Saurus */
	DRIVER( pulstar )	/* 0089 (c) 1995 Aicom */
	DRIVER( whp )		/* 0090 (c) 1995 ADK / SNK */
						/* 0091 */
	DRIVER( kabukikl )	/* 0092 (c) 1995 Hudson */
	DRIVER( neobombe )	/* 0093 (c) 1997 Hudson */
	DRIVER( gowcaizr )	/* 0094 (c) 1995 Technos */
	DRIVER( rbff1 )		/* 0095 (c) 1995 SNK */
	DRIVER( rbff1a )	/* 0095 (c) 1995 SNK */
	DRIVER( aof3 )		/* 0096 (c) 1996 SNK */
	DRIVER( aof3k )		/* 0196 Censored Korean release of aof3 */
	DRIVER( sonicwi3 )	/* 0097 (c) 1995 Video System Co. */
						/* 0098 Idol Mahjong - final romance 2 (CD only? not confirmed, MVS might exist) */
						/* 0099 Neo Pool Masters */
	DRIVER( turfmast )	/* 0200 (c) 1996 Nazca */
	DRIVER( mslug )		/* 0201 (c) 1996 Nazca */
	DRIVER( puzzledp )	/* 0202 (c) 1995 Taito (Visco license) */
	DRIVER( mosyougi )	/* 0203 (c) 1995 ADK / SNK */
						/* 0204 QP (prototype) */
						/* 0205 Neo-Geo CD Special (CD only) */
	DRIVER( marukodq )	/* 0206 (c) 1995 Takara */
	DRIVER( neomrdo )	/* 0207 (c) 1996 Visco */
	DRIVER( sdodgeb )	/* 0208 (c) 1996 Technos */
	DRIVER( goalx3 )	/* 0209 (c) 1995 Visco */
						/* 0210 Karate Ninja Sho (prototype) 1995 Yumekobo */
						/* 0211 Oshidashi Zintrick (CD only? not confirmed, MVS might exist) 1996 SNK/ADK */
	DRIVER( zintrckb )	/* 0211 hack - this is not a genuine MVS proto, its a bootleg made from the CD version */
	DRIVER( overtop )	/* 0212 (c) 1996 ADK */
	DRIVER( neodrift )	/* 0213 (c) 1996 Visco */
	DRIVER( kof96 )		/* 0214 (c) 1996 SNK */
	DRIVER( kof96h )	/* 0214 (c) 1996 SNK */
	DRIVER( ssideki4 )	/* 0215 (c) 1996 SNK */
	DRIVER( kizuna )	/* 0216 (c) 1996 SNK */
						/*  Fu-un Super Tag Battle Special Version (4-player battle available) exists */
	DRIVER( ninjamas )	/* 0217 (c) 1996 ADK / SNK */
	DRIVER( ragnagrd )	/* 0218 (c) 1996 Saurus */
	DRIVER( pgoal )		/* 0219 (c) 1996 Saurus */
						/* 0220 (c) 1996 Saurus - Choutetsu Brikin'ger - Iron clad (undumped protoype) */
	DRIVER( magdrop2 )	/* 0221 (c) 1996 Data East Corporation */
	DRIVER( samsho4 )	/* 0222 (c) 1996 SNK */
	DRIVER( samsho4k )	/* Censored Korean release of samsho4 */
	DRIVER( rbffspec )	/* 0223 (c) 1996 SNK */

	DRIVER( twinspri )	/* 0224 (c) 1996 ADK */
	DRIVER( wakuwak7 )	/* 0225 (c) 1996 Sunsoft */
						/* 0226 Pair Pair Wars (prototype) 1996 Sunsoft? */
	DRIVER( stakwin2 )	/* 0227 (c) 1996 Saurus */
	DRIVER( ghostlop )	/* 0228 GhostLop (prototype) 1996? Data East */
						/* 0229 King of Fighters '96 CD Collection (CD only) */
	DRIVER( breakers )	/* 0230 (c) 1996 Visco */
	DRIVER( miexchng )	/* 0231 (c) 1997 Face */
	DRIVER( kof97 )		/* 0232 (c) 1997 SNK */
	DRIVER( kof97h )	/* 0232 (c) 1997 SNK */
	DRIVER( kof97pls )	/* bootleg of kof97 */
	DRIVER( kog )		/* bootleg of kof97 */
	DRIVER( magdrop3 )	/* 0233 (c) 1997 Data East Corporation */
	DRIVER( lastblad )	/* 0234 (c) 1997 SNK */
	DRIVER( lastbladh )	/* 0234 (c) 1997 SNK */
	DRIVER( lastsold )	/* 0196 Censored Korean release of lastblad */
	DRIVER( puzzldpr )	/* 0235 (c) 1997 Taito (Visco license) */
	DRIVER( irrmaze )	/* 0236 (c) 1997 SNK / Saurus */
	DRIVER( popbounc )	/* 0237 (c) 1997 Video System Co. */
	DRIVER( shocktro )	/* 0238 (c) 1997 Saurus */
	DRIVER( shocktra )	/* 0238 (c) 1997 Saurus */
	DRIVER( blazstar )	/* 0239 (c) 1998 Yumekobo */
	DRIVER( rbff2 )		/* 0240 (c) 1998 SNK */
	DRIVER( rbff2h )	/* 0240 (c) 1998 SNK */
	DRIVER( rbff2k )	/* 0140 Censored Korean release of rbff2 */
	DRIVER( mslug2 )	/* 0241 (c) 1998 SNK */
	DRIVER( kof98 )		/* 0242 (c) 1998 SNK */
	DRIVER( kof98k )	/* 0242 (c) 1998 SNK */


	DRIVER( lastbld2 )	/* 0243 (c) 1998 SNK */
	DRIVER( neocup98 )	/* 0244 (c) 1998 SNK */
	DRIVER( breakrev )	/* 0245 (c) 1998 Visco */
	DRIVER( shocktr2 )	/* 0246 (c) 1998 Saurus */
	DRIVER( lans2004 )	/* bootleg of shocktr2 */
	DRIVER( flipshot )	/* 0247 (c) 1998 Visco */
	DRIVER( pbobbl2n )	/* 0248 (c) 1999 Taito (SNK license) */
	DRIVER( ctomaday )	/* 0249 (c) 1999 Visco */
	DRIVER( mslugx )	/* 0250 (c) 1999 SNK */
	DRIVER( kof99 )		/* 0251 (c) 1999 SNK */
	DRIVER( kof99a )	/* 0251 (c) 1999 SNK */
	DRIVER( kof99e )	/* 0251 (c) 1999 SNK */

	DRIVER( kof99p )	/* 0251 (c) 1999 SNK */
	DRIVER( ganryu )	/* 0252 (c) 1999 Visco */
	DRIVER( garou )		/* 0253 (c) 1999 SNK */
	DRIVER( garouo )	/* 0253 (c) 1999 SNK */
	DRIVER( garoup )	/* 0253 (c) 1999 SNK */
	DRIVER( garoubl )	/* bootleg */
	DRIVER( s1945p )	/* 0254 (c) 1999 Psikyo */
	DRIVER( preisle2 )	/* 0255 (c) 1999 Yumekobo */
	DRIVER( mslug3 )	/* 0256 (c) 2000 SNK */
	DRIVER( mslug3h )	/* 0256 (c) 2000 SNK */
	DRIVER( mslug3b6 )	/* bootleg */
	DRIVER( kof2000 )	/* 0257 (c) 2000 SNK */
	DRIVER( kof2000n )	/* 0257 (c) 2000 SNK */
						/* 0258 SNK vs. Capcom? */
	DRIVER( bangbead )	/* 0259 (c) 2000 Visco */
	DRIVER( nitd )		/* 0260 (c) 2000 Eleven / Gavaking */
	DRIVER( nitdbl )	/* bootleg */
	DRIVER( sengoku3 )	/* 0261 (c) 2001 Noise Factory / SNK */
	DRIVER( kof2001 )	/* 0262 (c) 2001 Eolith / SNK */
	DRIVER( kof2001h )	/* 0262 (c) 2001 Eolith / SNK */
	DRIVER( cthd2003 )	/* bootleg of kof2001 */
	DRIVER( ct2k3sp )	/* bootleg of kof2001 */
	DRIVER( ct2k3sa )	/* bootleg of kof2001 */
	DRIVER( mslug4 )	/* 0263 (c) 2002 Mega Enterprise */
	DRIVER( mslug4h )	/* 0263 (c) 2002 Mega Enterprise */
	DRIVER( ms4plus )	/* bootleg */
	DRIVER( rotd )		/* 0264 (c) 2002 Evoga */
	DRIVER( kof2002 )	/* 0265 (c) 2002 Eolith / Playmore */
	DRIVER( kof2002b )	/* bootleg */
	DRIVER( kf2k2pls )	/* bootleg */
	DRIVER( kf2k2pla )	/* bootleg */
	DRIVER( kf2k2mp )	/* bootleg */
	DRIVER( kf2k2mp2 )	/* bootleg */
	DRIVER( kof10th )	/* bootleg of kof2002 */
	DRIVER( kf2k5uni )	/* bootleg of kof2002 */
	DRIVER( kf10thep )	/* bootleg of kof2002 */
	DRIVER( kof2k4se )	/* bootleg of kof2002 */
	DRIVER( matrim )	/* 0266 (c) 2002 Atlus */
	DRIVER( matrimbl )	/* bootleg */
	DRIVER( pnyaa )		/* 0267 (c) 2003 Aiky / Taito */
	DRIVER( ms5pcb )	/* 0268 (c) 2003 Playmore */
	DRIVER( mslug5 )	/* 0268 (c) 2003 Playmore */
	DRIVER( mslug5h )	/* 0268 (c) 2003 Playmore */
	DRIVER( ms5plus )	/* bootleg */
	DRIVER( svcpcb )	/* 0269 (c) 2003 Playmore / Capcom - JAMMA PCB */
	DRIVER( svcpcba )	/* 0269 (c) 2003 Playmore / Capcom - JAMMA PCB */
	DRIVER( svc )		/* 0269 (c) 2003 Playmore / Capcom */
	DRIVER( svcboot )	/* bootleg */
	DRIVER( svcplus )	/* bootleg */
	DRIVER( svcplusa )	/* bootleg */
	DRIVER( svcsplus )	/* bootleg */
	DRIVER( samsho5 )	/* 0270 (c) 2003 Playmore */
	DRIVER( samsho5h )	/* 0270 (c) 2003 Playmore */
	DRIVER( samsho5b )	/* bootleg */
	DRIVER( kf2k3pcb )	/* 0271 (c) 2003 Playmore - JAMMA PCB */
	DRIVER( kof2003 )	/* 0271 (c) 2003 Playmore */
	DRIVER( kof2003h )	/* 0271 (c) 2003 Playmore */
	DRIVER( kf2k3bl )	/* bootleg */
	DRIVER( kf2k3bla )	/* bootleg */
	DRIVER( kf2k3pl )	/* bootleg */
	DRIVER( kf2k3upl )	/* bootleg */
	DRIVER( samsh5sp )	/* 0272 (c) 2004 Playmore */
	DRIVER( samsh5sph )	/* 0272 (c) 2004 Playmore */
	DRIVER( samsh5spn )	/* 0272 (c) 2004 Playmore */
	DRIVER( robby )		/* (c) 1981 Bally Midway */
	DRIVER( gridlee )	/* [1983 Videa] prototype - no copyright notice */
	DRIVER( alienar )	/* (c) 1985 Duncan Brown */
	DRIVER( carpolo )	/* (c) 1977 Exidy */
	DRIVER( sidetrac )	/* (c) 1979 Exidy */
	DRIVER( targ )		/* (c) 1980 Exidy */
	DRIVER( spectar )	/* (c) 1980 Exidy */
	DRIVER( teetert )	/* (c) 1982 Exidy */
	DRIVER( circus )	/* (c) 1977 Exidy */
	DRIVER( robotbwl )	/* (c) 197? Exidy */
	DRIVER( crash )		/* (c) 1979 Exidy */
	DRIVER( ripcord )	/* (c) 1979 Exidy */
	DRIVER( starfire )	/* (c) 1979 Exidy */
	DRIVER( starfirea )	/* (c) 1979 Exidy */
	DRIVER( fireone )	/* (c) 1979 Exidy */
	DRIVER( starfir2 )	/* (c) 1979 Exidy */
	DRIVER( wrally )	/* (c) 1993 - Ref 930705 */

#endif	/* DRIVER_RECURSIVE */
