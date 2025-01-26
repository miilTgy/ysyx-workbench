#include "tb_common.h"
#include "Vysyx_25010001_IDU.h"

#define CONFIG_MBASE 0x80000000

static const uint32_t img [] = {
  0x00000297,  // auipc t0,0
  0x00028823,  // sb    zero,16(t0)
  0x0102c503,  // lbu   a0,16(t0)
  0xffc10113,  // addi  sp,sp,-4
  0x00100073,  // ebreak (used as nemu_trap)
  0xdeadbeef,  // some data
};
vluint64_t sim_pc = 0x80000000;

TESTBENCH<Vysyx_25010001_IDU> *__TB__;

int main(int argc, char *argv[])
{
    __TB__ = new TESTBENCH<Vysyx_25010001_IDU>(argc, argv);
    TB(init_comb([](){ }));
    TB(step_comb([](){ TB(DUT(pc)) = sim_pc; TB(DUT(inst)) = img[sim_pc - CONFIG_MBASE]; }));
    sim_pc += 1;
    TB(step_comb([](){ TB(DUT(pc)) = sim_pc; TB(DUT(inst)) = img[sim_pc - CONFIG_MBASE]; }));
    sim_pc += 1;
    TB(step_comb([](){ TB(DUT(pc)) = sim_pc; TB(DUT(inst)) = img[sim_pc - CONFIG_MBASE]; }));
    sim_pc += 1;
    TB(step_comb([](){ TB(DUT(pc)) = sim_pc; TB(DUT(inst)) = img[sim_pc - CONFIG_MBASE]; }));
    sim_pc += 1;
    TB(step_comb([](){ TB(DUT(pc)) = sim_pc; TB(DUT(inst)) = img[sim_pc - CONFIG_MBASE]; }));
    sim_pc += 1;
    TB(step_comb([](){ TB(DUT(pc)) = sim_pc; TB(DUT(inst)) = img[sim_pc - CONFIG_MBASE]; }));
    sim_pc += 1;
    TB(step_comb([](){ }));

    TB(~TESTBENCH());
    exit(EXIT_SUCCESS);
}

