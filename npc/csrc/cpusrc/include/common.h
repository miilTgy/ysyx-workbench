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

#define CONFIG_MBASE 0x80000000
#define CONFIG_SERIAL_MMIO 0xa00003f8
#define CONFIG_RTC_MMIO 0xa0000048
#define CONFIG_I8042_DATA_MMIO 0xa0000060
#define CONFIG_FB_ADDR 0xa1000000
#define CONFIG_VGA_CTL_MMIO 0xa0000100
#define CONFIG_SB_ADDR 0xa1200000
#define CONFIG_SB_SIZE 0x10000
#define CONFIG_AUDIO_CTL_MMIO 0xa0000200
#define CONFIG_DISK_CTL_MMIO 0xa0000300
#define IO_SPACE_MAX (2 * 1024 * 1024)

#endif
