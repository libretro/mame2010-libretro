
class fitfight_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, fitfight_state(machine)); }

	fitfight_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *  fof_100000;
	uint16_t *  fof_600000;
	uint16_t *  fof_700000;
	uint16_t *  fof_800000;
	uint16_t *  fof_900000;
	uint16_t *  fof_a00000;
	uint16_t *  fof_bak_tileram;
	uint16_t *  fof_mid_tileram;
	uint16_t *  fof_txt_tileram;
	uint16_t *  spriteram;
//  uint16_t *  paletteram;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *fof_bak_tilemap, *fof_mid_tilemap, *fof_txt_tilemap;

	/* misc */
	int      bbprot_kludge;
	uint16_t   fof_700000_data;
};


/*----------- defined in video/fitfight.c -----------*/

WRITE16_HANDLER( fof_bak_tileram_w );
WRITE16_HANDLER( fof_mid_tileram_w );
WRITE16_HANDLER( fof_txt_tileram_w );

VIDEO_START(fitfight);
VIDEO_UPDATE(fitfight);
