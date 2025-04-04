#ifndef __FTRACE_H__
#define __FTRACE_H__

#include <common.h>
#include <utils.h> 
#include "../monitor/sdb/sdb.h"

typedef struct fnode {
    vaddr_t addr;
    size_t size;
    char fname[64];
    int call_times;
    struct fnode *nextf;
} fnode;

static fnode *ftail = NULL, *fhead = NULL;
static int call_depth = 0;

void fnode_pushback(vaddr_t addr, size_t size, char *fname) {
    fnode *node = malloc(sizeof(fnode));
    node->addr = addr;
    node->size = size;
    node->call_times = 0;
    strcpy(node->fname, fname);
    if (ftail == NULL && fhead == NULL) {
        fhead = node;
    } else {
        ftail->nextf = node;
    }
    ftail = node;
}

void call_or_ret(vaddr_t pc, vaddr_t dnpc, bool ret_possible, int rd) {
    bool is_call = false;
    fnode *node = malloc(sizeof(fnode));
    node = fhead;
    while (node != NULL && is_call == false) {
        if (node->addr == dnpc && rd != 0) {
            is_call = true;
            call_depth++;
            printf("0x%lx: ", pc);
            for (int i=0; i<call_depth; i++) {
                printf("  ");
            }
            node->call_times++;
            printf("CALL [%s@0x%lx]\n", node->fname, dnpc);
            break;
        }
        if (dnpc > node->addr && dnpc < node->addr + node->size && ret_possible == true) {
            if (node->call_times >= 0) {
                call_depth--;
                printf("0x%lx: ", pc);
                for (int i=0; i<call_depth; i++) {
                    printf("  ");
                }
                printf("RET [%s]\n", node->fname);
            }
            break;
        }
        node = node->nextf;
    }
}

void display_all_f() {
    fnode *node = fhead;
    while (node != NULL) {
        printf("FUNC: addr = 0x%lx, size = %ld, end = 0x%lx, name = %s\n", node->addr, node->size, node->addr + node->size, node->fname);
        node = node->nextf;
    }
}

#endif