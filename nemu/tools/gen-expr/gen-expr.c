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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static int buf_indx = 0;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"#include <stdint.h>\n"
"int main() { "
"  uint64_t result = %s; "
"  printf(\"%%ld\", result); "
"  return 0; "
"}";

uint64_t choose(uint64_t a) {
  return (rand() % 3);
}

void gen_num() {
  if (buf_indx+1 <= 65536) {
    struct timeval tv;        // get current time us
    gettimeofday(&tv, NULL);  // get current time us
    srand(tv.tv_usec);        // set seed
    int num = rand() % 10;
    char c[20];
    snprintf(c, 2, "%u", num);
    buf[buf_indx] = c[0];
    buf_indx++;
  }
}

void gen(char p) {
  if (buf_indx+1 <= 65536) {
    buf[buf_indx] = p;
    buf_indx++;
  }
}

void gen_rand_op() {
  if (buf_indx+1 <= 65536) {
    struct timeval tv;        // get current time us
    gettimeofday(&tv, NULL);  // get current time us
    srand(tv.tv_usec);        // set seed
    char op[4] = "+-*";
    buf[buf_indx] = op[rand() % 3];
    buf_indx++;
  }
}

void gen_div() {
  if (buf_indx+1 <= 65536) {
    buf[buf_indx] = '/';
    buf_indx++;
  }
}

static int gen_rand_expr() {
  int rr = 0;
  if (buf_indx >= 65536) {
    // printf("\033[0m\033[1;31m%s\033[0m\n", "Expression Oversize!");
    buf_indx = 0;
    buf[0] = '\0';
    return 1;
    // exit(1);
  }
  switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('(');
            rr = gen_rand_expr();
            if (rr == 1) return 1; // check oversize every recursive level
            gen(')');
            break;
    default: 
      rr = gen_rand_expr();
      if (rr == 1) return 1; // check oversize every recursive level
      if (rand() % 4 == 3) {
        gen_div();
        gen('(');
        rr = gen_rand_expr();
        if (rr == 1) return 1; // check oversize every recursive level
        gen('+');
        gen('1');
        gen(')');
        gen_rand_op();
      } else {
        gen_rand_op();
      }
      rr = gen_rand_expr();
      if (rr == 1) return 1; // check oversize every recursive level
      break;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf_indx = 0;
    int r = gen_rand_expr();
    if (r == 1){
      i--;
      continue;
    }
    buf[buf_indx++] = '\0';

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
