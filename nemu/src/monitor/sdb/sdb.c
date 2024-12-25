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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args) {
  char *exe_times = strtok(NULL, " ");
  int times = 0;
  if (exe_times == NULL) {
    times = 1;
  } else {
    sscanf( exe_times, "%d", &times);
  }
  if (times <= 0) {
    printf("ERROR: execution less than 1 time");
  } else {
    cpu_exec(times);
  }
  return 0;
}

void desplay_wp();
static int cmd_info(char *args) {
  char *target = strtok(NULL, " ");
  if (target == NULL) {
    printf("ERROR: missing info target: r/w\n");
    return 0;
  }
  // printf("target=%s\n", target);
  if (strcmp(target, "r") == 0) {
    isa_reg_display();
  } else {
    // printf("watchpoint featre under development...\n");
    desplay_wp();
  }
  
  return 0;
}

static int cmd_p(char *args) {
  bool flag = true;
  expr(args, &flag);
  return 0;
}

word_t paddr_read(paddr_t addr, int len);
static int cmd_x(char *args) {
  char *scan_num = strtok(NULL, " ");
  char *scan_addr = strtok(NULL, " ");

  if ((scan_num == NULL) && (scan_addr == NULL)) {
    printf("ERROR: N & EXPR can not be NULL.\n");
    return 0;
  } else if (scan_num == NULL) {
    printf("ERROR: N can not be NULL.\n");
    return 0;
  } else if (scan_addr == NULL) {
    printf("ERROR: EXPR can not be NULL.\n");
    return 0;
  }

  if ((strspn(scan_num, "0123456789")!=strlen(scan_num))) {
    printf("Invalid N: A number is required\n");
    return 0;
  }
  // if ((scan_addr[0] != '0') || (scan_addr[1] != 'x') || (strspn(scan_num, "0123456789x")!=strlen(scan_num))) {
  //   printf("Invalid Address: A hex number required.\n");
  //   return 0;
  // }
  int num = 0;
  sscanf( scan_num, "%d", &num);
  paddr_t addr = 0;
  sscanf( scan_addr, "%x", &addr);
  bool flag = true;
  word_t addr_to_read = expr(scan_addr, &flag);
  for (int i = 0; i < num; i++) {
    printf("Address=0x%lx, value=0x%08lx\n", addr_to_read, paddr_read(addr_to_read, 4));
    addr_to_read += 4;
  }
  return 0;
}

void create_awatchpoint();
static int cmd_w(char *args) {
  char *expr = strtok(NULL, " ");
  if (expr == NULL) {
    printf("ERROR: watchpoint expression can't be NULL.\n");
    return 0;
  }
  create_awatchpoint(expr);
  return 0;
}

int delete_watchpoint(int NO);
static int cmd_d(char *args) {
  char *dNO = strtok(NULL, " ");
  if (dNO != NULL) {
    return delete_watchpoint(atoi(dNO));
  } else {
    printf("ERROR: watchpoint NO can't be NULL.\n");
    return 0;
  }
  
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "help [command], Display information about all supported commands", cmd_help },
  { "c", "c, Continue the execution of the program", cmd_c },
  { "q", "q, Exit NEMU", cmd_q },
  { "si", "si [N], Single-step excution, N is No. of steps default 1", cmd_si},
  { "info", "info r/w, Show register value OR Show watch point info", cmd_info},
  { "x", "x N EXPR, Scan memory, N is No. of address, EXPR is starting address", cmd_x},
  { "p", "p EXPR, Calculate the value of EXPR", cmd_p},
  { "w", "w EXPR, Set watchpoint", cmd_w},
  { "d", "d NO , Delete watchpoint NO", cmd_d}
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
