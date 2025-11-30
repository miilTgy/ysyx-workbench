#ifndef __DEVICE_CLINT_H__
#define __DEVICE_CLINT_H__

#include <stdbool.h>

void init_clint();
bool clint_timer_intr_pending();
bool clint_software_intr_pending();

#endif