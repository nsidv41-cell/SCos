/* ============================================================================
 * SCos 1.3.5 - Timer Driver (PIT - Programmable Interval Timer)
 * ============================================================================ */

#include "../include/scos.h"

#define PIT_CHANNEL0    0x40
#define PIT_CHANNEL1    0x41
#define PIT_CHANNEL2    0x42
#define PIT_COMMAND     0x43
#define PIT_FREQUENCY   1193182

static volatile uint32_t tick_count = 0;
static uint32_t timer_frequency = 0;

/* Timer interrupt handler */
void timer_handler(registers_t *regs) {
    (void)regs;
    tick_count++;
    
    /* Update process CPU time */
    process_t *proc = process_get_current();
    if (proc) {
        proc->cpu_time++;
    }
}

/* Initialize timer */
void timer_init(uint32_t frequency) {
    timer_frequency = frequency;
    
    /* Calculate divisor */
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    /* Set PIT to repeating mode */
    outb(PIT_COMMAND, 0x36);
    
    /* Set divisor */
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
    
    /* Install timer handler */
    irq_install_handler(0, timer_handler);
    
    tick_count = 0;
}

/* Get tick count */
uint32_t timer_get_ticks(void) {
    return tick_count;
}

/* Sleep for specified milliseconds */
void timer_sleep(uint32_t ms) {
    uint32_t target = tick_count + (ms * timer_frequency / 1000);
    
    while (tick_count < target) {
        hlt();
    }
}

/* Get uptime in seconds */
uint32_t timer_get_uptime(void) {
    return tick_count / timer_frequency;
}
