#include "tb_common.h"
// #include <iostream>

// #include "verilated.h"
// #include "verilated_vcd_c.h"

#include "VCPU.h"
#include "VCPU__Dpi.h"

// #include "VCPU_IFU.h"

// Create memory
#define PG_ALIGN __attribute((aligned(4096)))
#define CONFIG_MSIZE 0x8000000

// Addr conversion
#define CONFIG_MBASE 0x80000000
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};

static inline bool in_pmem(uint32_t addr) {
    return addr - CONFIG_MBASE < CONFIG_MSIZE;
  }
  
uint8_t* guest_to_host(uint32_t paddr) { return pmem + paddr - CONFIG_MBASE; }
uint32_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

// host_read(void *addr, int len);
static inline uint64_t host_read(void *addr, int len) {
    return *(uint32_t *) addr;
}

// Instance common TB
TESTBENCH<VCPU> *__TB__;

// Definition of paddr_rear from DPI-C
extern "C" int pimem_read(unsigned long long paddr) {
    int len = 4;
    uint64_t ret = 0;
    if (in_pmem(paddr)) {
        ret = host_read(guest_to_host(paddr), len);
    }
    return ret;
}

extern "C" int pmem_read(unsigned long long raddr) {
    return 0;
}

extern "C" void pmem_write(unsigned long long waddr, unsigned long long wdata, char wmask) {
    return;
}

// mem img
static const uint32_t img [] = {
    0x00000297,  // auipc t0,0
    0x00028823,  // sb  zero,16(t0)
    0x0102c503,  // lbu a0,16(t0)
    0x00100073,  // ebreak (used as nemu_trap)
    0xdeadbeef,  // some data
  };  

  // init_pmem()
#define RESET_VECTOR CONFIG_MBASE
void init_imem() {
    memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));
}

int main(int argc, char *argv[]) {
    std::cout << "Starting CPU testbench" << std::endl;
    __TB__ = new TESTBENCH<VCPU>(argc, argv);
    
    std::cout << "Starting MEM init" << std::endl;
    init_imem();
    std::cout << "[OK] MEM init" << std::endl;
    std::cout << std::hex << *(uint32_t *) guest_to_host(0x80000000) << std::dec << std::endl;
    TB(sim_init());
    std::cout << "[OK] SIM init" << std::endl;
    TB(sim_reset());
    std::cout << "[OK] SIM reset" << std::endl;
    TB(cycles(10));
    TB(~TESTBENCH());
    exit(EXIT_SUCCESS);
}