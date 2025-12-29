// SCos Timer Driver Implementation

#include "timer.h"

// Timer state - use two 32-bit values instead of one 64-bit
static uint32_t tick_count_low = 0;
static uint32_t tick_count_high = 0;
static uint32_t timer_frequency = 0;
static uint32_t seconds_counter = 0;
static uint32_t milliseconds_counter = 0;
static uint32_t ticks_per_second = 0;
static uint32_t ticks_per_millisecond = 0;
static uint32_t tick_accumulator = 0;
static uint32_t ms_tick_accumulator = 0;

namespace Timer {

void init(uint32_t frequency) {
    timer_frequency = frequency;
    tick_count_low = 0;
    tick_count_high = 0;
    seconds_counter = 0;
    milliseconds_counter = 0;
    tick_accumulator = 0;
    ms_tick_accumulator = 0;
    
    // Pre-calculate for faster runtime
    ticks_per_second = frequency;
    ticks_per_millisecond = frequency / 1000;
    if (ticks_per_millisecond == 0) ticks_per_millisecond = 1;
    
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
    tick_count_low++;
    
    // Handle overflow
    if (tick_count_low == 0) {
        tick_count_high++;
    }
    
    // Update seconds counter
    tick_accumulator++;
    if (tick_accumulator >= ticks_per_second) {
        tick_accumulator -= ticks_per_second;
        seconds_counter++;
    }
    
    // Update milliseconds counter
    ms_tick_accumulator++;
    if (ms_tick_accumulator >= ticks_per_millisecond) {
        ms_tick_accumulator -= ticks_per_millisecond;
        milliseconds_counter++;
    }
}

uint32_t get_ticks() {
    return tick_count_low;
}

uint32_t get_ticks_high() {
    return tick_count_high;
}

uint32_t get_seconds() {
    return seconds_counter;
}

uint32_t get_milliseconds() {
    return milliseconds_counter;
}

void sleep(uint32_t milliseconds) {
    uint32_t target = milliseconds_counter + milliseconds;
    
    // Handle potential overflow
    if (target < milliseconds_counter) {
        // Wait for overflow first
        while (milliseconds_counter > target) {
            asm volatile("hlt");
        }
    }
    
    while (milliseconds_counter < target) {
        asm volatile("hlt");
    }
}

void sleep_ticks(uint32_t ticks) {
    uint32_t start = tick_count_low;
    uint32_t target = start + ticks;
    
    // Handle potential overflow
    if (target < start) {
        // Wait for overflow first
        while (tick_count_low >= start) {
            asm volatile("hlt");
        }
    }
    
    while (tick_count_low < target) {
        asm volatile("hlt");
    }
}

uint32_t get_frequency() {
    return timer_frequency;
}

} // namespace Timer
