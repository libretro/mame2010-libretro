class sslam_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, sslam_state(machine)); }

	sslam_state(running_machine &machine) { }

	emu_timer *music_timer;

	int sound;
	int melody;
	int bar;
	int track;
	int snd_bank;

	uint16_t *bg_tileram;
	uint16_t *tx_tileram;
	uint16_t *md_tileram;
	uint16_t *spriteram;
	uint16_t *regs;

	uint8_t oki_control;
	uint8_t oki_command;
	uint8_t oki_bank;

	tilemap_t *bg_tilemap;
	tilemap_t *tx_tilemap;
	tilemap_t *md_tilemap;

	int sprites_x_offset;
};


/*----------- defined in video/sslam.c -----------*/

WRITE16_HANDLER( sslam_tx_tileram_w );
WRITE16_HANDLER( sslam_md_tileram_w );
WRITE16_HANDLER( sslam_bg_tileram_w );
WRITE16_HANDLER( powerbls_bg_tileram_w );
VIDEO_START(sslam);
VIDEO_START(powerbls);
VIDEO_UPDATE(sslam);
VIDEO_UPDATE(powerbls);
