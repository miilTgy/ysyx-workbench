#ifndef __ISA_RISCV_H__
#define __ISA_RISCV_H__

#include "common.h"

typedef struct {
    reg_t gpr[32];
    MCSR_state csr;
    vaddr_t pc;
} CPU_state;

#endif