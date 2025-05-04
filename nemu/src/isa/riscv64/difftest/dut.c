/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/difftest.h>
#include "../local-include/reg.h"

#define check_reg(name) \
  if (cpu.name != ref_r->name) { \
    printf("Diff error: reg %s: DUT = 0x%lx, REF = 0x%lx\n", #name, cpu.name, ref_r->name); \
    return false; \
  }

  extern const char *regs[];
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  for (size_t i=0; i<32; i++) {
    if (cpu.gpr[i] != ref_r->gpr[i] /* && strcmp(regs[i], "t0") != 0 && strcmp(regs[i], "t0") != 0 && strcmp(regs[i], "a2") != 0 */) {
      printf("Diff error: reg %s: DUT = 0x%lx, REF = 0x%lx\n", reg_name(i), cpu.gpr[i], ref_r->gpr[i]);
      return false;
    }
  }
  check_reg(pc)
  check_reg(csr.mtvec)
  check_reg(csr.mepc)
  check_reg(csr.mstatus)
  check_reg(csr.mcause)
  return true;
}

void isa_difftest_attach() {
}
