#ifndef __TIMER_H__
#define __TIMER_H__

#include <chrono>
#include <stdint.h>

uint64_t soc_start_time = 0;

uint64_t get_time() {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    return duration.count();
}

void init_rtc() {
    soc_start_time = get_time();
}

#endif