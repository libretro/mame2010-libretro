/*************************************************************************

    Double Dragon & Double Dragon II (but also China Gate)

*************************************************************************/


class ddragon_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ddragon_state(machine)); }

	ddragon_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *        rambase;
	uint8_t *        bgvideoram;
	uint8_t *        fgvideoram;
	uint8_t *        spriteram;
	uint8_t *        scrollx_lo;
	uint8_t *        scrolly_lo;
	uint8_t *        darktowr_mcu_ports;
//  uint8_t *        paletteram;  // currently this uses generic palette handling
//  uint8_t *        paletteram_2;    // currently this uses generic palette handling
	size_t         spriteram_size;	// FIXME: this appears in chinagat.c, but is it really used?

	/* video-related */
	tilemap_t        *fg_tilemap, *bg_tilemap;
	uint8_t          technos_video_hw;
	uint8_t          scrollx_hi;
	uint8_t          scrolly_hi;

	/* misc */
	uint8_t          dd_sub_cpu_busy;
	uint8_t          sprite_irq, sound_irq, ym_irq, adpcm_sound_irq;
	uint32_t         adpcm_pos[2], adpcm_end[2];
	uint8_t          adpcm_idle[2];
	int            adpcm_data[2];

	/* for Sai Yu Gou Ma Roku */
	int            adpcm_addr;
	int            i8748_P1;
	int            i8748_P2;
	int            pcm_shift;
	int            pcm_nibble;
	int            mcu_command;
#if 0
	int            m5205_clk;
#endif

	/* devices */
	running_device *maincpu;
	running_device *snd_cpu;
	running_device *sub_cpu;
	running_device *adpcm_1;
	running_device *adpcm_2;
};


/*----------- defined in video/ddragon.c -----------*/

WRITE8_HANDLER( ddragon_bgvideoram_w );
WRITE8_HANDLER( ddragon_fgvideoram_w );

VIDEO_START( chinagat );
VIDEO_START( ddragon );
VIDEO_UPDATE( ddragon );

