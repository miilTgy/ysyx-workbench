#include "cpusrc/include/utils.h"
#include "cpusrc/include/difftest.h"

// Definition of paddr_rear from DPI-C
extern "C" int pimem_read(unsigned long long paddr) {
    int len = 4;
    uint64_t ret = 0;
    if (in_pmem(paddr)) {
        ret = host_read(guest_to_host(paddr), len);
    } else {
        printf("%s[HIT] IMEM out of bound at pc = 0x%lx%s\n", ANSI_BG_RED, TB(DUT(io_pc)), ANSI_NONE);
    }
    if (ret == 0x00100073) {
        npc_state = NPC_END;
    }
    return ret;
}

extern "C" unsigned long long pmem_read(unsigned long long raddr) {
    if (in_pmem(raddr)) {
        std::cout << "load addr 0x" << std::hex << raddr << " data " << host_read(guest_to_host(raddr), 8) << std::dec << std::endl;
        return host_read(guest_to_host(raddr), 8);
    } else {
        printf("%s[HIT] PMEM out of bound at pc = 0x%lx%s\n", ANSI_BG_RED, TB(DUT(io_pc)), ANSI_NONE);
        if (TB(DUT(io_pc)) > CONFIG_MBASE && TB(DUT(io_pc)) < CONFIG_MSIZE + CONFIG_MSIZE)
            npc_state = NPC_ABORT;
    }
    return 0;
}

extern "C" void pmem_write(unsigned long long waddr, unsigned long long wdata, unsigned char wmask) {
    int len = 0;
    unsigned long long data = wdata;
    switch (wmask) {
    case 0x01: len = 1; data = data & 0x0000000f; break;
    case 0x03: len = 2; data = data & 0x000000ff; break;
    case 0x0f: len = 4; data = data & 0x0000ffff; break;
    case 0xff: len = 8; data = data & 0xffffffff; break;
    default: len = 0; break;
    }
    if (in_pmem(waddr)) {
        host_write(guest_to_host(waddr), len, data);
    } else {
        printf("%s[HIT] PMEM out of bound at pc = 0x%lx%s\n", ANSI_BG_RED, TB(DUT(io_pc)), ANSI_NONE);
        npc_state = NPC_ABORT;
    }
}

int main(int argc, char *argv[]) {
    std::cout << "Starting CPU testbench" << std::endl;
    __TB__ = new TESTBENCH<VCPU>(argc, argv);
    /* You must set pc to 0x80000000 here */
    tb->dut->rootp->CPU__DOT__ifu__DOT__pc = RESET_VECTOR;
    
    std::cout << "Starting MEM init" << std::endl;
    init_imem();
    std::cout << "[OK] MEM init" << std::endl;

    std::cout << "Starting CPU init" << std::endl;
    init_npc(argc, argv);
    std::cout << "[OK] CPU init" << std::endl;

    std::cout << std::hex << *(uint32_t *) guest_to_host(0x80000000) << std::dec << std::endl;

/* Here Don't touch */
    TB(sim_init());
    std::cout << "[OK] SIM init" << std::endl;

    TB(sim_reset());
    std::cout << "[OK] SIM reset" << std::endl;
/* Here Don't touch */
    int cycle_num = 0;
    set_cpu();
    ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
    while (npc_state != NPC_END) {
        cycle_num ++;
        // std::cout << "cycle_num " << cycle_num
        // << " pc " << std::hex << TB(DUT(io_pc))
        // << " inst " << tb->dut->rootp->CPU__DOT___imem_ioIDU_inst_data
        // << std::dec << std::endl;
        set_cpu();
        TB(cycles(1));
        if (npc_state == NPC_ABORT) {
            std::cout << "[HIT] NPC_ABORT" << std::endl;
            break;
        } else if (npc_state == NPC_END) {
            std::cout << "[HIT] NPC_END" << std::endl;
            break;
        } else if (cycle_num > 10000) {
            std::cout << "[HIT] TIMEOUT" << std::endl;
            npc_state = NPC_ABORT;
            break;
        }
        difftest_step(tb->dut->rootp->CPU__DOT__ifu__DOT__pc);
    }
    // TB(cycles(1));
    
    TB(~TESTBENCH());
    if (npc_state == NPC_END)
        exit(EXIT_SUCCESS);
    else
        exit(EXIT_FAILURE);
}