// SCos Timer Driver
// Programmable Interval Timer (PIT) support

#ifndef TIMER_H
#define TIMER_H

#include "../kernel/types.h"
#include "../kernel/io.h"

// PIT ports
#define PIT_CHANNEL0    0x40
#define PIT_CHANNEL1    0x41
#define PIT_CHANNEL2    0x42
#define PIT_COMMAND     0x43

// PIT frequency
#define PIT_FREQUENCY   1193182

namespace Timer {
    // Initialize timer with given frequency (Hz)
    void init(uint32_t frequency);
    
    // Timer tick handling
    void handle_interrupt();
    
    // Time functions - using 32-bit to avoid division issues
    uint32_t get_ticks();
    uint32_t get_ticks_high();  // Upper 32 bits if needed
    uint32_t get_seconds();
    uint32_t get_milliseconds();
    
    // Delay functions
    void sleep(uint32_t milliseconds);
    void sleep_ticks(uint32_t ticks);
    
    // Timer frequency
    uint32_t get_frequency();
}

#endif // TIMER_H
