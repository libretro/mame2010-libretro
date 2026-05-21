/*----------- defined in drivers/tatsumi.c -----------*/

extern uint16_t *apache3_g_ram;
extern uint16_t bigfight_a40000[2];

extern uint8_t* tatsumi_rom_sprite_lookup1;
extern uint8_t* tatsumi_rom_sprite_lookup2;
extern uint8_t* tatsumi_rom_clut0;
extern uint8_t* tatsumi_rom_clut1;

extern uint16_t *roundup5_d0000_ram, *roundup5_e0000_ram;
extern uint16_t *roundup5_unknown0, *roundup5_unknown1, *roundup5_unknown2;

/*----------- defined in machine/tatsumi.c -----------*/

READ16_HANDLER( apache3_bank_r );
WRITE16_HANDLER( apache3_bank_w );
WRITE16_HANDLER( apache3_z80_ctrl_w );
READ16_HANDLER( apache3_v30_v20_r );
WRITE16_HANDLER( apache3_v30_v20_w );
READ16_HANDLER( roundup_v30_z80_r );
WRITE16_HANDLER( roundup_v30_z80_w );
READ16_HANDLER( tatsumi_v30_68000_r );
WRITE16_HANDLER( tatsumi_v30_68000_w ) ;
READ16_HANDLER( apache3_z80_r );
WRITE16_HANDLER( apache3_z80_w );
READ8_HANDLER( apache3_adc_r );
WRITE8_HANDLER( apache3_adc_w );
WRITE16_HANDLER( apache3_rotate_w );
WRITE16_HANDLER( cyclwarr_control_w );
READ16_HANDLER( cyclwarr_control_r );
WRITE16_HANDLER( roundup5_control_w );
WRITE16_HANDLER( roundup5_d0000_w );
WRITE16_HANDLER( roundup5_e0000_w );

READ8_DEVICE_HANDLER(tatsumi_hack_ym2151_r);
READ8_DEVICE_HANDLER(tatsumi_hack_oki_r);

extern uint16_t *tatsumi_68k_ram;
extern uint8_t *apache3_z80_ram;
extern uint16_t tatsumi_control_word;
extern uint16_t apache3_rotate_ctrl[12];

void tatsumi_reset(running_machine *machine);

/*----------- defined in video/tatsumi.c -----------*/

WRITE16_HANDLER( roundup5_palette_w );
WRITE16_HANDLER( tatsumi_sprite_control_w );
WRITE16_HANDLER( roundup5_text_w );
WRITE16_HANDLER( roundup5_crt_w );
READ16_HANDLER( cyclwarr_videoram0_r );
WRITE16_HANDLER( cyclwarr_videoram0_w );
READ16_HANDLER( cyclwarr_videoram1_r );
WRITE16_HANDLER( cyclwarr_videoram1_w );
READ16_HANDLER( roundup5_vram_r );
WRITE16_HANDLER( roundup5_vram_w );
WRITE16_HANDLER( apache3_palette_w );
WRITE8_HANDLER( apache3_road_x_w );
WRITE16_HANDLER( apache3_road_z_w );

extern uint16_t* tatsumi_sprite_control_ram;
extern uint16_t *cyclwarr_videoram0, *cyclwarr_videoram1;
extern uint16_t *roundup_r_ram, *roundup_p_ram, *roundup_l_ram;

VIDEO_START( apache3 );
VIDEO_START( roundup5 );
VIDEO_START( cyclwarr );
VIDEO_START( bigfight );

VIDEO_UPDATE( roundup5 );
VIDEO_UPDATE( apache3 );
VIDEO_UPDATE( cyclwarr );
VIDEO_UPDATE( bigfight );

