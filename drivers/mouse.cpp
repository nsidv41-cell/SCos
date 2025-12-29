// SCos Mouse Driver Implementation

#include "mouse.h"

// Mouse state
static int mouse_x = 160;  // Center of 320px screen
static int mouse_y = 100;  // Center of 200px screen
static int min_x = 0, min_y = 0;
static int max_x = 319, max_y = 199;
static uint8_t mouse_buttons = 0;
static uint8_t mouse_cycle = 0;
static int8_t mouse_bytes[3];
static bool event_ready = false;
static MouseEvent current_event;

// Helper to wait for mouse controller
static void mouse_wait(bool is_signal) {
    uint32_t timeout = 100000;
    if (is_signal) {
        while (timeout--) {
            if (IO::inb(0x64) & 0x01) return;
        }
    } else {
        while (timeout--) {
            if (!(IO::inb(0x64) & 0x02)) return;
        }
    }
}

static void mouse_write(uint8_t data) {
    mouse_wait(false);
    IO::outb(0x64, 0xD4);
    mouse_wait(false);
    IO::outb(0x60, data);
}

static uint8_t mouse_read() {
    mouse_wait(true);
    return IO::inb(0x60);
}

namespace Mouse {

void init() {
    // Enable auxiliary device (mouse)
    mouse_wait(false);
    IO::outb(0x64, 0xA8);
    
    // Enable interrupts
    mouse_wait(false);
    IO::outb(0x64, 0x20);
    mouse_wait(true);
    uint8_t status = IO::inb(0x60) | 0x02;
    mouse_wait(false);
    IO::outb(0x64, 0x60);
    mouse_wait(false);
    IO::outb(0x60, status);
    
    // Use default settings
    mouse_write(0xF6);
    mouse_read();  // Acknowledge
    
    // Enable mouse
    mouse_write(0xF4);
    mouse_read();  // Acknowledge
    
    // Reset state
    mouse_x = 160;
    mouse_y = 100;
    mouse_buttons = 0;
    mouse_cycle = 0;
    event_ready = false;
}

void handle_interrupt() {
    uint8_t status = IO::inb(0x64);
    
    // Check if data is from mouse (bit 5 set)
    if (!(status & 0x20)) return;
    
    int8_t data = IO::inb(0x60);
    
    switch (mouse_cycle) {
        case 0:
            mouse_bytes[0] = data;
            // Validate packet (bit 3 should always be set)
            if (data & 0x08) {
                mouse_cycle++;
            }
            break;
            
        case 1:
            mouse_bytes[1] = data;
            mouse_cycle++;
            break;
            
        case 2:
            mouse_bytes[2] = data;
            mouse_cycle = 0;
            
            // Process complete packet
            uint8_t flags = mouse_bytes[0];
            int16_t dx = mouse_bytes[1];
            int16_t dy = mouse_bytes[2];
            
            // Handle sign extension
            if (flags & 0x10) dx |= 0xFF00;
            if (flags & 0x20) dy |= 0xFF00;
            
            // Check for overflow
            if (flags & 0x40) dx = 0;
            if (flags & 0x80) dy = 0;
            
            // Update position (note: Y is inverted)
            mouse_x += dx;
            mouse_y -= dy;
            
            // Clamp to bounds
            if (mouse_x < min_x) mouse_x = min_x;
            if (mouse_x > max_x) mouse_x = max_x;
            if (mouse_y < min_y) mouse_y = min_y;
            if (mouse_y > max_y) mouse_y = max_y;
            
            // Update buttons
            mouse_buttons = flags & 0x07;
            
            // Create event
            current_event.delta_x = dx;
            current_event.delta_y = -dy;
            current_event.delta_scroll = 0;
            current_event.buttons = mouse_buttons;
            current_event.left_pressed = (mouse_buttons & MOUSE_LEFT) != 0;
            current_event.right_pressed = (mouse_buttons & MOUSE_RIGHT) != 0;
            current_event.middle_pressed = (mouse_buttons & MOUSE_MIDDLE) != 0;
            
            event_ready = true;
            break;
    }
}

bool has_event() {
    // Check if there's mouse data
    if (IO::inb(0x64) & 0x21) {
        if (IO::inb(0x64) & 0x20) {
            handle_interrupt();
        }
    }
    return event_ready;
}

MouseEvent get_event() {
    while (!has_event()) {
        asm volatile("hlt");
    }
    event_ready = false;
    return current_event;
}

int get_x() {
    return mouse_x;
}

int get_y() {
    return mouse_y;
}

void set_position(int x, int y) {
    mouse_x = x;
    mouse_y = y;
    
    // Clamp to bounds
    if (mouse_x < min_x) mouse_x = min_x;
    if (mouse_x > max_x) mouse_x = max_x;
    if (mouse_y < min_y) mouse_y = min_y;
    if (mouse_y > max_y) mouse_y = max_y;
}

void set_bounds(int x1, int y1, int x2, int y2) {
    min_x = x1;
    min_y = y1;
    max_x = x2;
    max_y = y2;
}

bool is_left_pressed() {
    return (mouse_buttons & MOUSE_LEFT) != 0;
}

bool is_right_pressed() {
    return (mouse_buttons & MOUSE_RIGHT) != 0;
}

bool is_middle_pressed() {
    return (mouse_buttons & MOUSE_MIDDLE) != 0;
}

uint8_t get_buttons() {
    return mouse_buttons;
}

} // namespace Mouse
