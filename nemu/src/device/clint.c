/***************************************************************************************
* CLINT (Core Local Interruptor) implementation for RISC-V
* Handles timer and software interrupts
***************************************************************************************/

#include "difftest-def.h"
#include <device/map.h>
#include <stdint.h>
#include <utils.h>
#include <isa.h>

#define CLINT_MSIP      0x0000      // Machine Software Interrupt Pending
#define CLINT_MTIMECMP  0x4000      // Machine Timer Compare  
#define CLINT_MTIME     0xBFF8      // Machine Time Register

static uint8_t *clint_base = NULL;
static uint64_t mtime = 0;          // Current time value
static uint64_t mtimecmp = UINT64_MAX;       // Timer compare value
static bool timer_intr_pending = false;

// Update mtime
// static void update_mtime() {
//     mtime += 1;
// }

// Get current time in microseconds
static uint64_t get_current_time() {
    static uint64_t base_time = 0;
    if (base_time == 0) {
        base_time = get_time();
    }
    return get_time() - base_time;
}

// Update MTIME and check for timer interrupts
static void clint_update_timer() {
    mtime = get_current_time();
    // printf("CLINT: Updating time (mtime=%lu)\n", mtime);
    
    // Update MTIME register
    if (clint_base) {
        *(uint64_t *)(clint_base + CLINT_MTIME) = mtime;
    }
    
    // Check if timer interrupt should be triggered
    if (mtime >= mtimecmp && mtimecmp != 0) {
        timer_intr_pending = true;
        // printf("CLINT: Timer interrupt pending (mtime=%lu, mtimecmp=%lu)\n", mtime, mtimecmp);
    } else {
        timer_intr_pending = false;
    }
}

// CLINT I/O handler
static void clint_io_handler(uint32_t offset, int len, bool is_write) {
    // printf("CLINT callback at pc = %016lx\n", cpu.pc);
    clint_update_timer();
    
    switch (offset) {
        case CLINT_MSIP:
            if (is_write) {
                // Software interrupt - set MSIP bit
                uint32_t value = *(uint32_t *)(clint_base + CLINT_MSIP);
                if (value & 0x1) {
                    printf("CLINT: Software interrupt triggered\n");
                    // In a real system, this would set the MSIP bit in mip CSR
                }
            }
            break;
            
        case CLINT_MTIMECMP:
            if (is_write) {
                // Update timer compare value
                if (len == 8) {
                    mtimecmp = *(uint64_t *)(clint_base + CLINT_MTIMECMP);
                    // printf("CLINT: MTIMECMP set to 0x%016lx\n", mtimecmp);
                    timer_intr_pending = false; // Clear pending interrupt when new value is set
                } else if (len == 4) {
                    // Handle 32-bit write (update lower 32 bits)
                    uint32_t value = *(uint32_t *)(clint_base + CLINT_MTIMECMP);
                    mtimecmp = (mtimecmp & 0xFFFFFFFF00000000) | value;
                    timer_intr_pending = false;
                }
            } else {
                // Read MTIMECMP
                if (len == 8) {
                    *(uint64_t *)(clint_base + CLINT_MTIMECMP) = mtimecmp;
                } else if (len == 4) {
                    // Handle 32-bit read
                    *(uint32_t *)(clint_base + CLINT_MTIMECMP) = mtimecmp & 0xFFFFFFFF;
                }
            }
            break;
            
        case CLINT_MTIME:
            if (!is_write) {
                // printf("read mtime = %016lx\n", mtime);
                // Read MTIME - return current time
                if (len == 8) {
                    *(uint64_t *)(clint_base + CLINT_MTIME) = mtime;
                } else if (len == 4) {
                    *(uint32_t *)(clint_base + CLINT_MTIME) = mtime & 0xFFFFFFFF;
                }
            } else {
                // MTIME is read-only in standard CLINT, but we'll allow writes for testing
                if (len == 8) {
                    mtime = *(uint64_t *)(clint_base + CLINT_MTIME);
                }
            }
            break;
            
        default:
            // Handle other offsets if needed
            break;
    }
}

// Check if timer interrupt is pending (called from isa_query_intr)
bool clint_timer_intr_pending() {
    clint_update_timer();
    return timer_intr_pending;
}

// Check if software interrupt is pending
bool clint_software_intr_pending() {
    if (clint_base) {
        uint32_t msip = *(uint32_t *)(clint_base + CLINT_MSIP);
        return (msip & 0x1) != 0;
    }
    return false;
}

// Initialize CLINT
void init_clint() {
    clint_base = new_space(0x10000); // 64KB space for CLINT
    
    // Initialize registers
    mtime = 0;
    mtimecmp = 0;
    timer_intr_pending = false;
    
    // Clear MSIP
    if (clint_base) {
        *(uint32_t *)(clint_base + CLINT_MSIP) = 0;
    }
    
    // Map CLINT to memory
    add_mmio_map("clint", CONFIG_CLINT_MMIO, clint_base, 0x10000, clint_io_handler);
    
    printf("CLINT initialized at 0x2000000\n");
}