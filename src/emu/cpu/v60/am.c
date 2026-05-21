
// NOTE for bit string / field addressing
// ************************************
// cpustate->moddim must be passed as 10 for bit string instructions,
// and as 11 for bit field instructions




// Addressing mode functions and tables
#include "am1.c" // ReadAM
#include "am2.c" // ReadAMAddress
#include "am3.c" // WriteAM

/*
  Input:
  cpustate->modadd
    cpustate->moddim

  Output:
    cpustate->amout
    amLength
*/

static uint32_t ReadAM(v60_state *cpustate)
{
	cpustate->modm = cpustate->modm?1:0;
	cpustate->modval = OpRead8(cpustate->program, cpustate->modadd);
	return AMTable1[cpustate->modm][cpustate->modval >> 5](cpustate);
}

static uint32_t BitReadAM(v60_state *cpustate)
{
	cpustate->modm = cpustate->modm?1:0;
	cpustate->modval = OpRead8(cpustate->program, cpustate->modadd);
	return BAMTable1[cpustate->modm][cpustate->modval >> 5](cpustate);
}



/*
  Input:
  cpustate->modadd
    cpustate->moddim

  Output:
    cpustate->amout
    cpustate->amflag
    amLength
*/

static uint32_t ReadAMAddress(v60_state *cpustate)
{
	cpustate->modm = cpustate->modm?1:0;
	cpustate->modval = OpRead8(cpustate->program, cpustate->modadd);
	return AMTable2[cpustate->modm][cpustate->modval >> 5](cpustate);
}

static uint32_t BitReadAMAddress(v60_state *cpustate)
{
	cpustate->modm = cpustate->modm?1:0;
	cpustate->modval = OpRead8(cpustate->program, cpustate->modadd);
	return BAMTable2[cpustate->modm][cpustate->modval >> 5](cpustate);
}

/*
  Input:
  cpustate->modadd
    cpustate->moddim
    cpustate->modwritevalb / H/W

  Output:
    cpustate->amout
    amLength
*/

static uint32_t WriteAM(v60_state *cpustate)
{
	cpustate->modm = cpustate->modm?1:0;
	cpustate->modval = OpRead8(cpustate->program, cpustate->modadd);
	return AMTable3[cpustate->modm][cpustate->modval >> 5](cpustate);
}


