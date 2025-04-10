#include "tb_common.h"
#include "VIMEM_d.h"
#include "VIMEM_d__Dpi.h"

// Create memory
#define PG_ALIGN __attribute((aligned(4096)))
#define CONFIG_MSIZE 0x8000000
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};

// Addr conversion
#define CONFIG_MBASE 0x80000000
uint8_t* guest_to_host(uint32_t paddr) { return pmem + paddr - CONFIG_MBASE; }
uint32_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

// host_read(void *addr, int len);
static inline uint64_t host_read(void *addr, int len) {
    return *(uint32_t *) addr;
}

// Instance common TB
TESTBENCH<VIMEM_d> *__TB__;

// Definition of paddr_rear from DPI-C
extern "C" int pimem_read(unsigned long long paddr) {
    int len = 4;
    uint64_t ret = host_read(guest_to_host(paddr), len);
    return ret;
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
void init_pmem() {
    memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));
}

int main(int argc, char *argv[]) {
    __TB__ = new TESTBENCH<VIMEM_d>(argc, argv);

    init_pmem();
    std::cout << std::hex << *(uint32_t *) guest_to_host(0x80000000) << std::dec << std::endl;
    TB(DUT(IFUio_pc) = 0x80000000); TB(init_comb([](){}));
    for (int i = 0; i < 5; i++) {
        TB(step_comb([&](){
            TB(DUT(IFUio_pc) = 0x80000000 + i * 4);
        }));
        TB(check_eq(*(uint32_t *)guest_to_host(TB(DUT(IFUio_pc))), TB(DUT(ioIDU_inst_data))));
    }
    TB(step_comb([](){}));

    TB(~TESTBENCH());
    exit(EXIT_SUCCESS);
}