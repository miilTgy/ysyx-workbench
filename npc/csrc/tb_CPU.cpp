#include "cpusrc/include/utils.h"
#include "cpusrc/include/difftest.h"

// Definition of paddr_rear from DPI-C
extern "C" int pimem_read(unsigned long long paddr) {
    int len = 4;
    uint64_t ret = 0;
    if (in_pmem(paddr)) {
        ret = (uint32_t) host_read(guest_to_host(paddr), len);
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
        // std::cout << "load low addr 0x" << std::hex << raddr << " data " << *(uint64_t *) (pmem + raddr - CONFIG_MBASE) << std::dec << std::endl;
        return host_read(guest_to_host(raddr), 8);
    } else if (raddr == CONFIG_RTC_MMIO) {
        // std::cout << "visit RTC at pc=0x" << std::hex << TB(DUT(io_pc)) << std::dec << std::endl;
        #ifdef CONFIG_DIFFTEST
        difftest_skip_ref();
        #endif
        return get_time() - soc_start_time;
    } else if (raddr == CONFIG_SERIAL_MMIO) {
        // std::cout << "read serial at pc=0x" << std::hex << TB(DUT(io_pc)) << std::dec << std::endl;
        #ifdef CONFIG_DIFFTEST
        difftest_skip_ref();
        #endif
    } else {
        printf("%s[HIT] RD PMEM addr=0x%08llx out of bound at pc = 0x%lx%s\n", ANSI_BG_RED, raddr, TB(DUT(io_pc)), ANSI_NONE);
        // if (TB(DUT(io_pc)) > CONFIG_MBASE && TB(DUT(io_pc)) < CONFIG_MSIZE + CONFIG_MSIZE)
        npc_state = NPC_ABORT;
    }
    return 0;
}

extern "C" void pmem_write(unsigned long long waddr, unsigned long long wdata, unsigned char wmask) {
    int len = 0;
    uint64_t data = wdata;
    switch (wmask) {
        case 0x01: len = 1; data = data & 0x00000000000000ff; break;
        case 0x03: len = 2; data = data & 0x000000000000ffff; break;
        case 0x0f: len = 4; data = data & 0x00000000ffffffff; break;
        case 0xff: len = 8; data = data & 0xffffffffffffffff; break;
        default: len = 0; break;
    }
    // printf("store addr 0x%016llx len %02x data %016lx\n", waddr, len, data);
    if (in_pmem(waddr)) {
        host_write(guest_to_host(waddr), len, data);
        // printf("after store, mem@0x%016llx = %016lx\n", waddr, *(uint64_t *) (pmem + waddr - CONFIG_MBASE));
    } else if (waddr == CONFIG_SERIAL_MMIO) {
        // std::cout << "write serial data " << std::endl;
        #ifdef CONFIG_DIFFTEST
        difftest_skip_ref();
        #endif
        // std::cout << "pc=0x" << std::hex << TB(DUT(io_pc)) << std::dec << " write serial data " << (uint8_t) data << std::endl;
        putchar((uint8_t) data);
    } else {
        printf("%s[HIT] WR PMEM addr=0x%08llx out of bound at pc = 0x%lx%s\n", ANSI_BG_RED, waddr, TB(DUT(io_pc)), ANSI_NONE);
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
    #endif
    std::cout << "[OK] copy regs to ref" << std::endl;
    init_rtc();
    while (npc_state != NPC_END) {
        cycle_num ++;
        // std::cout << "cycle_num " << cycle_num
        // << " pc " << std::hex << TB(DUT(io_pc))
        // << " inst " << tb->dut->rootp->CPU__DOT___imem_ioIDU_inst_data
        // << std::dec << std::endl;
        set_cpu();
        TB(cycles(1));
        if (npc_state == NPC_ABORT) {
            printf("%s[HIT] NPC_ABORT%s\n", ANSI_BG_RED, ANSI_NONE);
            break;
        } else if (npc_state == NPC_END) {
            printf("%s[HIT] NPC_END%s\n", ANSI_BG_GREEN, ANSI_NONE);
            break;
        } else if (cycle_num > 100000) {
            printf("%s[HIT] NPC_TIMEOUT%s\n", ANSI_BG_YELLOW, ANSI_NONE);
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