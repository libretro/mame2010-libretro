#include "devlegcy.h"

#ifdef MAME_DEBUG
#define LOGLEVEL  5
#else
#define LOGLEVEL  0
#endif
#define LOG(n,x)  do { if (LOGLEVEL >= n) logerror x; } while (0)


DECLARE_LEGACY_DEVICE(DECOCASS_TAPE, decocass_tape);

#define MDRV_DECOCASS_TAPE_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, DECOCASS_TAPE, 0)




class decocass_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, decocass_state(machine)); }

	decocass_state(running_machine &machine) { }

	/* memory pointers */
	uint8_t *   rambase;
	uint8_t *   charram;
	uint8_t *   fgvideoram;
	uint8_t *   colorram;
	uint8_t *   bgvideoram;	/* shares bits D0-3 with tileram! */
	uint8_t *   tileram;
	uint8_t *   objectram;
	uint8_t *   paletteram;
	size_t    fgvideoram_size;
	size_t    colorram_size;
	size_t    bgvideoram_size;
	size_t    tileram_size;
	size_t    objectram_size;

	/* video-related */
	tilemap_t   *fg_tilemap, *bg_tilemap_l, *bg_tilemap_r;
	int32_t     watchdog_count;
	int32_t     watchdog_flip;
	int32_t     color_missiles;
	int32_t     color_center_bot;
	int32_t     mode_set;
	int32_t     back_h_shift;
	int32_t     back_vl_shift;
	int32_t     back_vr_shift;
	int32_t     part_h_shift;
	int32_t     part_v_shift;
	int32_t     center_h_shift_space;
	int32_t     center_v_shift;
	rectangle bg_tilemap_l_clip;
	rectangle bg_tilemap_r_clip;

	/* sound-related */
	uint8_t     sound_ack;	/* sound latches, ACK status bits and NMI timer */
	uint8_t     audio_nmi_enabled;
	uint8_t     audio_nmi_state;

	/* misc */
	uint8_t     *decrypted;
	uint8_t     *decrypted2;
	int32_t     firsttime;
	uint8_t     latch1;
	uint8_t     decocass_reset;
	int32_t     de0091_enable;	/* DE-0091xx daughter board enable */
	uint8_t     quadrature_decoder[4];	/* four inputs from the quadrature decoder (H1, V1, H2, V2) */
	int       showmsg;		// for debugging purposes

	/* i8041 */
	uint8_t     i8041_p1;
	uint8_t     i8041_p2;
	int       i8041_p1_write_latch, i8041_p1_read_latch;
	int       i8041_p2_write_latch, i8041_p2_read_latch;

	/* dongles-related */
	read8_space_func    dongle_r;
	write8_space_func   dongle_w;

	/* dongle type #1 */
	uint32_t    type1_inmap;
	uint32_t    type1_outmap;

	/* dongle type #2: status of the latches */
	int32_t     type2_d2_latch;	/* latched 8041-STATUS D2 value */
	int32_t     type2_xx_latch;	/* latched value (D7-4 == 0xc0) ? 1 : 0 */
	int32_t     type2_promaddr;	/* latched PROM address A0-A7 */

	/* dongle type #3: status and patches */
	int32_t     type3_ctrs;		/* 12 bit counter stage */
	int32_t     type3_d0_latch;	/* latched 8041-D0 value */
	int32_t     type3_pal_19;		/* latched 1 for PAL input pin-19 */
	int32_t     type3_swap;

	/* dongle type #4: status */
	int32_t     type4_ctrs;		/* latched PROM address (E5x0 LSB, E5x1 MSB) */
	int32_t     type4_latch;		/* latched enable PROM (1100xxxx written to E5x1) */

	/* dongle type #5: status */
	int32_t     type5_latch;		/* latched enable PROM (1100xxxx written to E5x1) */

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *mcu;
	running_device *cassette;
};



extern WRITE8_HANDLER( decocass_coin_counter_w );
extern WRITE8_HANDLER( decocass_sound_command_w );
extern READ8_HANDLER( decocass_sound_data_r );
extern READ8_HANDLER( decocass_sound_ack_r );
extern WRITE8_HANDLER( decocass_sound_data_w );
extern READ8_HANDLER( decocass_sound_command_r );
extern TIMER_DEVICE_CALLBACK( decocass_audio_nmi_gen );
extern WRITE8_HANDLER( decocass_sound_nmi_enable_w );
extern READ8_HANDLER( decocass_sound_nmi_enable_r );
extern READ8_HANDLER( decocass_sound_data_ack_reset_r );
extern WRITE8_HANDLER( decocass_sound_data_ack_reset_w );
extern WRITE8_HANDLER( decocass_nmi_reset_w );
extern WRITE8_HANDLER( decocass_quadrature_decoder_reset_w );
extern WRITE8_HANDLER( decocass_adc_w );
extern READ8_HANDLER( decocass_input_r );

extern WRITE8_HANDLER( decocass_reset_w );

extern READ8_HANDLER( decocass_e5xx_r );
extern WRITE8_HANDLER( decocass_e5xx_w );
extern WRITE8_HANDLER( decocass_de0091_w );
extern WRITE8_HANDLER( decocass_e900_w );

extern MACHINE_START( decocass );
extern MACHINE_RESET( decocass );
extern MACHINE_RESET( ctsttape );
extern MACHINE_RESET( chwy );
extern MACHINE_RESET( clocknch );
extern MACHINE_RESET( ctisland );
extern MACHINE_RESET( csuperas );
extern MACHINE_RESET( castfant );
extern MACHINE_RESET( cluckypo );
extern MACHINE_RESET( cterrani );
extern MACHINE_RESET( cexplore );
extern MACHINE_RESET( cprogolf );
extern MACHINE_RESET( cmissnx );
extern MACHINE_RESET( cdiscon1 );
extern MACHINE_RESET( cptennis );
extern MACHINE_RESET( ctornado );
extern MACHINE_RESET( cbnj );
extern MACHINE_RESET( cburnrub );
extern MACHINE_RESET( cbtime );
extern MACHINE_RESET( cgraplop );
extern MACHINE_RESET( cgraplop2 );
extern MACHINE_RESET( clapapa );
extern MACHINE_RESET( cfghtice );
extern MACHINE_RESET( cprobowl );
extern MACHINE_RESET( cnightst );
extern MACHINE_RESET( cprosocc );
extern MACHINE_RESET( cppicf );
extern MACHINE_RESET( cscrtry );
extern MACHINE_RESET( cflyball );
extern MACHINE_RESET( cbdash );
extern MACHINE_RESET( czeroize );

extern WRITE8_HANDLER( i8041_p1_w );
extern READ8_HANDLER( i8041_p1_r );
extern WRITE8_HANDLER( i8041_p2_w );
extern READ8_HANDLER( i8041_p2_r );

void decocass_machine_state_save_init(running_machine *machine);

/*----------- defined in video/decocass.c -----------*/
extern WRITE8_HANDLER( decocass_paletteram_w );
extern WRITE8_HANDLER( decocass_charram_w );
extern WRITE8_HANDLER( decocass_fgvideoram_w );
extern WRITE8_HANDLER( decocass_colorram_w );
extern WRITE8_HANDLER( decocass_bgvideoram_w );
extern WRITE8_HANDLER( decocass_tileram_w );
extern WRITE8_HANDLER( decocass_objectram_w );

extern WRITE8_HANDLER( decocass_watchdog_count_w );
extern WRITE8_HANDLER( decocass_watchdog_flip_w );
extern WRITE8_HANDLER( decocass_color_missiles_w );
extern WRITE8_HANDLER( decocass_mode_set_w );
extern WRITE8_HANDLER( decocass_color_center_bot_w );
extern WRITE8_HANDLER( decocass_back_h_shift_w );
extern WRITE8_HANDLER( decocass_back_vl_shift_w );
extern WRITE8_HANDLER( decocass_back_vr_shift_w );
extern WRITE8_HANDLER( decocass_part_h_shift_w );
extern WRITE8_HANDLER( decocass_part_v_shift_w );
extern WRITE8_HANDLER( decocass_center_h_shift_space_w );
extern WRITE8_HANDLER( decocass_center_v_shift_w );

extern VIDEO_START( decocass );
extern VIDEO_UPDATE( decocass );

void decocass_video_state_save_init(running_machine *machine);
