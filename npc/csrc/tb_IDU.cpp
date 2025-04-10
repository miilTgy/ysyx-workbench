#include "tb_common.h"
#include "VIDU.h"

#define CONFIG_MBASE 0x80000000

static const uint32_t img [] = {
  0x00000297,  // auipc t0,0
  0x00028823,  // sb    zero,16(t0)
  0x0102c503,  // lbu   a0,16(t0)
  0xffc10113,  // addi  sp,sp,-4
  0x00100073,  // ebreak (used as nemu_trap)
  0x40005013,  // srai x0, 0(x0)
  0xdeadbeef,  // some data
};
vluint64_t sim_pc = 0x80000000;

TESTBENCH<VIDU> *__TB__;

int main(int argc, char *argv[])
{
    __TB__ = new TESTBENCH<VIDU>(argc, argv);
    TB(step_comb([](){ TB(DUT(IMEMio_pc)) = sim_pc; TB(DUT(IMEMio_inst_data)) = img[sim_pc - CONFIG_MBASE]; }));
    sim_pc += 1;
    TB(step_comb([](){ TB(DUT(IMEMio_pc)) = sim_pc; TB(DUT(IMEMio_inst_data)) = img[sim_pc - CONFIG_MBASE]; }));
    sim_pc += 1;
    TB(step_comb([](){ TB(DUT(IMEMio_pc)) = sim_pc; TB(DUT(IMEMio_inst_data)) = img[sim_pc - CONFIG_MBASE]; }));
    sim_pc += 1;
    TB(step_comb([](){ TB(DUT(IMEMio_pc)) = sim_pc; TB(DUT(IMEMio_inst_data)) = img[sim_pc - CONFIG_MBASE]; }));
    sim_pc += 1;
    TB(step_comb([](){ TB(DUT(IMEMio_pc)) = sim_pc; TB(DUT(IMEMio_inst_data)) = img[sim_pc - CONFIG_MBASE]; }));
    sim_pc += 1;
    TB(step_comb([](){ TB(DUT(IMEMio_pc)) = sim_pc; TB(DUT(IMEMio_inst_data)) = img[sim_pc - CONFIG_MBASE]; }));
    sim_pc += 1;
    TB(step_comb([](){ TB(DUT(IMEMio_pc)) = sim_pc; TB(DUT(IMEMio_inst_data)) = img[sim_pc - CONFIG_MBASE]; }));
    sim_pc += 1;
    TB(step_comb([](){ }));
    TB(step_comb([](){ }));
    TB(step_comb([](){ }));

    TB(~TESTBENCH());
    exit(EXIT_SUCCESS);
}

