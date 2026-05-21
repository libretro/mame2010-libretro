/*
 * 74181
 *
 * 4-bit arithmetic Logic Unit
 *
 */

#include "emu.h"
#include "74181.h"



#define TTL74181_MAX_CHIPS		(2)
#define TTL74181_INPUT_TOTAL	(14)
#define TTL74181_OUTPUT_TOTAL	(8)



typedef struct _TTL74181_state TTL74181_state;
struct _TTL74181_state
{
	uint8_t inputs[TTL74181_INPUT_TOTAL];
	uint8_t outputs[TTL74181_OUTPUT_TOTAL];
	uint8_t dirty;
};

static TTL74181_state chips[TTL74181_MAX_CHIPS];


void TTL74181_config(running_machine *machine, int which, void *intf)
{
	TTL74181_state *c;

	assert_always(machine->phase() == MACHINE_PHASE_INIT, "Can only call at init time!");
	assert_always(intf == 0, "Interface must be NULL");
	assert_always((which >= 0) && (which < TTL74181_MAX_CHIPS), "Exceeded maximum number of 74181 chips");

	c = &chips[which];

	c->dirty = 1;

	state_save_register_item_array(machine, "TTL74181", NULL, which, c->inputs);
	state_save_register_item_array(machine, "TTL74181", NULL, which, c->outputs);
	state_save_register_item      (machine, "TTL74181", NULL, which, c->dirty);
}


void TTL74181_reset(int which)
{
	/* nothing to do */
}


static void TTL74181_update(int which)
{
	TTL74181_state *c = &chips[which];

	uint8_t a0 =  c->inputs[TTL74181_INPUT_A0];
	uint8_t a1 =  c->inputs[TTL74181_INPUT_A1];
	uint8_t a2 =  c->inputs[TTL74181_INPUT_A2];
	uint8_t a3 =  c->inputs[TTL74181_INPUT_A3];

	uint8_t b0 =  c->inputs[TTL74181_INPUT_B0];
	uint8_t b1 =  c->inputs[TTL74181_INPUT_B1];
	uint8_t b2 =  c->inputs[TTL74181_INPUT_B2];
	uint8_t b3 =  c->inputs[TTL74181_INPUT_B3];

	uint8_t s0 =  c->inputs[TTL74181_INPUT_S0];
	uint8_t s1 =  c->inputs[TTL74181_INPUT_S1];
	uint8_t s2 =  c->inputs[TTL74181_INPUT_S2];
	uint8_t s3 =  c->inputs[TTL74181_INPUT_S3];

	uint8_t cp =  c->inputs[TTL74181_INPUT_C];
	uint8_t mp = !c->inputs[TTL74181_INPUT_M];

	uint8_t ap0 = !(a0 | (b0 & s0) | (s1 & !b0));
	uint8_t bp0 = !(((!b0) & s2 & a0) | (a0 & b0 & s3));
	uint8_t ap1 = !(a1 | (b1 & s0) | (s1 & !b1));
	uint8_t bp1 = !(((!b1) & s2 & a1) | (a1 & b1 & s3));
	uint8_t ap2 = !(a2 | (b2 & s0) | (s1 & !b2));
	uint8_t bp2 = !(((!b2) & s2 & a2) | (a2 & b2 & s3));
	uint8_t ap3 = !(a3 | (b3 & s0) | (s1 & !b3));
	uint8_t bp3 = !(((!b3) & s2 & a3) | (a3 & b3 & s3));

	uint8_t fp0 = !(cp & mp) ^ ((!ap0) & bp0);
	uint8_t fp1 = (!((mp & ap0) | (mp & bp0 & cp))) ^ ((!ap1) & bp1);
	uint8_t fp2 = (!((mp & ap1) | (mp & ap0 & bp1) | (mp & cp & bp0 & bp1))) ^ ((!ap2) & bp2);
	uint8_t fp3 = (!((mp & ap2) | (mp & ap1 & bp2) | (mp & ap0 & bp1 & bp2) | (mp & cp & bp0 & bp1 & bp2))) ^ ((!ap3) & bp3);

	uint8_t aeqb = fp0 & fp1 & fp2 & fp3;
	uint8_t pp = !(bp0 & bp1 & bp2 & bp3);
	uint8_t gp = !((ap0 & bp1 & bp2 & bp3) | (ap1 & bp2 & bp3) | (ap2 & bp3) | ap3);
	uint8_t cn4 = (!(cp & bp0 & bp1 & bp2 & bp3)) | gp;

	c->outputs[TTL74181_OUTPUT_F0]   = fp0;
	c->outputs[TTL74181_OUTPUT_F1]   = fp1;
	c->outputs[TTL74181_OUTPUT_F2]   = fp2;
	c->outputs[TTL74181_OUTPUT_F3]   = fp3;
	c->outputs[TTL74181_OUTPUT_AEQB] = aeqb;
	c->outputs[TTL74181_OUTPUT_P]    = pp;
	c->outputs[TTL74181_OUTPUT_G]    = gp;
	c->outputs[TTL74181_OUTPUT_CN4]  = cn4;
}


void TTL74181_write(int which, int startline, int lines, uint8_t data)
{
	int line;
	TTL74181_state *c;

	assert_always((which >= 0) && (which < TTL74181_MAX_CHIPS), "Chip index out of range");

	c = &chips[which];

	assert_always(c != NULL, "Invalid index - chip has not been configured");
	assert_always(lines >= 1, "Must set at least one line");
	assert_always(lines <= 4, "Can't set more than 4 lines at once");
	assert_always((startline + lines) <= TTL74181_INPUT_TOTAL, "Input line index out of range");

	for (line = 0; line < lines; line++)
	{
		uint8_t input = (data >> line) & 0x01;

		if (c->inputs[startline + line] != input)
		{
			c->inputs[startline + line] = input;

			c->dirty = 1;
		}
	}
}


uint8_t TTL74181_read(int which, int startline, int lines)
{
	int line;
	uint8_t data;
	TTL74181_state *c;

	assert_always((which >= 0) && (which < TTL74181_MAX_CHIPS), "Chip index out of range");

	c = &chips[which];

	assert_always(c != NULL, "Invalid index - chip has not been configured");
	assert_always(lines >= 1, "Must read at least one line");
	assert_always(lines <= 4, "Can't read more than 4 lines at once");
	assert_always((startline + lines) <= TTL74181_OUTPUT_TOTAL, "Output line index out of range");

	if (c->dirty)
	{
		TTL74181_update(which);

		c->dirty = 0;
	}


	data = 0;

	for (line = 0; line < lines; line++)
	{
		data = data | (c->outputs[startline + line] << line);
	}

	return data;
}
