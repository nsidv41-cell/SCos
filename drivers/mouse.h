// SCos Mouse Driver
// PS/2 mouse support

#ifndef MOUSE_H
#define MOUSE_H

#include "../kernel/types.h"
#include "../kernel/io.h"

// Mouse buttons
#define MOUSE_LEFT      0x01
#define MOUSE_RIGHT     0x02
#define MOUSE_MIDDLE    0x04

// Mouse event structure
struct MouseEvent {
    int16_t delta_x;
    int16_t delta_y;
    int16_t delta_scroll;
    uint8_t buttons;
    bool left_pressed;
    bool right_pressed;
    bool middle_pressed;
};

namespace Mouse {
    // Initialize mouse driver
    void init();
    
    // Mouse handling
    void handle_interrupt();
    bool has_event();
    MouseEvent get_event();
    
    // Position tracking
    int get_x();
    int get_y();
    void set_position(int x, int y);
    void set_bounds(int min_x, int min_y, int max_x, int max_y);
    
    // Button state
    bool is_left_pressed();
    bool is_right_pressed();
    bool is_middle_pressed();
    uint8_t get_buttons();
}

#endif // MOUSE_H
