/*----------- defined in drivers/seibuspi.c -----------*/

extern uint32_t *spimainram;


/*----------- defined in machine/spisprit.c -----------*/

void seibuspi_sprite_decrypt(uint8_t *src, int romsize);


/*----------- defined in machine/seibuspi.c -----------*/

void seibuspi_text_decrypt(uint8_t *rom);
void seibuspi_bg_decrypt(uint8_t *rom, int size);

void seibuspi_rise10_text_decrypt(uint8_t *rom);
void seibuspi_rise10_bg_decrypt(uint8_t *rom, int size);
void seibuspi_rise10_sprite_decrypt(uint8_t *rom, int romsize);

void seibuspi_rise11_text_decrypt(uint8_t *rom);
void seibuspi_rise11_bg_decrypt(uint8_t *rom, int size);
void seibuspi_rise11_sprite_decrypt_rfjet(uint8_t *rom, int romsize);
void seibuspi_rise11_sprite_decrypt_feversoc(uint8_t *rom, int romsize);


/*----------- defined in video/seibuspi.c -----------*/

extern uint32_t *spi_scrollram;

VIDEO_START( spi );
VIDEO_UPDATE( spi );

VIDEO_START( sys386f2 );
VIDEO_UPDATE( sys386f2 );

READ32_HANDLER( spi_layer_bank_r );
WRITE32_HANDLER( spi_layer_bank_w );
WRITE32_HANDLER( spi_layer_enable_w );

void rf2_set_layer_banks(int banks);

WRITE32_HANDLER( tilemap_dma_start_w );
WRITE32_HANDLER( palette_dma_start_w );
WRITE32_HANDLER( video_dma_length_w );
WRITE32_HANDLER( video_dma_address_w );
WRITE32_HANDLER( sprite_dma_start_w );
