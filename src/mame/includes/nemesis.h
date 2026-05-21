class nemesis_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, nemesis_state(machine)); }

	nemesis_state(running_machine &machine) { }

	/* memory pointers */
	uint16_t *  videoram1;
	uint16_t *  videoram2;
	uint16_t *  colorram1;
	uint16_t *  colorram2;
	uint16_t *  charram;
	uint16_t *  spriteram;
	uint16_t *  paletteram;
	uint16_t *  xscroll1;
	uint16_t *  xscroll2;
	uint16_t *  yscroll1;
	uint16_t *  yscroll2;
	uint8_t *   gx400_shared_ram;

	size_t    charram_size;
	size_t    spriteram_size;

	/* video-related */
	tilemap_t *background, *foreground;
	int       spriteram_words;
	int       tilemap_flip;
	int       flipscreen;
	uint8_t     irq_port_last;
	uint8_t     blank_tile[8*8];

	/* misc */
	int       irq_on;
	int       irq1_on;
	int       irq2_on;
	int       irq4_on;
	uint16_t    selected_ip; /* Copied from WEC Le Mans 24 driver, explicity needed for Hyper Crash */
	int       gx400_irq1_cnt;
	uint8_t     frame_counter;

	/* devices */
	cpu_device *maincpu;
	cpu_device *audiocpu;
	running_device *vlm;
};


/*----------- defined in video/nemesis.c -----------*/

WRITE16_HANDLER( nemesis_gfx_flipx_word_w );
WRITE16_HANDLER( nemesis_gfx_flipy_word_w );
WRITE16_HANDLER( salamand_control_port_word_w );
WRITE16_HANDLER( salamander_palette_word_w );
WRITE16_HANDLER( nemesis_palette_word_w );

WRITE16_HANDLER( nemesis_videoram1_word_w );
WRITE16_HANDLER( nemesis_videoram2_word_w );
WRITE16_HANDLER( nemesis_colorram1_word_w );
WRITE16_HANDLER( nemesis_colorram2_word_w );
WRITE16_HANDLER( nemesis_charram_word_w );

VIDEO_START( nemesis );
VIDEO_UPDATE( nemesis );
