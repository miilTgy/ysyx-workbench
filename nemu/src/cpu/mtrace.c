#ifndef MTRACE_H
#define MTRACE_H

#include <common.h>
#include <utils.h>
#include "../monitor/sdb/sdb.h"

#ifdef CONFIG_MTRACE

/* mtrace is in:
 * - src/memory/paddr.c/paddr_read()
 * - src/memory/vaddr.c/paddr_write()
 */

#endif // CONFIG_MTRACE

#endif // MTRACE_H