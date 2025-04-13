#ifndef __ISA_RISCV_H__
#define __ISA_RISCV_H__

#include "common.h"

typedef struct {
    reg_t gpr[32];
    vaddr_t pc;
} CPU_state;

#endif