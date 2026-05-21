/*----------- defined in drivers/suprnova.c -----------*/

extern uint32_t *skns_tilemapA_ram, *skns_tilemapB_ram, *skns_v3slc_ram;
extern uint32_t *skns_palette_ram;
extern uint32_t *skns_pal_regs, *skns_v3_regs, *skns_spc_regs;

/*----------- defined in video/suprnova.c -----------*/

extern int suprnova_alt_enable_sprites;

void skns_sprite_kludge(int x, int y);
void skns_draw_sprites(
	running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect,
	uint32_t* spriteram_source, size_t spriteram_size,
	uint8_t* gfx_source, size_t gfx_length,
	uint32_t* sprite_regs );

WRITE32_HANDLER ( skns_tilemapA_w );
WRITE32_HANDLER ( skns_tilemapB_w );
WRITE32_HANDLER ( skns_v3_regs_w );
WRITE32_HANDLER ( skns_pal_regs_w );
WRITE32_HANDLER ( skns_palette_ram_w );
VIDEO_START(skns);
VIDEO_RESET(skns);
VIDEO_EOF(skns);
VIDEO_UPDATE(skns);
