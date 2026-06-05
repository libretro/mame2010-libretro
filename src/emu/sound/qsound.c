/***************************************************************************

  Capcom System QSound(tm)
  ========================

  Driver by Paul Leaman (paul@vortexcomputing.demon.co.uk)
        and Miguel Angel Horna (mahorna@teleline.es)

  A 16 channel stereo sample player.

  QSpace position is simulated by panning the sound in the stereo space.

  Register
  0  xxbb   xx = unknown bb = start high address
  1  ssss   ssss = sample start address
  2  pitch
  3  unknown (always 0x8000)
  4  loop offset from end address
  5  end
  6  master channel volume
  7  not used
  8  Balance (left=0x0110  centre=0x0120 right=0x0130)
  9  unknown (most fixed samples use 0 for this register)

  Many thanks to CAB (the author of Amuse), without whom this probably would
  never have been finished.

  If anybody has some information about this hardware, please send it to me
  to mahorna@teleline.es or 432937@cepsz.unizar.es.
  http://teleline.terra.es/personal/mahorna

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "qsound.h"

/*
Debug defines
*/
#define LOG_WAVE	0
#define VERBOSE  0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/* 8 bit source ROM samples */
typedef int8_t QSOUND_SRC_SAMPLE;


#define QSOUND_CLOCKDIV 166			 /* Clock divider */
#define QSOUND_CHANNELS 16

/* Echo delay-line geometry.  The DSP derives the ring length from
 * register 0xd9 as (end_pos - QSOUND_DELAY_BASE_OFFSET); we cap it at
 * the local ring size. */
#define QSOUND_DELAY_BASE_OFFSET 0x554
#define QSOUND_DELAY_BUFFER_LEN  1024

/* Output-stage (spatial-audio) post-processing geometry. */
#define QSOUND_FIR_TAPS   95
#define QSOUND_DELAY_TAPS 51

/* Firmware-derived pan-mix curves and FIR coefficient sets for the
 * QSound output stage.  These constant arrays are reproduced from the
 * qsound-hle project
 *   https://github.com/ValleyBell/qsound-hle
 *   Copyright (c) 2018, ValleyBell, Ian Karlsson  (BSD 3-Clause)
 * which derived them by disassembling and analysing the Capcom DL-1425
 * DSP program ROM.  They are the firmware's own pan-curve and filter
 * coefficient values, included here as data (not code) under the
 * BSD-3-Clause license.
 *
 * The dry table is the per-voice coefficient curve for the unfiltered
 * audio path: indexed by pan position 0..32, table[i] gives the L
 * coefficient and table[32-i] gives the R coefficient (so symmetric pan
 * positions produce mirrored amplitudes).  The wet table is the same
 * shape for the FIR-filtered path; its very different curve is what
 * gives the chip its characteristic spatial colour. */
static const int16_t qsound_dry_mix_table[33] = {
	-16384,-16384,-16384,-16384,-16384,-16384,-16384,-16384,
	-16384,-16384,-16384,-16384,-16384,-16384,-16384,-16384,
	-16384,-14746,-13107,-11633,-10486, -9175, -8520, -7209,
	 -6226, -5226, -4588, -3768, -3277, -2703, -2130, -1802,
	     0
};

static const int16_t qsound_wet_mix_table[33] = {
	     0, -1638, -1966, -2458, -2949, -3441, -4096, -4669,
	 -4915, -5120, -5489, -6144, -7537, -8831, -9339, -9830,
	-10240,-10322,-10486,-10568,-10650,-11796,-12288,-12288,
	-12534,-12648,-12780,-12829,-12943,-13107,-13418,-14090,
	-16384
};

/* Default left-channel FIR taps (firmware ROM 0xdb2). */
static const int16_t qsound_fir_taps_l[QSOUND_FIR_TAPS] = {
	    0,   0,   0,  85,  24, -76,-123, -86, -29, -14, -20,  -7,   6, -28, -87, -89,  -5, 100, 154, 160,
	  150, 118,  41, -48, -78, -23,  59,  83,  -2,-176,-333,-344,-203, -66, -39,   2, 224, 495, 495, 280,
	  432,1340,2483,5377,1905, 658,   0,  97, 347, 285,  35, -95, -78, -82,-151,-192,-171,-149,-147,-113,
	  -22,  71, 118, 129, 127, 110,  71,  31,  20,  36,  46,  23, -27, -63, -53, -21, -19, -60, -92, -69,
	  -12,  25,  29,  30,  40,  41,  29,  30,  46,  39, -15, -74,   0,   0,   0
};

/* Default right-channel FIR taps (firmware ROM 0xe11). */
static const int16_t qsound_fir_taps_r[QSOUND_FIR_TAPS] = {
	    0,   0,   0,  23,  42,  47,  29,  10,   2, -14, -54, -92, -93, -70, -64, -77, -57,  18,  94, 113,
	   87,  69,  67,  50,  25,  29,  58,  62,  24, -39,-131,-256,-325,-234, -45,  58,  78, 223, 485, 496,
	  127,   6, 857,2283,2683,4928,1328, 132,  79, 314, 189, -80, -90,  35, -21,-186,-195, -99,-136,-258,
	 -189,  82, 257, 185,  53,  41,  84,  68,  38,  63,  77,  14, -60, -71, -71,-120,-151, -84,  14,  29,
	   -8,   7,  66,  69,  12,  -3,  54,  92,  52,  -6, -15,  -2,   0,   0,   0
};

struct qsound_fir_state
{
	int16_t        delay_line[QSOUND_FIR_TAPS];
	int            pos;        /* write position; reads walk backward */
	int            silent_run; /* consecutive zero inputs; result is provably zero at TAPS */
	const int16_t *taps;       /* coefficient set (qsound_fir_taps_l/r) */
};

struct qsound_delay_state
{
	int16_t delay_line[QSOUND_DELAY_TAPS];
	int     write_pos;
	int     read_pos;
	int     delay;      /* desired read-pos lag behind write_pos */
	int     volume;     /* attenuation applied to each output sample */
};

typedef stream_sample_t QSOUND_SAMPLE;

struct QSOUND_CHANNEL
{
	int32_t bank;	   /* bank (x16)    */
	int32_t address;	/* start address */
	int32_t pitch;	  /* pitch */
	int32_t reg3;	   /* unknown (always 0x8000) */
	int32_t loop;	   /* loop address */
	int32_t end;		/* end address */
	int32_t vol;		/* master volume */
	int32_t pan;		/* Pan value */
	int32_t echo;	   /* echo send level (register 0xba+ch); 0 = no echo */

	/* Work variables */
	int32_t key;		/* Key on / key off */

	int32_t lvol;	   /* left volume */
	int32_t rvol;	   /* right volume */
	int32_t pan_index; /* cached pan-table index 0..32 (spatial path) */
	int32_t lastdt;	 /* last sample value */
	int32_t offset;	 /* current offset counter */
};

typedef struct _qsound_state qsound_state;
struct _qsound_state
{
	/* Private variables */
	sound_stream * stream;				/* Audio stream */
	struct QSOUND_CHANNEL channel[QSOUND_CHANNELS];
	int data;				  /* register latch data */
	QSOUND_SRC_SAMPLE *sample_rom;	/* Q sound sample ROM */
	uint32_t sample_rom_length;

	int pan_table[33];		 /* Pan volume table */
	float frq_ratio;		   /* Frequency ratio */

	/* QSound echo effect state.  The DL-1425 DSP applies a feedback echo
	 * to the summed voice contributions on the way to the output mixer.
	 * Each voice has a per-voice echo send level (register 0xba+ch, the
	 * channel[].echo field); two global registers drive the delay line:
	 * 0x93 sets the feedback gain and 0xd9 sets the delay-line end
	 * position (length = end_pos - 0x554, capped at the local ring).
	 * The ring stores the HIGH 16 bits of the 32-bit feedback sum, which
	 * bounds the feedback path so it cannot run away into noise the
	 * moment a voice writes a non-zero echo-send register.  Per output
	 * sample the DSP averages the two-most-recent ring entries, scales
	 * by the feedback gain, adds the current accumulated voice input,
	 * writes the high half back to the ring, and mixes the averaged
	 * value into the output.  Without the FIR filter (the real DSP
	 * filters only the wet path) we mix the averaged echo into both L
	 * and R equally: this loses the chip's subtle L-dry/R-wet asymmetry
	 * but keeps the stereo image balanced. */
	int16_t echo_buffer[QSOUND_DELAY_BUFFER_LEN];
	int echo_pos;		/* current ring read/write position */
	int echo_length;	 /* current ring length (samples) */
	int echo_end_pos;	/* raw value written to register 0xd9 */
	int echo_feedback;   /* feedback gain (16-bit signed) */
	int echo_last;	   /* previous ring entry (for the 2-tap average) */

	/* Output-stage post-processing (spatial-audio path).  When
	 * output_filter_enabled is zero this entire stage is bypassed and
	 * the legacy sqrt-equal-power pan is used.  When non-zero, voices
	 * are routed into separate dry and wet accumulators via the
	 * firmware pan curves, the wet path runs through the 95-tap FIR,
	 * echo is routed asymmetrically (L dry / R wet -- the algorithm's
	 * "outside-the-speakers" trick), both paths run through their own
	 * delay rings (wet 0, dry 46 left / 48 right), and the two are
	 * summed at the output with DSP-round rounding to the high word. */
	struct qsound_fir_state   fir[2];        /* [0]=L, [1]=R */
	struct qsound_delay_state delay_wet[2];  /* [0]=L, [1]=R */
	struct qsound_delay_state delay_dry[2];  /* [0]=L, [1]=R */
	int output_filter_enabled;

	FILE *fpRawDataL;
	FILE *fpRawDataR;
};

INLINE qsound_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_QSOUND);
	return (qsound_state *)downcast<legacy_device_base *>(device)->token();
}

/* Set by the libretro layer from the core option mame2010-qsound_hle
 * (default disabled).  Read once per device at DEVICE_START into the
 * per-instance chip->output_filter_enabled flag.  When zero the spatial
 * path is bypassed entirely and the legacy stereo mix is used. */
int qsound_output_filter_enabled = 0;


/* Function prototypes */
static STREAM_UPDATE( qsound_update );
static void qsound_set_command(qsound_state *chip, int data, int value);
static void qsound_delay_recompute(struct qsound_delay_state *d);

static DEVICE_START( qsound )
{
	qsound_state *chip = get_safe_token(device);
	int i;

	chip->sample_rom = (QSOUND_SRC_SAMPLE *)*device->region();
	chip->sample_rom_length = device->region()->bytes();

	memset(chip->channel, 0, sizeof(chip->channel));

	/* pan_index defaults to centre (16) for every voice; used only by
	 * the spatial path when the core option is enabled. */
	for (i=0; i<QSOUND_CHANNELS; i++)
		chip->channel[i].pan_index = 16;

	chip->frq_ratio = 16.0;

	/* Create pan table */
	for (i=0; i<33; i++)
	{
		chip->pan_table[i]=(int)((256/sqrt(32.0)) * sqrt((double)i));
	}

	LOG(("Pan table\n"));
	for (i=0; i<33; i++)
		LOG(("%02x ", chip->pan_table[i]));

	/* Echo delay line.  Initial state matches the algorithm's power-on
	 * values: the ring is empty, the end-position register defaults to
	 * BASE_OFFSET + 6 (so the initial length is 6 samples until the game
	 * programs a real value via register 0xd9), feedback is zero, and
	 * the moving-average carry slot starts at zero.  Per-voice .echo
	 * defaults to 0 from the channel memset above. */
	memset(chip->echo_buffer, 0, sizeof(chip->echo_buffer));
	chip->echo_pos      = 0;
	chip->echo_end_pos  = QSOUND_DELAY_BASE_OFFSET + 6;
	chip->echo_length   = 6;
	chip->echo_feedback = 0;
	chip->echo_last     = 0;

	/* Snapshot the spatial-path enable from the libretro core option. */
	chip->output_filter_enabled = qsound_output_filter_enabled;

	/* Output-stage FIR + delay state.  Each FIR channel binds to its
	 * firmware tap set (left = ROM 0xdb2, right = ROM 0xe11); the FIR
	 * delay lines start cleared and in the silent-skip fast path.  The
	 * wet delays start at zero offset, the dry delays at the algorithm's
	 * mode-1 defaults (46 left / 48 right).  Output volumes default to
	 * 0x3fff (~unity) so the path is audible without the game having to
	 * program them. */
	memset(&chip->fir[0], 0, sizeof(chip->fir[0]));
	memset(&chip->fir[1], 0, sizeof(chip->fir[1]));
	chip->fir[0].taps       = qsound_fir_taps_l;
	chip->fir[1].taps       = qsound_fir_taps_r;
	chip->fir[0].silent_run = QSOUND_FIR_TAPS;
	chip->fir[1].silent_run = QSOUND_FIR_TAPS;
	memset(&chip->delay_wet[0], 0, sizeof(chip->delay_wet[0]));
	memset(&chip->delay_wet[1], 0, sizeof(chip->delay_wet[1]));
	memset(&chip->delay_dry[0], 0, sizeof(chip->delay_dry[0]));
	memset(&chip->delay_dry[1], 0, sizeof(chip->delay_dry[1]));
	chip->delay_wet[0].delay  = 0;
	chip->delay_wet[1].delay  = 0;
	chip->delay_dry[0].delay  = 46;
	chip->delay_dry[1].delay  = 48;
	chip->delay_wet[0].volume = 0x3fff;
	chip->delay_wet[1].volume = 0x3fff;
	chip->delay_dry[0].volume = 0x3fff;
	chip->delay_dry[1].volume = 0x3fff;
	qsound_delay_recompute(&chip->delay_wet[0]);
	qsound_delay_recompute(&chip->delay_wet[1]);
	qsound_delay_recompute(&chip->delay_dry[0]);
	qsound_delay_recompute(&chip->delay_dry[1]);

	{
		/* Allocate stream */
		chip->stream = stream_create(
			device, 0, 2,
			device->clock() / QSOUND_CLOCKDIV,
			chip,
			qsound_update );
	}

	if (LOG_WAVE)
	{
		chip->fpRawDataR=fopen("qsoundr.raw", "w+b");
		chip->fpRawDataL=fopen("qsoundl.raw", "w+b");
	}

	/* state save */
	for (i=0; i<QSOUND_CHANNELS; i++)
	{
		state_save_register_device_item(device, i, chip->channel[i].bank);
		state_save_register_device_item(device, i, chip->channel[i].address);
		state_save_register_device_item(device, i, chip->channel[i].pitch);
		state_save_register_device_item(device, i, chip->channel[i].loop);
		state_save_register_device_item(device, i, chip->channel[i].end);
		state_save_register_device_item(device, i, chip->channel[i].vol);
		state_save_register_device_item(device, i, chip->channel[i].pan);
		state_save_register_device_item(device, i, chip->channel[i].key);
		state_save_register_device_item(device, i, chip->channel[i].lvol);
		state_save_register_device_item(device, i, chip->channel[i].rvol);
		state_save_register_device_item(device, i, chip->channel[i].lastdt);
		state_save_register_device_item(device, i, chip->channel[i].offset);
		state_save_register_device_item(device, i, chip->channel[i].echo);
		state_save_register_device_item(device, i, chip->channel[i].pan_index);
	}

	/* Echo delay-line global state */
	state_save_register_device_item(device, 0, chip->echo_pos);
	state_save_register_device_item(device, 0, chip->echo_length);
	state_save_register_device_item(device, 0, chip->echo_end_pos);
	state_save_register_device_item(device, 0, chip->echo_feedback);
	state_save_register_device_item(device, 0, chip->echo_last);
	state_save_register_device_item_array(device, 0, chip->echo_buffer);
}

static DEVICE_STOP( qsound )
{
	qsound_state *chip = get_safe_token(device);
	if (chip->fpRawDataR)
	{
		fclose(chip->fpRawDataR);
	}
	chip->fpRawDataR = NULL;
	if (chip->fpRawDataL)
	{
		fclose(chip->fpRawDataL);
	}
	chip->fpRawDataL = NULL;
}

WRITE8_DEVICE_HANDLER( qsound_w )
{
	qsound_state *chip = get_safe_token(device);
	switch (offset)
	{
		case 0:
			chip->data=(chip->data&0xff)|(data<<8);
			break;

		case 1:
			chip->data=(chip->data&0xff00)|data;
			break;

		case 2:
			qsound_set_command(chip, data, chip->data);
			break;

		default:
			logerror("%s: unexpected qsound write to offset %d == %02X\n", cpuexec_describe_context(device->machine), offset, data);
			break;
	}
}

READ8_DEVICE_HANDLER( qsound_r )
{
	/* Port ready bit (0x80 if ready) */
	return 0x80;
}

static void qsound_set_command(qsound_state *chip, int data, int value)
{
	int ch=0,reg=0;
	if (data < 0x80)
	{
		ch=data>>3;
		reg=data & 0x07;
	}
	else
	{
		if (data < 0x90)
		{
			ch=data-0x80;
			reg=8;
		}
		else if (data == 0x93)
		{
			/* Global echo feedback gain */
			reg = 10;
		}
		else if (data == 0xd9)
		{
			/* Global echo delay-line end position; length is derived
			 * as (end_pos - QSOUND_DELAY_BASE_OFFSET) and clamped into
			 * the local ring buffer below. */
			reg = 11;
		}
		else
		{
			if (data >= 0xba && data < 0xca)
			{
				ch=data-0xba;
				reg=9;
			}
			else
			{
				/* Unknown registers */
				ch=99;
				reg=99;
			}
		}
	}

	switch (reg)
	{
		case 0: /* Bank */
			ch=(ch+1)&0x0f;	/* strange ... */
			chip->channel[ch].bank=(value&0x7f)<<16;
#ifdef MAME_DEBUG
			if (!(value & 0x8000))
				popmessage("Register3=%04x",value);
#endif

			break;
		case 1: /* start */
			chip->channel[ch].address=value;
			break;
		case 2: /* pitch */
			chip->channel[ch].pitch=value * 16;
			if (!value)
			{
				/* Key off */
				chip->channel[ch].key=0;
			}
			break;
		case 3: /* unknown */
			chip->channel[ch].reg3=value;
#ifdef MAME_DEBUG
			if (value != 0x8000)
				popmessage("Register3=%04x",value);
#endif
			break;
		case 4: /* loop offset */
			chip->channel[ch].loop=value;
			break;
		case 5: /* end */
			chip->channel[ch].end=value;
			break;
		case 6: /* master volume */
			if (value==0)
			{
				/* Key off */
				chip->channel[ch].key=0;
			}
			else if (chip->channel[ch].key==0)
			{
				/* Key on */
				chip->channel[ch].key=1;
				chip->channel[ch].offset=0;
				chip->channel[ch].lastdt=0;
			}
			chip->channel[ch].vol=value;
			break;

		case 7:  /* unused */
#ifdef MAME_DEBUG
				popmessage("UNUSED QSOUND REG 7=%04x",value);
#endif

			break;
		case 8:
			{
			   int pandata=(value-0x10)&0x3f;
			   if (pandata > 32)
			   {
					pandata=32;
			   }
			   chip->channel[ch].rvol=chip->pan_table[pandata];
			   chip->channel[ch].lvol=chip->pan_table[32-pandata];
			   chip->channel[ch].pan_index = pandata;
			   chip->channel[ch].pan = value;
			}
			break;
		 case 9: /* per-voice echo send (register 0xba+ch) */
			chip->channel[ch].echo=value;
			break;
		 case 10: /* global echo feedback (register 0x93) */
			chip->echo_feedback = (int16_t)value;
			break;
		 case 11: /* global echo end-position (register 0xd9) */
			chip->echo_end_pos = value;
			{
				int len = value - QSOUND_DELAY_BASE_OFFSET;
				if (len < 0) len = 0;
				if (len > QSOUND_DELAY_BUFFER_LEN)
					len = QSOUND_DELAY_BUFFER_LEN;
				chip->echo_length = len;
				/* Reset the ring position so the new delay window starts
				 * coherently rather than mid-tap. */
				if (chip->echo_pos >= len)
					chip->echo_pos = 0;
			}
			break;
	}
	LOG(("QSOUND WRITE %02x CH%02d-R%02d =%04x\n", data, ch, reg, value));
}


/* Convolve the input with the channel's fixed firmware tap set, walking
 * the delay line backward from the most recently written sample.
 * Matches the algorithm's accumulation: per tap, subtract
 * (tap * delayed_sample) << 2 from the accumulator (the firmware taps
 * were designed for this negative-accumulate convention and for the pan
 * tables' negative-coefficient output, so the two negations cancel and
 * the audible signal is positive).  Input arrives pre-scaled (>>16 of
 * the dry/wet accumulator at the call site, back into int16 range). */
static int32_t qsound_fir_apply(struct qsound_fir_state *s, int16_t input)
{
	int32_t acc = 0;
	int     pos;
	int     i;

	/* Fast path: once the input has been zero long enough for the full
	 * tap-count to roll through the delay line, every tap multiplies by
	 * zero and the result is provably zero.  Skip the 95-iter loop; the
	 * ring slot still advances to keep position consistent. */
	if (input == 0 && s->silent_run >= QSOUND_FIR_TAPS)
	{
		s->delay_line[s->pos] = 0;
		s->pos++;
		if (s->pos >= QSOUND_FIR_TAPS) s->pos = 0;
		return 0;
	}
	if (input == 0) s->silent_run++;
	else            s->silent_run = 0;

	s->delay_line[s->pos] = input;
	pos = s->pos;
	for (i = 0; i < QSOUND_FIR_TAPS; i++)
	{
		acc -= ((int32_t)s->taps[i] * (int32_t)s->delay_line[pos]) << 2;
		if (pos == 0) pos = QSOUND_FIR_TAPS - 1;
		else          pos--;
	}

	s->pos++;
	if (s->pos >= QSOUND_FIR_TAPS) s->pos = 0;

	return acc;
}

/* Apply a fixed-offset delay with output volume gain: write the new
 * input's high-16 to the int16 ring, read the lagging value, scale by
 * the channel's volume.  write_pos and read_pos advance in lockstep so
 * the lag is preserved across calls. */
static int32_t qsound_delay_apply(struct qsound_delay_state *d, int32_t input)
{
	int32_t output;

	d->delay_line[d->write_pos] = (int16_t)(input >> 16);
	d->write_pos++;
	if (d->write_pos >= QSOUND_DELAY_TAPS) d->write_pos = 0;

	output = (int32_t)d->delay_line[d->read_pos] * d->volume;
	d->read_pos++;
	if (d->read_pos >= QSOUND_DELAY_TAPS) d->read_pos = 0;

	return output;
}

/* Recompute the delay read position from the current write position and
 * the desired lag.  Called at init and whenever the host programs a new
 * delay value. */
static void qsound_delay_recompute(struct qsound_delay_state *d)
{
	int new_read = d->write_pos - d->delay;
	while (new_read < 0) new_read += QSOUND_DELAY_TAPS;
	while (new_read >= QSOUND_DELAY_TAPS) new_read -= QSOUND_DELAY_TAPS;
	d->read_pos = new_read;
}

/* Apply the echo to a single accumulated voice-input sample, return the
 * echoed output to be mixed into the L/R outputs.  The ring stores the
 * HIGH 16 bits of the 32-bit feedback sum -- this is what bounds the
 * feedback path and prevents the ring from saturating into noise the
 * moment any voice writes a non-zero echo-send register.  Returns the
 * 2-tap-averaged delay-line value, already in int16 range. */
static int16_t qsound_echo_apply(qsound_state *chip, int32_t input)
{
	int32_t old_sample = chip->echo_buffer[chip->echo_pos]; /* sign-extended int16 -> int32 */
	int32_t last       = chip->echo_last;
	int32_t new_sample;

	chip->echo_last = (int16_t)old_sample;
	/* 2-tap moving average over the delay-line output */
	old_sample = (old_sample + last) >> 1;

	/* Feedback path: add the feedback-attenuated average back to the
	 * current accumulated voice input.  old_sample is int16-bounded
	 * (read from the int16 ring), feedback is 16-bit, so the product is
	 * bounded ~1G and fits int32. */
	new_sample = input + ((old_sample * chip->echo_feedback) << 2);
	/* Truncate-store the HIGH 16 bits into the ring.  input here is the
	 * sum across up to 16 voices of (vol-scaled-sample * echo-send) and
	 * can reach hundreds of millions; the >>16 dispatches that into the
	 * int16 ring slot without taking the feedback path into overflow on
	 * the next pass. */
	chip->echo_buffer[chip->echo_pos] = (int16_t)(new_sample >> 16);

	chip->echo_pos++;
	if (chip->echo_pos >= chip->echo_length)
		chip->echo_pos = 0;

	return (int16_t)old_sample;
}


static STREAM_UPDATE( qsound_update )
{
	/* Per-sample outer loop: precompute per-voice setup once, then for
	 * each output sample step through every active voice, accumulating
	 * its contribution.  This structure is the natural shape for any
	 * "post-process every voice's contribution" effect (echo, FIR); the
	 * old per-voice-outer structure made those harder to add. */
	qsound_state *chip = (qsound_state *)param;
	int i,j;
	QSOUND_SAMPLE *pOutL;
	QSOUND_SAMPLE *pOutR;
	/* Per-voice volume scaling cached for the whole buffer (legacy path). */
	int32_t lvol[QSOUND_CHANNELS];
	int32_t rvol[QSOUND_CHANNELS];

	pOutL = outputs[0];
	pOutR = outputs[1];
	memset(pOutL, 0x00, samples * sizeof(*pOutL));
	memset(pOutR, 0x00, samples * sizeof(*pOutR));

	for (i=0; i<QSOUND_CHANNELS; i++)
	{
		if (chip->channel[i].key)
		{
			lvol[i] = (chip->channel[i].lvol * chip->channel[i].vol) >> 8;
			rvol[i] = (chip->channel[i].rvol * chip->channel[i].vol) >> 8;
		}
	}

	for (j=0; j<samples; j++)
	{
		struct QSOUND_CHANNEL *pC = &chip->channel[0];
		int32_t lacc    = 0;
		int32_t racc    = 0;
		int32_t l_dry   = 0;
		int32_t r_dry   = 0;
		int32_t l_wet   = 0;
		int32_t r_wet   = 0;
		int32_t echo_in = 0;
		int16_t echo_out;

		for (i=0; i<QSOUND_CHANNELS; i++, pC++)
		{
			int count, v;
			if (!pC->key)
				continue;
			count = (pC->offset) >> 16;
			pC->offset &= 0xffff;
			if (count)
			{
				pC->address += count;
				if (pC->address >= pC->end)
				{
					if (!pC->loop)
					{
						/* Reached the end of a non-looped sample */
						pC->key = 0;
						continue;
					}
					/* Reached the end, restart the loop */
					pC->address = (pC->end - pC->loop) & 0xffff;
				}
				pC->lastdt = chip->sample_rom[(pC->bank+pC->address)%(chip->sample_rom_length)];
			}
			v = pC->lastdt;

			if (chip->output_filter_enabled)
			{
				/* Spatial path: approximate the algorithm's voice_output
				 * (8-bit sample expanded to int16 then scaled by the
				 * voice volume) as (v * vol) >> 6, then accumulate into
				 * separate dry and wet accumulators via the firmware pan
				 * tables.  Those tables hold negative coefficients for
				 * the DSP's subtract-accumulate convention, so we -=. */
				int32_t vo = (v * pC->vol) >> 6;
				int     p  = pC->pan_index;
				l_dry -= (vo * qsound_dry_mix_table[p])      << 2;
				r_dry -= (vo * qsound_dry_mix_table[32 - p]) << 2;
				l_wet -= (vo * qsound_wet_mix_table[p])      << 2;
				r_wet -= (vo * qsound_wet_mix_table[32 - p]) << 2;
				if (pC->echo)
					echo_in += vo * pC->echo;
			}
			else
			{
				/* Legacy path: simple stereo mix using the sqrt-equal-
				 * power pan lookup, bit-identical to the pre-echo code. */
				lacc += (v * lvol[i]) >> 6;
				racc += (v * rvol[i]) >> 6;
				if (pC->echo)
					echo_in += ((v * pC->vol) >> 8) * pC->echo;
			}
			pC->offset += pC->pitch;
		}

		/* Run the echo state machine for this output sample.  Skipped
		 * when the delay line is disabled (length 0). */
		if (chip->echo_length > 0)
			echo_out = qsound_echo_apply(chip, echo_in);
		else
			echo_out = 0;

		if (chip->output_filter_enabled)
		{
			/* Echo routing: the algorithm mixes the echo into the L
			 * channel's dry path and the R channel's wet path (the
			 * asymmetric "outside-the-speakers" trick).  After echo, the
			 * wet accumulator runs through the channel FIR, both paths
			 * run through their delay rings (wet 0, dry 46/48), and the
			 * two are summed with DSP-round to the high word. */
			int32_t l_wet_f, r_wet_f, l_out, r_out;

			l_dry += (int32_t)echo_out << 16;
			r_wet += (int32_t)echo_out << 16;

			l_wet_f = qsound_fir_apply(&chip->fir[0], (int16_t)(l_wet >> 16));
			r_wet_f = qsound_fir_apply(&chip->fir[1], (int16_t)(r_wet >> 16));

			l_out = (qsound_delay_apply(&chip->delay_wet[0], l_wet_f) +
			         qsound_delay_apply(&chip->delay_dry[0], l_dry)) << 2;
			r_out = (qsound_delay_apply(&chip->delay_wet[1], r_wet_f) +
			         qsound_delay_apply(&chip->delay_dry[1], r_dry)) << 2;

			/* DSP-round: round to nearest 0x10000 then take high word. */
			l_out = (l_out + 0x8000) & ~0xffff;
			r_out = (r_out + 0x8000) & ~0xffff;
			pOutL[j] = (QSOUND_SAMPLE)(l_out >> 16);
			pOutR[j] = (QSOUND_SAMPLE)(r_out >> 16);
		}
		else
		{
			lacc += echo_out;
			racc += echo_out;
			pOutL[j] = lacc;
			pOutR[j] = racc;
		}
	}

	if (chip->fpRawDataL)
		fwrite(outputs[0], samples*sizeof(QSOUND_SAMPLE), 1, chip->fpRawDataL);
	if (chip->fpRawDataR)
		fwrite(outputs[1], samples*sizeof(QSOUND_SAMPLE), 1, chip->fpRawDataR);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( qsound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(qsound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( qsound );			break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( qsound );			break;
		case DEVINFO_FCT_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Q-Sound");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Capcom custom");				break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

/**************** end of file ****************/

DEFINE_LEGACY_SOUND_DEVICE(QSOUND, qsound);
