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

#include "macro.h"
#include <stdint.h>
#include <sys/types.h>
#include <utils.h>
#include <device/map.h>

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

/* http://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming */
// NOTE: this is compatible to 16550

#define CH_OFFSET 0

static uint8_t *serial_base = NULL;

static struct termios orig_termios;
static bool tty_inited = false;
static bool stdin_eof = false;

// 退出 NEMU 时恢复终端
static void serial_reset_tty(void) {
  if (!tty_inited) return;
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

// 把 stdin 设成“原始模式”（无回显、无行缓冲）
static void serial_init_tty(void) {
  if (tty_inited) return;
  tty_inited = true;

  struct termios t;
  tcgetattr(STDIN_FILENO, &t);
  orig_termios = t;

  t.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &t);

  atexit(serial_reset_tty);
}

// 有没有待读的键盘输入（非阻塞）
static bool serial_stdin_ready(void) {
  if (stdin_eof) return false;
  int n = 0;
  if (ioctl(STDIN_FILENO, FIONREAD, &n) < 0) {
    stdin_eof = true;
    return false;
  }
  return n > 0;
}

// 真正从 stdin 读一个字节（只在 ready 时调用）
static uint8_t serial_getc(void) {
  if (stdin_eof) return 0;
  unsigned char ch = 0;
  int r = read(STDIN_FILENO, &ch, 1);
  if (r <= 0) {
    stdin_eof = true;
    return 0;
  }
  return ch;
}

static void serial_putc(char ch) {
  MUXDEF(CONFIG_TARGET_AM, putch(ch), putc(ch, stderr));
}
// static uint8_t serial_getc() {
//   const char *str = "ls\n";
//   static int i = 0;
//   if (str[i] != '\0') {
//     return str[i++];
//   }
//   else {
//     return 0;
//   }
// }
static void serial_io_handler(uint32_t offset, int len, bool is_write) {
  assert(len == 1);
  switch (offset) {
    /* We bind the serial port with the host stderr in NEMU. */
    case CH_OFFSET:
      if (is_write) serial_putc(serial_base[0]);
      // else panic("do not support read");
      else {
        if (serial_stdin_ready()) {
          serial_base[CH_OFFSET] = serial_getc();
        } else {
          serial_base[CH_OFFSET] = 0;
        }
      }
      break;
    case 3:  // LCR，配置参数；简单实现：读写都忽略
      if (!is_write) {
        serial_base[offset] = 0x00;
      }
      break;
    case 5: { // LSR，线路状态寄存器
      if (!is_write) {
        uint8_t lsr = 0x60;          // THRE/TEMT = 1，发送始终就绪
        if (serial_stdin_ready()) {
          lsr |= 0x01;               // Data Ready，如果有键盘数据
        }
        serial_base[offset] = lsr;
      }
      break;
    }
    default: break; // panic("do not support offset = %d", offset);
  }
}

void init_serial() {
  serial_base = new_space(8);
  serial_init_tty();
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("serial", CONFIG_SERIAL_PORT, serial_base, 8, serial_io_handler);
#else
  add_mmio_map("serial", CONFIG_SERIAL_MMIO, serial_base, 8, serial_io_handler);
#endif

}
