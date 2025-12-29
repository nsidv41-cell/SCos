// SCos Timer Driver Implementation

#include "timer.h"

// Timer state
static uint64_t tick_count = 0;
static uint32_t timer_frequency = 0;

namespace Timer {

void init(uint32_t frequency) {
    timer_frequency = frequency;
    tick_count = 0;
    
    // Calculate divisor
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    // Ensure divisor fits in 16 bits
    if (divisor > 65535) divisor = 65535;
    if (divisor < 1) divisor = 1;
    
    // Send command byte: Channel 0, Access mode lobyte/hibyte, Mode 3 (square wave)
    IO::outb(PIT_COMMAND, 0x36);
    
    // Send divisor
    IO::outb(PIT_CHANNEL0, divisor & 0xFF);
    IO::outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

void handle_interrupt() {
    tick_count++;
}

uint64_t get_ticks() {
    return tick_count;
}

uint32_t get_seconds() {
    if (timer_frequency == 0) return 0;
    return (uint32_t)(tick_count / timer_frequency);
}

uint32_t get_milliseconds() {
    if (timer_frequency == 0) return 0;
    return (uint32_t)((tick_count * 1000) / timer_frequency);
}

void sleep(uint32_t milliseconds) {
    uint64_t target = tick_count + (milliseconds * timer_frequency) / 1000;
    while (tick_count < target) {
        asm volatile("hlt");
    }
}

void sleep_ticks(uint32_t ticks) {
    uint64_t target = tick_count + ticks;
    while (tick_count < target) {
        asm volatile("hlt");
    }
}

uint32_t get_frequency() {
    return timer_frequency;
}

} // namespace Timer
