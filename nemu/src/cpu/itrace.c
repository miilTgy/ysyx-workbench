#ifndef __ITRACE_H__
#define __ITRACE_H__

#include <common.h>
#include <utils.h>
#include "../monitor/sdb/sdb.h"

#ifdef CONFIG_ITRACE
typedef struct inode {
    vaddr_t pc;
    uint32_t inst;
    char ilogbuf[128];
} inode;

struct inode iringbuf[CONFIG_ITRACE_BUF_LEN];

int iringbuf_ptr = 0;

void push_iringbuf(vaddr_t pc, uint32_t inst, char *ilogbuf) {
    iringbuf[iringbuf_ptr].pc = pc;
    iringbuf[iringbuf_ptr].inst = inst;
    snprintf(iringbuf[iringbuf_ptr].ilogbuf, sizeof(iringbuf[iringbuf_ptr].ilogbuf), "%s", ilogbuf);
    iringbuf_ptr = (iringbuf_ptr + 1) % CONFIG_ITRACE_BUF_LEN;
}

void display_iringbuf() {
    for (int i = 0; i < CONFIG_ITRACE_BUF_LEN; i++) {
        struct inode node = iringbuf[(i + iringbuf_ptr) % CONFIG_ITRACE_BUF_LEN];
        if (node.pc != 0) {
            printf("INST.%d, pc=0x%lx:\t%s\n", i - CONFIG_ITRACE_BUF_LEN + 1, node.pc, node.ilogbuf);
        }
    }
}

#endif // CONFIG_ITRACE
#endif // __ITRACE_H__