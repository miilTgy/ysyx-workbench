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

#include "common.h"
#include <isa.h>
#include "../../../../include/device/clint.h"

word_t isa_raise_intr(word_t EXcode, vaddr_t epc) {
  if (EXcode == 3) { // Breakpoint exception
    printf("Breakpoint exception at PC: 0x%016lx\n", epc);
  }
  /* Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  cpu.csr.mepc = epc; // mepc = pc
  cpu.csr.mcause = cpu.csr.mcause & ~(1LU<<63); // mcause[63](mcause.INT) = 1'b0
  cpu.csr.mcause = EXcode; // mcause = exception code
  word_t mstatus = cpu.csr.mstatus;
  mstatus = (mstatus & ~(3ull << 11)) | ((cpu.priv & 3ull) << 11); // mstatus.MPP(12,11) = current privilege mode
  // 保存 MIE 到 MPIE，并关中断
  word_t mie = (mstatus >> 3) & 1;
  mstatus = (mstatus & ~(1ull << 7)) | (mie << 7); // MPIE = MIE
  mstatus &= ~(1ull << 3);                         // MIE = 0
  cpu.csr.mstatus = mstatus;
  // 进入 M 模式
  cpu.priv = 3;
  // 返回中断入口地址 mtvec
  word_t eea = cpu.csr.mtvec & ~(0x3ull);
  return eea;
}

word_t isa_mret() {
  // word_t mpp = cpu.csr.mstatus & ((1<<11) + (1<<12));
  // if ((mpp>>11) != 0b11) {
  //   cpu.csr.mstatus = cpu.csr.mstatus & ~(1<<17); // MPRV = 0
  // }

  // word_t mpie = cpu.csr.mstatus & 0x80;
  // cpu.csr.mstatus = cpu.csr.mstatus | (mpie >> 4); // MIE = MPIE
  word_t mstatus = cpu.csr.mstatus;
  //take out MPP
  word_t mpp = (mstatus >> 11) & 0x3;
  cpu.priv = mpp; // 恢复特权级
  // MIE = MPIE, MPIE = 1
  word_t mpie = (mstatus >> 7) & 0x1;
  mstatus = (mstatus & ~(1ull << 3)) | (mpie << 3); // MIE = MPIE
  mstatus = mstatus | (1ull << 7);               // MPIE = 1
  // clear MPP
  mstatus = mstatus & ~(3ull << 11);
  cpu.csr.mstatus = mstatus;
  // cpu.csr.mstatus = (((cpu.csr.mstatus >> 7) & 0x1) == 1) ? (cpu.csr.mstatus | (1<<3)) : (cpu.csr.mstatus & ~(1<<3)); // mstatus.MIE(3) = MPIE(7)
  // cpu.csr.mstatus = cpu.csr.mstatus | (1 << 7); // mstatus.MPIE(7) = 1

  // switch ((mpp>>11)) {
  //   case 0b00: /* Change Privilege Mode to U */ break;
  //   case 0b01: /* Change Privilege Mode to S */ break;
  //   case 0b11: /* Change Privilege Mode to M */ break;
  //   default: break;
  // }

  /* if U-extension implanted MPP=2'b00 else MPP=2'b11; */
  // cpu.csr.mstatus = cpu.csr.mstatus & ~((1<<11)+(1<<12));

  return cpu.csr.mepc;
}

word_t isa_wfi(vaddr_t pc) {
  cpu.csr.mstatus = cpu.csr.mstatus | (1<<3); // enable MIE
  return pc + 4;
}

word_t isa_query_intr() {
  // Check software interrupt first (higher priority)
  if (clint_software_intr_pending() && 
    ((cpu.csr.mie >> 3) & 0x1) == 1 &&  // MIE.MSIE = 1
    ((cpu.csr.mstatus >> 3) & 0x1) == 1) { // MSTATUS.MIE = 1
    printf("CLINT: Returning software interrupt\n");
    return IRQ_SOFTW;  // Machine software interrupt
  }
  
  // Check timer interrupt
  if (clint_timer_intr_pending() && 
    ((cpu.csr.mie >> 7) & 0x1) == 1 &&  // MIE.MTIE = 1
    ((cpu.csr.mstatus >> 3) & 0x1) == 1) { // MSTATUS.MIE = 1
    // printf("CLINT: Returning timer interrupt\n");
    return IRQ_TIMER;  // Machine timer interrupt
  }

  if (((cpu.csr.mie >> 7) & 0x1) == 1 && // MIE.MTIE = 1
      ((cpu.csr.mstatus >> 3) & 0x1) == 1 && // MSTATUS.MIE = 1
      (cpu.INTR)) {
    cpu.INTR = false;
    return IRQ_TIMER;
  }
  return INTR_EMPTY;
}
