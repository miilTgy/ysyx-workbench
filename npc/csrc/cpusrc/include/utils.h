#ifndef __UTILS_H__
#define __UTILS_H__

#include "common.h"
#include "debug.h"
#include "isa-def.h"
#include "devices/timer.h"

// Create NPE State
enum NPC_STATE { NPC_END, NPC_ABORT, NPC_RUNNING };
NPC_STATE npc_state = NPC_RUNNING;

// Create memory
#define PG_ALIGN __attribute((aligned(4096)))
#define CONFIG_MSIZE 0x8000000

// Addr conversion
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};

static inline bool in_pmem(uint32_t addr) {
    return addr - CONFIG_MBASE < CONFIG_MSIZE;
  }

uint8_t* guest_to_host(uint32_t paddr) { return pmem + paddr - CONFIG_MBASE; }
uint32_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

// host_read(void *addr, int len);
static inline uint64_t host_read(void *addr, int len) {
    return *(uint64_t *) addr;
}
// host_write(void *addr, int len, uint64_t data);
static inline void host_write(void *addr, int len, uint64_t data) {
    switch (len) {
    case 1: *(uint8_t *)addr = data; return;
    case 2: *(uint16_t *)addr = data; return;
    case 4: *(uint32_t *)addr = data; return;
    case 8: *(uint64_t *)addr = data; return;
    default: printf("%s[HIT] illegal write addr%s\n", ANSI_BG_RED, ANSI_NONE); npc_state = NPC_ABORT; break;
    }
}

// mem img
static const uint32_t img [] = {
    0x00000297,  // auipc t0,0
    0x00028823,  // sb  zero,16(t0)
    0x0102c503,  // lbu a0,16(t0)
    0x00100073,  // ebreak (used as nemu_trap)
    0xdeadbeef,  // some data
};  

// init_pmem()
#define RESET_VECTOR CONFIG_MBASE
void init_imem() {
    memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));
}

static char *img_file = NULL;
static char *diff_so_file = NULL;
static int difftest_port = 1234;

// Load imgs
static long load_img() {
    if (img_file == NULL) {
      printf("%sNo image is given. Use the default build-in image.%s\n", ANSI_BG_RED, ANSI_NONE);
      return 4096; // built-in image size
    }
  
    FILE *fp = fopen(img_file, "rb");
    if (fp == NULL) {
        printf("%simg_file doesn't exist: %s%s\n", ANSI_BG_RED, img_file, ANSI_NONE);
    }
  
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
  
    printf("The image is %s, size = %ld\n", img_file, size);
  
    fseek(fp, 0, SEEK_SET);
    int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
    assert(ret == 1);
  
    fclose(fp);
    return size;
  }  
static int parse_args(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]=%s ", i, argv[i]);
    }
    puts("");
    const struct option table[] = {
    //   {"batch"    , no_argument      , NULL, 'b'},
    //   {"log"      , required_argument, NULL, 'l'},
      {"diff"     , required_argument, NULL, 'd'},
    //   {"port"     , required_argument, NULL, 'p'},
      // {"img"      , required_argument, NULL, 'i'},
      {"help"     , no_argument      , NULL, 'h'},
      {0          , 0                , NULL,  0 },
    };
    int o;
    while ( (o = getopt_long(argc, argv, "-bhl:d:p:", table, NULL)) != -1) {
      switch (o) {
        // case 'b': sdb_set_batch_mode(); break;
        // case 'p': sscanf(optarg, "%d", &difftest_port); break;
        // case 'l': log_file = optarg; break;
        case 'd': diff_so_file = optarg; break;
        case 1: if (optarg[0] != '+') img_file = optarg; printf("img_file=%s\n", img_file); return 0;
        default:
          printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
          printf("\t-b,--batch              run with batch mode\n");
          printf("\t-l,--log=FILE           output log to FILE\n");
          printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
          printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
          printf("\n");
          exit(0);
      }
    }
    return 0;
  }

void init_difftest(char *ref_so_file, long img_size, int port);
void init_npc(int argc, char *argv[]) {
    /* Parse arguments. */
    parse_args(argc, argv);

    /* Load the image to memory. This will overwrite the built-in image. */
    long img_size = load_img();

#ifdef CONFIG_DIFFTEST
    /* Init difftest */
    if (diff_so_file != NULL) {
      std::cout << "difftest init" << std::endl;
      init_difftest(diff_so_file, img_size, difftest_port);
    }
#endif
}

// Define cpu state
CPU_state cpu = {};
CPU_state ref = {};
#define SET_GPR(i) cpu.gpr[i] = tb->dut->rootp->CPU__DOT__gpr__DOT__regs_##i;
void set_cpu() {
  cpu.pc = tb->dut->rootp->CPU__DOT__ifu__DOT__pc;
  cpu.gpr[0] = 0;
  SET_GPR(1)  SET_GPR(2)  SET_GPR(3)  SET_GPR(4)
  SET_GPR(5)  SET_GPR(6)  SET_GPR(7)  SET_GPR(8)
  SET_GPR(9)  SET_GPR(10) SET_GPR(11) SET_GPR(12)
  SET_GPR(13) SET_GPR(14) SET_GPR(15) SET_GPR(16)
  SET_GPR(17) SET_GPR(18) SET_GPR(19) SET_GPR(20)
  SET_GPR(21) SET_GPR(22) SET_GPR(23) SET_GPR(24)
  SET_GPR(25) SET_GPR(26) SET_GPR(27) SET_GPR(28)
  SET_GPR(29) SET_GPR(30) SET_GPR(31)
}


#endif