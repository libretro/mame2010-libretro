/*----------- defined in machine/wrally.c -----------*/

WRITE16_HANDLER( wrally_vram_w );
WRITE16_HANDLER( wrally_flipscreen_w );
WRITE16_HANDLER( OKIM6295_bankswitch_w );
WRITE16_HANDLER( wrally_coin_counter_w );
WRITE16_HANDLER( wrally_coin_lockout_w );

/*----------- defined in video/wrally.c -----------*/

extern tilemap_t *wrally_pant[2];

extern uint16_t *wrally_vregs;
extern uint16_t *wrally_videoram;
extern uint16_t *wrally_spriteram;

VIDEO_START( wrally );
VIDEO_UPDATE( wrally );

