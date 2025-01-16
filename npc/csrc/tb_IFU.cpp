#include "tb_common.h"
#include "Vysyx_25010001_IFU.h"


TESTBENCH<Vysyx_25010001_IFU> *__TB__;

int main(int argc, char *argv[]) {
    __TB__ = new TESTBENCH<Vysyx_25010001_IFU>(argc, argv);
    TB(sim_init());
    TB(sim_reset());
    TB(cycles(10));

    TB(~TESTBENCH());
    exit(EXIT_SUCCESS);
}