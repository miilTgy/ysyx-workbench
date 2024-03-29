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

#include <common.h>

#define TEST_EXPR

#ifdef TEST_EXPR
#include "monitor/sdb/sdb.h"
#endif

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

#ifdef TEST_EXPR
  FILE *fp = fopen("/home/miil/ysyx-workbench/nemu/tools/gen-expr/input", "r");
  char exprna[65600];
  uint64_t answer;
  if (fp == NULL) {
    printf("Could not find test file!\n");
    return(-1);
  }
  int i = 0;
  while (fgets(exprna, 65600, fp) != NULL) {
    char *ans;
    ans = strtok(exprna, " ");
    char *ptr;
    answer = strtoul(ans, &ptr, 0);
    // printf("%lu\n", answer);
    if (ans != NULL) {
      char *expr6;
      expr6 = strtok(NULL, "\n");
      // puts(expr6);
      bool suc = true;
      printf("line: %d; ", (i+1));
      uint64_t result = expr(expr6, &suc);
      if (result != answer) {
        printf("Unmatch calculate result occured! ");
        printf("Require: %lu    ", answer);
        printf("But get: %lu    \n", result);
      }
      // printf("%lu\n", result);
    } else {
      printf("Could not read answer, does the input hit the BUTTOM?\n");
      printf("Line: %d", i);
      break;
    }
    i++;
  }

  // expr();
#else
  /* Start engine. */
  engine_start();
#endif

  return is_exit_status_bad();
}
