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
#include <memory/host.h>
#include <memory/vaddr.h>
#include <device/map.h>

#define IO_SPACE_MAX (2 * 1024 * 1024)

static uint8_t *io_space = NULL;
static uint8_t *p_space = NULL;

uint8_t* new_space(int size) {
  uint8_t *p = p_space;
  // page aligned;
  size = (size + (PAGE_SIZE - 1)) & ~PAGE_MASK;
  p_space += size;
  assert(p_space - io_space < IO_SPACE_MAX);
  return p;
}

static void check_bound(IOMap *map, paddr_t addr) {
  if (map == NULL) {
    Assert(map != NULL, "IO address (" FMT_PADDR ") is NULL at pc = " FMT_WORD, addr, cpu.pc);
  } else {
    Assert(addr <= map->high && addr >= map->low,
        "IO address (" FMT_PADDR ") is out of bound {%s} [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
        addr, map->name, map->low, map->high, cpu.pc);
  }
}
// is_write: false = read, true = write
// static bool check_bound(IOMap *map, paddr_t addr, int len, bool is_write) {
//   if (map == NULL) {
//     cpu.csr.mtval = addr;
//     word_t ex = is_write ? 7 : 5;
//     word_t npc = isa_raise_intr(ex, cpu.pc);
//     cpu.pc = npc;
//     return false;
//   }

//   // 这里顺便把 len 也考虑进去，防止末尾越界
//   if (addr < map->low || addr + len - 1 > map->high) {
//     cpu.csr.mtval = addr;
//     word_t ex = is_write ? 7 : 5;
//     word_t npc = isa_raise_intr(ex, cpu.pc);
//     cpu.pc = npc;
//     return false;
//   }

//   return true;
// }

static void invoke_callback(io_callback_t c, paddr_t offset, int len, bool is_write) {
  if (c != NULL) { c(offset, len, is_write); }
}

void init_map() {
  io_space = malloc(IO_SPACE_MAX);
  assert(io_space);
  p_space = io_space;
}

word_t map_read(paddr_t addr, int len, IOMap *map) {
  assert(len >= 1 && len <= 8);
  // if (addr == 0xfffffee8)
  //   printf("read\n");
  check_bound(map, addr);
  //   if (!check_bound(map, addr, len, false)) {
  //   // 抛了异常，直接返回一个占位值
  //   return 0;
  // }
  paddr_t offset = addr - map->low;
  IFDEF(CONFIG_DTRACE, Log("read %d bytes at offset 0x%08x in map "ANSI_FG_RED"%s"ANSI_FG_BLUE, len, offset, map->name);)
  invoke_callback(map->callback, offset, len, false); // prepare data to read
  word_t ret = host_read(map->space + offset, len);
  return ret;
}

void map_write(paddr_t addr, int len, word_t data, IOMap *map) {
  assert(len >= 1 && len <= 8);
  // if (addr == 0xfffffee8)
  //   printf("write\n");
  check_bound(map, addr);
  // if (!check_bound(map, addr, len, true)) {
  //   // 抛了异常，忽略本次写（Linux 会在异常处理里决定下一步）
  //   return;
  // }
  paddr_t offset = addr - map->low;
  IFDEF(CONFIG_DTRACE, Log("write %d bytes at offset 0x%08x in map "ANSI_FG_RED"%s"ANSI_FG_BLUE" with data 0x%016lx", len, offset, map->name, data);)
  host_write(map->space + offset, len, data);
  invoke_callback(map->callback, offset, len, true);
}
