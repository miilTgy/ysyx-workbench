#include "tb_common.h"
#include "Vysyx_25010001_RegisterFile.h"

#define set_write(tb, addr, data) do { \
    tb->dut->wen = 1; \
    tb->dut->waddr = addr; \
    tb->dut->wdata = data; \
} while (0)

#define set_read(tb, addr) do { \
    tb->dut->raddr = addr; \
} while(0)


TESTBENCH<Vysyx_25010001_RegisterFile> *tb;

int main(int argc, char *argv[]) {
    tb = new TESTBENCH<Vysyx_25010001_RegisterFile>(argc, argv);

    tb->sim_init();
    tb->dut->clk = 1;
    tb->sim_cycles(3);
    int last = 0;
    for (int i=0; i<32; i++) {
        step(tb, {
            set_write(tb, i, tb->rand64()); set_read(tb, last);
        });
        last = i;
    }
    tb->sim_cycles(10);

    tb->~TESTBENCH();
    std::cout << "sim finished." << std::endl;
    exit(EXIT_SUCCESS);
}