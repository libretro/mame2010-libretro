/***************************************************************************

    1943

***************************************************************************/

class _1943_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, _1943_state(machine)); }

	_1943_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t * videoram;
	uint8_t * colorram;
	uint8_t * spriteram;
	uint8_t * scrollx;
	uint8_t * scrolly;
	uint8_t * bgscrollx;
	size_t  spriteram_size;

	/* video-related */
	tilemap_t *fg_tilemap, *bg_tilemap, *bg2_tilemap;
	int     char_on, obj_on, bg1_on, bg2_on;
};



/*----------- defined in video/1943.c -----------*/

extern WRITE8_HANDLER( c1943_c804_w );
extern WRITE8_HANDLER( c1943_d806_w );
extern WRITE8_HANDLER( c1943_videoram_w );
extern WRITE8_HANDLER( c1943_colorram_w );

extern PALETTE_INIT( 1943 );
extern VIDEO_START( 1943 );
extern VIDEO_UPDATE( 1943 );
