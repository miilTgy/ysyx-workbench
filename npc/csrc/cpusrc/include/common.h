#ifndef __COMMON_H__
#define __COMMON_H__

#include <iostream>

#include <unistd.h>
#include <getopt.h>

#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include <assert.h>
#include <stdlib.h>

#include "../../tb_common.h"
#include "VCPU.h"
#include "VCPU__Dpi.h"
#include "VCPU___024root.h"

typedef uint64_t word_t;
typedef int64_t sword_t;

typedef word_t reg_t;
typedef uint32_t paddr_t;
typedef uint64_t vaddr_t;
typedef uint16_t ioaddr_t;

// Instance common TB
TESTBENCH<VCPU> *__TB__;

#endif
