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

#include <elf.h>
#include <isa.h>
#include <memory/paddr.h>
#include "sdb/sdb.h"

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
  // Log("Exercise: Please remove me in the source code and compile NEMU again.");
  // assert(0);
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
#ifdef CONFIG_FTRACE
static char elf_file[256];
#endif
static int difftest_port = 1234;

static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}

#ifdef CONFIG_FTRACE
static void load_elf() {
  if (img_file == NULL) {
    Log("No elf is given.");
    return;
  }
  char elf_tmp[256];
  strcpy(elf_tmp, img_file);
  char *suffix = strrchr(elf_tmp, '.');
  if (suffix != NULL) {
    strcpy(suffix, ".elf");
    strcpy(elf_file, elf_tmp);
    Log("Loading ELF file: %s", elf_file);
    FILE *fp = fopen(elf_file, "rb");
    Assert(fp, "Can not open ELF file '%s'", elf_file);

    // 1. Read ELF header
    Elf64_Ehdr ehdr;
    if (fread(&ehdr, sizeof(Elf64_Ehdr), 1, fp) == 0){
      fclose(fp);
      panic("Can not read ELF header from '%s'", elf_file);
    }

    // 2. Check if it is a valid ELF file
    if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
      fclose(fp);
      Assert(0, "Not a valid ELF file '%s'", elf_file);
    }

    // 3. Locate to Section header
    fseek(fp, ehdr.e_shoff, SEEK_SET);

    // 4. Read Section header
    Elf64_Shdr *shdr_table = (Elf64_Shdr *) malloc (ehdr.e_shnum * ehdr.e_shentsize);
    Assert(shdr_table, "Can not allocate memory for section header table");
    if (fread(shdr_table, ehdr.e_shentsize, ehdr.e_shnum, fp) == 0) {
      fclose(fp);
      panic("Can not read section header from '%s'", elf_file);
    }

    // 5. Find .symtab section and .strtab section
    size_t symtab_idx=0, strtab_idx=0;
    for(size_t i=0; i<ehdr.e_shnum; i++) {
      if (shdr_table[i].sh_type == SHT_SYMTAB) {
        symtab_idx = i;
        strtab_idx = shdr_table[i].sh_link;
      }
    }
    
    // 6. Locate to .strtab head
    fseek(fp, shdr_table[strtab_idx].sh_offset, SEEK_SET);

    // 7. Read .strtab section
    void *strtab = malloc(shdr_table[strtab_idx].sh_size);
    Assert(strtab, "Can not allocate memory for .strtab section");
    if (fread(strtab, shdr_table[strtab_idx].sh_size, 1, fp) == 0) {
      fclose(fp);
      panic("Can not read .strtab section from '%s'", elf_file);
    }
    
    // 8. Locate to .symtab head
    fseek(fp, shdr_table[symtab_idx].sh_offset, SEEK_SET);

    // 9. Read .symtab section
    Elf64_Sym *symtab = (Elf64_Sym *) malloc (shdr_table[symtab_idx].sh_size);
    Assert(symtab, "Can not allocate memory for .symtab section");
    if (fread(symtab, shdr_table[symtab_idx].sh_size, 1, fp) == 0) {
      fclose(fp);
      panic("Can not read .symtab section from '%s'", elf_file);
    }
    for (size_t i=0; i<shdr_table[symtab_idx].sh_size/sizeof(Elf64_Sym); i++) {
      if(ELF64_ST_TYPE(symtab[i].st_info) == STT_FUNC && symtab[i].st_size != 0) {
        fnode_pushback(symtab[i].st_value, symtab[i].st_size, (char *) ((uint64_t) strtab + symtab[i].st_name));
      }
    }
    display_all_f();
  }

}
#endif

static int parse_args(int argc, char *argv[]) {
  for (int i=0; i<argc; i++) {
    printf("%s ", argv[i]);
  }
  puts("");
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 1: img_file = optarg; return 0;
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

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);

  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();
#ifdef CONFIG_FTRACE
  /* Load the ELF file. */
  load_elf();
#endif
  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

#ifndef CONFIG_ISA_loongarch32r
  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv,
      MUXDEF(CONFIG_RV64,      "riscv64",
                               "riscv32"),
                               "bad"))) "-pc-linux-gnu"
  ));
#endif

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
