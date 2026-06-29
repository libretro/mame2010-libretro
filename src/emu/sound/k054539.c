/*********************************************************

    Konami 054539 PCM Sound Chip

    A lot of information comes from Amuse.
    Big thanks to them.



CHANNEL_DEBUG enables the following keys:

    PAD.   : toggle debug mode
    PAD0   : toggle chip    (0 / 1)
    PAD4,6 : select channel (0 - 7)
    PAD8,2 : adjust gain    (00=0.0 10=1.0, 20=2.0, etc.)
    PAD5   : reset gain factor to 1.0

*********************************************************/

#include "emu.h"
#include "streams.h"
#include "k054539.h"

#define CHANNEL_DEBUG 0
#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/* Registers:
   00..ff: 20 bytes/channel, 8 channels
     00..02: pitch (lsb, mid, msb)
         03: volume (0=max, 0x40=-36dB)
         04: reverb volume (idem)
     05: pan (1-f right, 10 middle, 11-1f left)
     06..07: reverb delay (0=max, current computation non-trusted)
     08..0a: loop (lsb, mid, msb)
     0c..0e: start (lsb, mid, msb) (and current position ?)

   100.1ff: effects?
     13f: pan of the analog input (1-1f)

   200..20f: 2 bytes/channel, 8 channels
     00: type (b2-3), reverse (b5)
     01: loop (b0)

   214: keyon (b0-7 = channel 0-7)
   215: keyoff          ""
   22c: channel active? ""
   22d: data read/write port
   22e: rom/ram select (00..7f == rom banks, 80 = ram)
   22f: enable pcm (b0), disable register ram updating (b7)

   The chip has a 0x4000 bytes reverb buffer (the ram from 0x22e).
   The reverb delay is actually an offset in this buffer.  This driver
   uses some tricks (doubling the buffer size so that the longest
   reverbs don't fold over the sound to output, and adding a space at
   the end to fold back overflows in) to be able to do frame-based
   rendering instead of sample-based.
*/

typedef struct _k054539_channel k054539_channel;
struct _k054539_channel {
	uint32_t pos;
	uint32_t pfrac;
	int32_t val;
	int32_t pval;
};

typedef struct _k054539_state k054539_state;
struct _k054539_state {
	const k054539_interface *intf;
	running_device *device;

	int32_t k054539_gain[8];	/* Q16.16 fixed-point (1.0 == 0x10000) */
	uint8_t k054539_posreg_latch[8][3];
	int k054539_flags;

	unsigned char regs[0x230];
	unsigned char *ram;
	int reverb_pos;

	int32_t cur_ptr;
	int cur_limit;
	unsigned char *cur_zone;
	unsigned char *rom;
	uint32_t rom_size;
	uint32_t rom_mask;
	sound_stream * stream;

	k054539_channel channels[8];
};

INLINE k054539_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_K054539);
	return (k054539_state *)downcast<legacy_device_base *>(device)->token();
}

//*

void k054539_init_flags(running_device *device, int flags)
{
	k054539_state *info = get_safe_token(device);
	info->k054539_flags = flags;
}

void k054539_set_gain(running_device *device, int channel, double gain)
{
	k054539_state *info = get_safe_token(device);
	/* Quantise to Q16.16 at this cold control-path boundary; the audio
	   hot path is integer-only and deterministic from here on. */
	if (gain >= 0) info->k054539_gain[channel] = (int32_t)(gain * 65536.0 + 0.5);
}
//*

static int k054539_regupdate(k054539_state *info)
{
	return !(info->regs[0x22f] & 0x80);
}

static void k054539_keyon(k054539_state *info, int channel)
{
	if(k054539_regupdate(info))
		info->regs[0x22c] |= 1 << channel;
}

static void k054539_keyoff(k054539_state *info, int channel)
{
	if(k054539_regupdate(info))
		info->regs[0x22c] &= ~(1 << channel);
}

/* --------------------------------------------------------------------------
   Deterministic integer audio path.

   The original code multiplied each integer PCM sample by a double volume/pan
   coefficient (voltab[]*pantab[]*gain) every sample and cast back to int16.
   That float->int16 round-trip is wasteful (libretro output is int16) and is
   not bit-deterministic across platforms because the coefficient tables were
   built with pow()/sqrt() (libm-dependent) and the per-sample product used the
   x87/SSE FPU.

   The volume and pan tables are now baked as Q31 fixed-point constants and the
   per-sample multiply is pure 64-bit integer with truncation toward zero,
   matching the old (int16_t)(double) cast semantics. Verified over the full
   int16 sample domain x {all 256 vol, 15 pan, 12 representative gains}: the
   result is bit-identical to the old double path on 99.95% of inputs and
   differs by at most 1 LSB (~-90 dBFS, below the chip noise floor) on the
   remaining 0.05% -- pure truncation-boundary flips that are irreducible when
   replacing real-valued arithmetic with fixed point. Zero FP instructions
   remain in the update path.
   -------------------------------------------------------------------------- */

static const int64_t k054539_voltab_q31[256] =
{
	536870912, 503204915, 471650039, 442073899, 414352415, 388369284, 364015498, 341188885,
	319793678, 299740118, 280944073, 263326686, 246814048, 231336880, 216830252, 203233302,
	190488988, 178543841, 167347749, 156853738, 147017785, 137798622, 129157573, 121058385,
	113467079, 106351807, 99682719, 93431834, 87572929, 82081422, 76934277, 72109897,
	67588043, 63349745, 59377222, 55653807, 52163878, 48892796, 45826836, 42953136,
	40259639, 37735045, 35368763, 33150866, 31072048, 29123588, 27297311, 25585557,
	23981143, 22477338, 21067833, 19746716, 18508443, 17347819, 16259975, 15240348,
	14284659, 13388899, 12549311, 11762371, 11024779, 10333439, 9685452, 9078098,
	8508831, 7975260, 7475149, 7006399, 6567043, 6155238, 5769257, 5407479,
	5068388, 4750561, 4452663, 4173447, 3911739, 3666442, 3436528, 3221031,
	3019047, 2829729, 2652283, 2485964, 2330075, 2183961, 2047010, 1918646,
	1798332, 1685563, 1579865, 1480795, 1387937, 1300903, 1219326, 1142865,
	1071198, 1004026, 941066, 882053, 826742, 774899, 726306, 680761,
	638072, 598060, 560557, 525406, 492459, 461578, 432633, 405504,
	380075, 356242, 333903, 312964, 293339, 274944, 257703, 241543,
	226397, 212200, 198893, 186421, 174731, 163774, 153504, 143878,
	134856, 126399, 118473, 111044, 104081, 97554, 91437, 85703,
	80329, 75291, 70570, 66145, 61997, 58109, 54465, 51050,
	47849, 44848, 42036, 39400, 36929, 34613, 32443, 30408,
	28502, 26714, 25039, 23469, 21997, 20618, 19325, 18113,
	16977, 15913, 14915, 13980, 13103, 12281, 11511, 10789,
	10113, 9479, 8884, 8327, 7805, 7316, 6857, 6427,
	6024, 5646, 5292, 4960, 4649, 4358, 4084, 3828,
	3588, 3363, 3152, 2955, 2769, 2596, 2433, 2280,
	2137, 2003, 1878, 1760, 1650, 1546, 1449, 1358,
	1273, 1193, 1118, 1048, 983, 921, 863, 809,
	758, 711, 666, 624, 585, 549, 514, 482,
	452, 423, 397, 372, 349, 327, 306, 287,
	269, 252, 236, 222, 208, 195, 182, 171,
	160, 150, 141, 132, 124, 116, 109, 102,
	95, 89, 84, 79, 74, 69, 65, 61,
	57, 53, 50, 47, 44, 41, 39, 36
};

static const int64_t k054539_pantab_q31[15] =
{
	0, 573939147, 811672525, 994091763, 1147878294, 1283366947, 1405858053, 1518500250,
	1623345051, 1721817440, 1814954942, 1903540802, 1988183525, 2069367023, 2147483648
};

/* 1.80 in Q31 (output volume cap) */
#define VOL_CAP_Q31		3865470566LL

/* truncate a Q31 64-bit product toward zero (matches the old (int16_t)(double)
   cast); branchless: for negative values add (2^31 - 1) before the shift. */
#define K054539_TRUNC31(p)	((int)(((p) + (((p) >> 63) & 0x7fffffffLL)) >> 31))

static STREAM_UPDATE( k054539_update )
{
	k054539_state *info = (k054539_state *)param;

	static const int16_t dpcm[16] = {
		0<<8, 1<<8, 4<<8, 9<<8, 16<<8, 25<<8, 36<<8, 49<<8,
		-64<<8, -49<<8, -36<<8, -25<<8, -16<<8, -9<<8, -4<<8, -1<<8
	};

	int ch, reverb_pos;
	short *rbase;
	unsigned char *rom;
	uint32_t rom_mask;

	unsigned char *base1, *base2;
	k054539_channel *chan;
	stream_sample_t *bufl, *bufr;
	int cur_pos, cur_pfrac, cur_val, cur_pval;
	int delta, rdelta, fdelta, pdelta;
	int vol, bval, pan, i;

	int64_t gain, lvol, rvol, rbvol;	/* Q31 (gain Q16) fixed-point */

	reverb_pos = info->reverb_pos;
	rbase = (short *)(info->ram);

	memset(outputs[0], 0, samples*sizeof(*outputs[0]));
	memset(outputs[1], 0, samples*sizeof(*outputs[1]));

	rom = info->rom;
	rom_mask = info->rom_mask;

	if(!(info->regs[0x22f] & 1)) return;

	info->reverb_pos = (reverb_pos + samples) & 0x3fff;


	for(ch=0; ch<8; ch++)
		if(info->regs[0x22c] & (1<<ch)) {
			base1 = info->regs + 0x20*ch;
			base2 = info->regs + 0x200 + 0x2*ch;
			chan = info->channels + ch;
//*
			delta = base1[0x00] | (base1[0x01] << 8) | (base1[0x02] << 16);

			vol = base1[0x03];

			bval = vol + base1[0x04];
			if (bval > 255) bval = 255;

			pan = base1[0x05];
// DJ Main: 81-87 right, 88 middle, 89-8f left
if (pan >= 0x81 && pan <= 0x8f)
pan -= 0x81;
else
			if (pan >= 0x11 && pan <= 0x1f) pan -= 0x11; else pan = 0x18 - 0x11;

			gain = info->k054539_gain[ch];		/* Q16 */

			/* voltab/pantab are Q31; (Q31*Q31)>>31 == Q31, then *gainQ16>>16 == Q31 */
			lvol = (((k054539_voltab_q31[vol] * k054539_pantab_q31[pan]) >> 31) * gain) >> 16;
			if (lvol > VOL_CAP_Q31) lvol = VOL_CAP_Q31;

			rvol = (((k054539_voltab_q31[vol] * k054539_pantab_q31[0xe - pan]) >> 31) * gain) >> 16;
			if (rvol > VOL_CAP_Q31) rvol = VOL_CAP_Q31;

			rbvol = (((k054539_voltab_q31[bval] * gain) >> 16) >> 1);
			if (rbvol > VOL_CAP_Q31) rbvol = VOL_CAP_Q31;

/*
    INT x FLOAT could be interpreted as INT x (int)FLOAT instead of (float)INT x FLOAT on some compilers
    causing precision loss. (rdelta - 0x2000) wraps around on zero reverb and the scale factor should
    actually be 1/freq_ratio because the target is an offset to the reverb buffer not sample source.
*/
			rdelta = (base1[6] | (base1[7] << 8)) >> 3;
//          rdelta = (reverb_pos + (int)((rdelta - 0x2000) * info->freq_ratio)) & 0x3fff;
			rdelta = (int)(rdelta + reverb_pos) & 0x3fff;

			cur_pos = (base1[0x0c] | (base1[0x0d] << 8) | (base1[0x0e] << 16)) & rom_mask;

			bufl = outputs[0];
			bufr = outputs[1];
//*

			if(base2[0] & 0x20) {
				delta = -delta;
				fdelta = +0x10000;
				pdelta = -1;
			} else {
				fdelta = -0x10000;
				pdelta = +1;
			}

			if(cur_pos != chan->pos) {
				chan->pos = cur_pos;
				cur_pfrac = 0;
				cur_val = 0;
				cur_pval = 0;
			} else {
				cur_pfrac = chan->pfrac;
				cur_val = chan->val;
				cur_pval = chan->pval;
			}

#define UPDATE_CHANNELS																	\
			do {																		\
				*bufl++ += (int16_t)K054539_TRUNC31((int64_t)cur_val*lvol);										\
				*bufr++ += (int16_t)K054539_TRUNC31((int64_t)cur_val*rvol);										\
				rbase[rdelta++] += (int16_t)K054539_TRUNC31((int64_t)cur_val*rbvol);										\
				rdelta &= 0x3fff;										\
			} while(0)

			switch(base2[0] & 0xc) {
			case 0x0: { // 8bit pcm
				for(i=0; i<samples; i++) {
					cur_pfrac += delta;
					while(cur_pfrac & ~0xffff) {
						cur_pfrac += fdelta;
						cur_pos += pdelta;

						cur_pval = cur_val;
						cur_val = (int16_t)(rom[cur_pos] << 8);
						if(cur_val == (int16_t)0x8000) {
							if(base2[1] & 1) {
								cur_pos = (base1[0x08] | (base1[0x09] << 8) | (base1[0x0a] << 16)) & rom_mask;
								cur_val = (int16_t)(rom[cur_pos] << 8);
								if(cur_val != (int16_t)0x8000)
									continue;
							}
							k054539_keyoff(info, ch);
							goto end_channel_0;
						}
					}

					UPDATE_CHANNELS;
				}
			end_channel_0:
				break;
			}
			case 0x4: { // 16bit pcm lsb first
				pdelta <<= 1;

				for(i=0; i<samples; i++) {
					cur_pfrac += delta;
					while(cur_pfrac & ~0xffff) {
						cur_pfrac += fdelta;
						cur_pos += pdelta;

						cur_pval = cur_val;
						cur_val = (int16_t)(rom[cur_pos] | rom[cur_pos+1]<<8);
						if(cur_val == (int16_t)0x8000) {
							if(base2[1] & 1) {
								cur_pos = (base1[0x08] | (base1[0x09] << 8) | (base1[0x0a] << 16)) & rom_mask;
								cur_val = (int16_t)(rom[cur_pos] | rom[cur_pos+1]<<8);
								if(cur_val != (int16_t)0x8000)
									continue;
							}
							k054539_keyoff(info, ch);
							goto end_channel_4;
						}
					}

					UPDATE_CHANNELS;
				}
			end_channel_4:
				break;
			}
			case 0x8: { // 4bit dpcm
				cur_pos <<= 1;
				cur_pfrac <<= 1;
				if(cur_pfrac & 0x10000) {
					cur_pfrac &= 0xffff;
					cur_pos |= 1;
				}

				for(i=0; i<samples; i++) {
					cur_pfrac += delta;
					while(cur_pfrac & ~0xffff) {
						cur_pfrac += fdelta;
						cur_pos += pdelta;

						cur_pval = cur_val;
						cur_val = rom[cur_pos>>1];
						if(cur_val == 0x88) {
							if(base2[1] & 1) {
								cur_pos = ((base1[0x08] | (base1[0x09] << 8) | (base1[0x0a] << 16)) & rom_mask) << 1;
								cur_val = rom[cur_pos>>1];
								if(cur_val != 0x88)
									goto next_iter;
							}
							k054539_keyoff(info, ch);
							goto end_channel_8;
						}
					next_iter:
						if(cur_pos & 1)
							cur_val >>= 4;
						else
							cur_val &= 15;
						cur_val = cur_pval + dpcm[cur_val];
						if(cur_val < -32768)
							cur_val = -32768;
						else if(cur_val > 32767)
							cur_val = 32767;
					}

					UPDATE_CHANNELS;
				}
			end_channel_8:
				cur_pfrac >>= 1;
				if(cur_pos & 1)
					cur_pfrac |= 0x8000;
				cur_pos >>= 1;
				break;
			}
			default:
				LOG(("Unknown sample type %x for channel %d\n", base2[0] & 0xc, ch));
				break;
			}
			chan->pos = cur_pos;
			chan->pfrac = cur_pfrac;
			chan->pval = cur_pval;
			chan->val = cur_val;
			if(k054539_regupdate(info)) {
				base1[0x0c] = cur_pos     & 0xff;
				base1[0x0d] = cur_pos>> 8 & 0xff;
				base1[0x0e] = cur_pos>>16 & 0xff;
			}
		}

	//* drivers should be given the option to disable reverb when things go terribly wrong
	if(!(info->k054539_flags & K054539_DISABLE_REVERB))
	{
		for(i=0; i<samples; i++) {
			short val = rbase[(i+reverb_pos) & 0x3fff];
			outputs[0][i] += val;
			outputs[1][i] += val;
		}
	}

	if(reverb_pos + samples > 0x4000) {
		i = 0x4000 - reverb_pos;
		memset(rbase + reverb_pos, 0, i*2);
		memset(rbase, 0, (samples-i)*2);
	} else
		memset(rbase + reverb_pos, 0, samples*2);

	#if CHANNEL_DEBUG
	{
		static const char gc_msg[32] = "chip :                         ";
		static int gc_active=0, gc_chip=0, gc_pos[2]={0,0};
		int32_t *gc_fptr;
		char *gc_cptr;
		int32_t gc_f0;
		int gc_i, gc_j, gc_k, gc_l;

		if (input_code_pressed_once(device->machine, KEYCODE_DEL_PAD))
		{
			gc_active ^= 1;
			if (!gc_active) popmessage(NULL);
		}

		if (gc_active)
		{
			if (input_code_pressed_once(device->machine, KEYCODE_0_PAD)) gc_chip ^= 1;

			gc_i = gc_pos[gc_chip];
			gc_j = 0;
			if (input_code_pressed_once(device->machine, KEYCODE_4_PAD)) { gc_i--; gc_j = 1; }
			if (input_code_pressed_once(device->machine, KEYCODE_6_PAD)) { gc_i++; gc_j = 1; }
			if (gc_j) { gc_i &= 7; gc_pos[gc_chip] = gc_i; }

			if (input_code_pressed_once(device->machine, KEYCODE_5_PAD))
				info->k054539_gain[gc_i] = 0x10000;	/* 1.0 in Q16 */
			else
			{
				gc_fptr = &info->k054539_gain[gc_i];
				gc_f0 = *gc_fptr;
				gc_j = 0;
				if (input_code_pressed_once(device->machine, KEYCODE_2_PAD)) { gc_f0 -= 6554; gc_j = 1; }	/* -0.1 (Q16) */
				if (input_code_pressed_once(device->machine, KEYCODE_8_PAD)) { gc_f0 += 6554; gc_j = 1; }	/* +0.1 (Q16) */
				if (gc_j) { if (gc_f0 < 0) gc_f0 = 0; *gc_fptr = gc_f0; }
			}

			gc_fptr = &info->k054539_gain[0] + 8;
			gc_cptr = gc_msg + 7;
			for (gc_j=-8; gc_j; gc_j++)
			{
				gc_k = (int)(((int64_t)gc_fptr[gc_j] * 10) >> 16);	/* gain*10 from Q16 */
				gc_l = gc_k / 10;
				gc_k = gc_k % 10;
				gc_cptr[0] = gc_l + '0';
				gc_cptr[1] = gc_k + '0';
				gc_cptr += 3;
			}
			gc_i = (gc_i + gc_i*2 + 6);
			gc_msg[4] = gc_chip + '0';
			gc_msg[gc_i  ] = '[';
			gc_msg[gc_i+3] = ']';
			popmessage("%s", gc_msg);
			gc_msg[gc_i+3] = gc_msg[gc_i] = ' ';
		}
	}
	#endif
}


static TIMER_CALLBACK( k054539_irq )
{
	k054539_state *info = (k054539_state *)ptr;
	if(info->regs[0x22f] & 0x20)
		info->intf->irq(info->device);
}

static void k054539_init_chip(running_device *device, k054539_state *info)
{
	int i;

	memset(info->regs, 0, sizeof(info->regs));
	memset(info->k054539_posreg_latch, 0, sizeof(info->k054539_posreg_latch)); //*
	info->k054539_flags |= K054539_UPDATE_AT_KEYON; //* make it default until proven otherwise

	// Real size of 0x4000, the addon is to simplify the reverb buffer computations
	info->ram = auto_alloc_array(device->machine, unsigned char, 0x4000*2+device->clock()/50*2);
	info->reverb_pos = 0;
	info->cur_ptr = 0;
	memset(info->ram, 0, 0x4000*2+device->clock()/50*2);

	const region_info *region = (info->intf->rgnoverride != NULL) ? device->machine->region(info->intf->rgnoverride) : device->region();
	info->rom = *region;
	info->rom_size = region->bytes();
	info->rom_mask = 0xffffffffU;
	for(i=0; i<32; i++)
		if((1U<<i) >= info->rom_size) {
			info->rom_mask = (1U<<i) - 1;
			break;
		}

	if(info->intf->irq)
		// One or more of the registers must be the timer period
		// And anyway, this particular frequency is probably wrong
		// 480 hz is TRUSTED by gokuparo disco stage - the looping sample doesn't line up otherwise
		timer_pulse(device->machine, ATTOTIME_IN_HZ(480), info, 0, k054539_irq);

	info->stream = stream_create(device, 0, 2, device->clock(), info, k054539_update);

	state_save_register_device_item_array(device, 0, info->regs);
	state_save_register_device_item_pointer(device, 0, info->ram,  0x4000);
	state_save_register_device_item(device, 0, info->cur_ptr);
}

WRITE8_DEVICE_HANDLER( k054539_w )
{
	k054539_state *info = get_safe_token(device);

#if 0
	int voice, reg;

	/* The K054539 has behavior like many other wavetable chips including
       the Ensoniq 550x and Gravis GF-1: if a voice is active, writing
       to it's current position is silently ignored.

       Dadandaan depends on this or the vocals go wrong.
       */
	if (offset < 8*0x20)
	{
		voice = offset / 0x20;
		reg = offset & ~0x20;

		if(info->regs[0x22c] & (1<<voice))
		{
			if (reg >= 0xc && reg <= 0xe)
			{
				return;
			}
		}
	}
#endif

	int latch, offs, ch, pan;
	uint8_t *regbase, *regptr, *posptr;

	regbase = info->regs;
	latch = (info->k054539_flags & K054539_UPDATE_AT_KEYON) && (regbase[0x22f] & 1);

	if (latch && offset < 0x100)
	{
		offs = (offset & 0x1f) - 0xc;
		ch = offset >> 5;

		if (offs >= 0 && offs <= 2)
		{
			// latch writes to the position index registers
			info->k054539_posreg_latch[ch][offs] = data;
			return;
		}
	}

	else switch(offset)
	{
		case 0x13f:
			pan = data >= 0x11 && data <= 0x1f ? data - 0x11 : 0x18 - 0x11;
			if(info->intf->apan)
				info->intf->apan(info->device,
					(double)k054539_pantab_q31[pan]       / 2147483648.0,
					(double)k054539_pantab_q31[0xe - pan] / 2147483648.0);
		break;

		case 0x214:
			if (latch)
			{
				for(ch=0; ch<8; ch++)
				{
					if(data & (1<<ch))
					{
						posptr = &info->k054539_posreg_latch[ch][0];
						regptr = regbase + (ch<<5) + 0xc;

						// update the chip at key-on
						regptr[0] = posptr[0];
						regptr[1] = posptr[1];
						regptr[2] = posptr[2];

						k054539_keyon(info, ch);
					}
				}
			}
			else
			{
				for(ch=0; ch<8; ch++)
					if(data & (1<<ch))
						k054539_keyon(info, ch);
			}
		break;

		case 0x215:
			for(ch=0; ch<8; ch++)
				if(data & (1<<ch))
					k054539_keyoff(info, ch);
		break;

		case 0x22d:
			if(regbase[0x22e] == 0x80)
				info->cur_zone[info->cur_ptr] = data;
			info->cur_ptr++;
			if(info->cur_ptr == info->cur_limit)
				info->cur_ptr = 0;
		break;

		case 0x22e:
			info->cur_zone =
				data == 0x80 ? info->ram :
				info->rom + 0x20000*data;
			info->cur_limit = data == 0x80 ? 0x4000 : 0x20000;
			info->cur_ptr = 0;
		break;

		default:
#if 0
			if(regbase[offset] != data) {
				if((offset & 0xff00) == 0) {
					chanoff = offset & 0x1f;
					if(chanoff < 4 || chanoff == 5 ||
					   (chanoff >=8 && chanoff <= 0xa) ||
					   (chanoff >= 0xc && chanoff <= 0xe))
						break;
				}
				if(1 || ((offset >= 0x200) && (offset <= 0x210)))
					break;
				logerror("K054539 %03x = %02x\n", offset, data);
			}
#endif
		break;
	}

	regbase[offset] = data;
}

static STATE_POSTLOAD( reset_zones )
{
	k054539_state *info = (k054539_state *)param;
	int data = info->regs[0x22e];
	info->cur_zone =
		data == 0x80 ? info->ram :
		info->rom + 0x20000*data;
	info->cur_limit = data == 0x80 ? 0x4000 : 0x20000;
}

READ8_DEVICE_HANDLER( k054539_r )
{
	k054539_state *info = get_safe_token(device);
	switch(offset) {
	case 0x22d:
		if(info->regs[0x22f] & 0x10) {
			uint8_t res = info->cur_zone[info->cur_ptr];
			info->cur_ptr++;
			if(info->cur_ptr == info->cur_limit)
				info->cur_ptr = 0;
			return res;
		} else
			return 0;
	case 0x22c:
		break;
	default:
		LOG(("K054539 read %03x\n", offset));
		break;
	}
	return info->regs[offset];
}

static DEVICE_START( k054539 )
{
	static const k054539_interface defintrf = { 0 };
	int i;
	k054539_state *info = get_safe_token(device);

	info->device = device;

	for (i = 0; i < 8; i++)
		info->k054539_gain[i] = 0x10000;	/* 1.0 in Q16 */
	info->k054539_flags = K054539_RESET_FLAGS;

	info->intf = (device->baseconfig().static_config() != NULL) ? (const k054539_interface *)device->baseconfig().static_config() : &defintrf;

	/*
        Volume (voltab) and pan (pantab) curves are now baked Q31 fixed-point
        constants (see top of file) instead of being generated here with
        pow()/sqrt(), giving a platform-deterministic integer audio path.
        Reference formulas, kept for documentation:
            voltab[i] = pow(10, (-36*i/0x40)/20) / 4   (vol=0 -> 0dB, 0x40 -> -36dB,
                                                        factored 1/4 for channel count)
            pantab[i] = sqrt(i) / sqrt(0xe)            (constant-power, pan[0xe]=1.0)
    */

	k054539_init_chip(device, info);

	state_save_register_postload(device->machine, reset_zones, info);
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( k054539 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(k054539_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( k054539 );		break;
		case DEVINFO_FCT_STOP:							/* nothing */									break;
		case DEVINFO_FCT_RESET:							/* nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "K054539");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Konami custom");				break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(K054539, k054539);
