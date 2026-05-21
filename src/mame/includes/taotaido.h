/*----------- defined in video/taotaido.c -----------*/

extern uint16_t *taotaido_spriteram;
extern uint16_t *taotaido_spriteram2;
extern uint16_t *taotaido_scrollram;
extern uint16_t *taotaido_bgram;

WRITE16_HANDLER( taotaido_sprite_character_bank_select_w );
WRITE16_HANDLER( taotaido_tileregs_w );
WRITE16_HANDLER( taotaido_bgvideoram_w );
VIDEO_START( taotaido );
VIDEO_UPDATE( taotaido );
VIDEO_EOF( taotaido );
