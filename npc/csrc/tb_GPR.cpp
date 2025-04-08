#include "tb_common.h"
#include "VGPR.h"


#define set_write(addr, data) do { \
    TB(DUT(GPRio_wen)) = 1; \
    TB(DUT(GPRio_rd)) = addr; \
    TB(DUT(dataInio_dataWB)) = data; \
} while (0)

#define set_read(addr) do { \
    TB(DUT(GPRio_src1)) = addr; \
} while( 0)

TESTBENCH<VGPR> *__TB__;

int main(int argc, char *argv[]) {
    __TB__ = new TESTBENCH<VGPR>(argc, argv);
    TB(sim_init());

    TB(cycles(3));
    int last = 0;
    vluint64_t last_data = 0;
    for (int i=1; i<32; i++) {
        vluint64_t tmp = TB(rand64());
        vluint64_t read_data = 0;

        TB(cycles)([&](){
            set_write(i, tmp); set_read(last);
        });

        read_data = TB(DUT(dataOutio_data1));
        TB(check_eq(last_data, read_data));

        last_data = tmp;
        last = i;
    }
    TB(cycles(10));

    TB(~TESTBENCH());
    exit(EXIT_SUCCESS);
}