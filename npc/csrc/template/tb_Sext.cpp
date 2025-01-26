#include "../tb_common.h"
#include "Vysyx_25010001_Sext.h"
#include "verilated_types.h"

TESTBENCH<Vysyx_25010001_Sext> *__TB__;

int main(int argc, char *argv[])
{
    __TB__ = new TESTBENCH<Vysyx_25010001_Sext>(argc, argv);

    TB(init_comb([&](){ TB(DUT(data_i = 0xfff)); }));
    TB(step_comb([&](){ TB(DUT(data_i = 0xfee)); }));
    TB(step_comb([&](){ TB(DUT(data_i = 0xfe0)); }));
    TB(step_comb([&](){ TB(DUT(data_i = 0x0ff)); }));
    TB(step_comb([&](){ TB(DUT(data_i = 0x000)); }));
    TB(step_comb([&](){ TB(DUT(data_i = 0x075)); }));
    TB(step_comb([&](){ TB(DUT(data_i = 0x14f)); }));
    for (int i = 0; i < 16; i++) {
        uint16_t to_data_i = ((uint16_t)TB(rand64())) & (uint16_t) 0xfff;
        TB(step_comb([&]() { TB(DUT(data_i = to_data_i)); }));
    }
    TB(step_comb([](){  }));
    TB(~TESTBENCH());
    exit(EXIT_SUCCESS);
}
