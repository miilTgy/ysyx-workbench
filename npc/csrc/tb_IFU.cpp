#include "tb_common.h"
#include "VIFU.h"


TESTBENCH<VIFU> *__TB__;

int main(int argc, char *argv[]) {
    __TB__ = new TESTBENCH<VIFU>(argc, argv);
    TB(sim_init());
    TB(sim_reset());
    TB(cycles(10));

    TB(~TESTBENCH());
    exit(EXIT_SUCCESS);
}