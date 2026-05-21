/*
    Manchester Small-Scale Experimental Machine (SSEM) emulator

    Written by MooglyGuy
*/

#pragma once

#ifndef __SSEM_H__
#define __SSEM_H__

enum
{
    SSEM_PC = 1,
    SSEM_A,
    SSEM_HALT,
};

DECLARE_LEGACY_CPU_DEVICE(SSEM, ssem);

extern offs_t ssem_dasm_one(char *buffer, offs_t pc, uint32_t op);

#endif /* __SSEM_H__ */
