#include "tb_common.h"
#include "Vysyx_25010001_RegisterFile.h"


#define set_write(addr, data) do { \
    TB(DUT(wen)) = 1; \
    TB(DUT(waddr)) = addr; \
    TB(DUT(wdata)) = data; \
} while (0)

#define set_read(addr) do { \
    TB(DUT(raddr)) = addr; \
} while( 0)

TESTBENCH<Vysyx_25010001_RegisterFile> *__TB__;

int main(int argc, char *argv[]) {
    __TB__ = new TESTBENCH<Vysyx_25010001_RegisterFile>(argc, argv);
    TB(sim_init());

    TB(cycles(3));
    int last = 0;
    vluint64_t last_data = 0;
    for (int i=1; i<32; i++) {
        vluint64_t tmp = TB(rand64());
        vluint64_t read_data = 0;
        step({
            set_write(i, tmp); set_read(last);
        });
        read_data = TB(DUT(rdata));
        TB(check_eq(last_data, read_data));
        last_data = tmp;
        last = i;
    }
    TB(cycles(10));

    TB(~TESTBENCH());
    exit(EXIT_SUCCESS);
}