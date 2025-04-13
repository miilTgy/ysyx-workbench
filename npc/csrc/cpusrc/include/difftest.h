#ifndef __DIFFTEST_H__
#define __DIFFTEST_H__

#include <dlfcn.h>

#include "utils.h"
#include "debug.h"

enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };

void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(uint64_t NO) = NULL;

void isa_reg_display();
void init_difftest(char *ref_so_file, long img_size, int port) {
    assert(ref_so_file != NULL);

    void *handle;
    handle = dlopen(ref_so_file, RTLD_LAZY);
    assert(handle);
  
    ref_difftest_memcpy = (void (*)(paddr_t addr, void *buf, size_t n, bool direction)) dlsym(handle, "difftest_memcpy");
    assert(ref_difftest_memcpy);
  
    ref_difftest_regcpy = (void (*)(void *dut, bool direction)) dlsym(handle, "difftest_regcpy");
    assert(ref_difftest_regcpy);
  
    ref_difftest_exec = (void (*)(uint64_t n)) dlsym(handle, "difftest_exec");
    assert(ref_difftest_exec);
  
    ref_difftest_raise_intr = (void (*)(uint64_t NO)) dlsym(handle, "difftest_raise_intr");
    assert(ref_difftest_raise_intr);
  
    void (*ref_difftest_init)(int) = (void (*)(int)) dlsym(handle, "difftest_init");
    assert(ref_difftest_init);
  
    printf("Differential testing: %s\n", ANSI_FMT("ON", ANSI_FG_GREEN));
    printf("The result of every instruction will be compared with %s. \n"
        "This will help you a lot for debugging, but also significantly reduce the performance. \n"
        "If it is not necessary, you can turn it off in menuconfig.\n", ref_so_file);
  
    std::cout << "Starting REF difftest init ..." << std::endl;
    (*ref_difftest_init)(port);
    std::cout << "[OK] REF difftest init" <<std::endl;

    std::cout << "Starting REF difftest memcpy ..." << std::endl;
    ref_difftest_memcpy(0x80000000, guest_to_host(RESET_VECTOR), img_size, DIFFTEST_TO_REF);
    std::cout << "[OK] REF difftest memcpy" << std::endl;

    std::cout << "Starting REF difftest regcpy ..." << std::endl;
    set_cpu();
    // std::cout << "DUT REGS:" << std::endl;
    // isa_reg_display();
    // std::cout << "REF REGS:" << std::endl;
    ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
    std::cout << "[OK] REF difftest regcpy" << std::endl;
}

/* DiffTest Reg Check */
const char *regs[] = {
    "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
  };  
static inline int check_reg_idx(int idx) {
    assert(idx >= 0 && idx < 32);
    return idx;
  }  
static inline const char* reg_name(int idx) {
    extern const char* regs[];
    return regs[check_reg_idx(idx)];
}  
bool isa_difftest_checkregs(CPU_state *ref_r, CPU_state *dut_r) {
    for (size_t i=0; i<32; i++) {
      if (dut_r->gpr[i] != ref_r->gpr[i]) {
        printf("Diff error: reg %s: DUT = 0x%016lx, REF = 0x%016lx\n", reg_name(i), dut_r->gpr[i], ref_r->gpr[i]);
        printf("DUT PC = 0x%lx; REF PC = 0x%lx\n", dut_r->pc, ref_r->pc);
        return false;
      }
    }
    if (dut_r->pc != ref_r->pc) {
      printf("Diff error: DUT pc = 0x%lx, REF pc = 0x%lx\n", dut_r->pc, ref_r->pc);
      return false;
    }
    return true;
  }
void isa_reg_display() {
    int num = sizeof(regs) / sizeof(regs[0]);
    for (int i = 0; i < num; i++) {
        printf("Reg$\033[1;31m%-4s\033[m 0x%016lx\t", regs[i], cpu.gpr[i]);
        if ((i + 1) % 3 == 0) {
            putchar('\n');
        }
    }
    printf("Reg$\033[1;31m%-4s\033[m 0x%08lx\n", "cpu.pc", cpu.pc);
}
void ref_reg_display(CPU_state *ref) {
    for (int i = 0; i < 32; i++) {
        printf("Reg$\033[1;31m%-4s\033[m 0x%016lx\t", regs[i], ref->gpr[i]);
        if ((i + 1) % 3 == 0) {
            putchar('\n');
        }
    }
    printf("Reg$\033[1;31m%-4s\033[m 0x%08lx\n", "ref->pc", ref->pc);
}
static void checkregs(CPU_state *ref, CPU_state *dut, vaddr_t pc) {
    if (!isa_difftest_checkregs(ref, dut)) {
        npc_state = NPC_ABORT;
        std::cout << "DUT REGS:" << std::endl;
        isa_reg_display();
        std::cout << "REF REGS:" << std::endl;
        ref_reg_display(ref);
    }
}

void difftest_step(vaddr_t pc) {
    ref_difftest_exec(1);
    CPU_state ref;
    ref_difftest_regcpy(&ref, DIFFTEST_TO_DUT);
    set_cpu();
    checkregs(&ref, &cpu, pc);
}

#endif