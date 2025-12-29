// SCos Desktop Environment
// Cyberpunk-themed graphical desktop

#ifndef DESKTOP_H
#define DESKTOP_H

#include "../kernel/types.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../drivers/mouse.h"
#include "window.h"
#include "taskbar.h"
#include "sysmon.h"

// Desktop colors (palette indices)
#define DESKTOP_BG          0       // Black background
#define DESKTOP_ACCENT      32      // Neon green
#define DESKTOP_HIGHLIGHT   33      // Bright neon green
#define DESKTOP_SHADOW      34      // Dark neon green
#define DESKTOP_TEXT        32      // Text color
#define DESKTOP_GRID        1       // Grid line color

namespace Desktop {
    // Initialize desktop environment
    void init();
    
    // Main loop
    void run();
    
    // Drawing
    void draw();
    void draw_background();
    void draw_cursor();
    void draw_clock();
    
    // Event handling
    void handle_mouse();
    void handle_keyboard();
    
    // Window management
    void add_window(Window* window);
    void remove_window(Window* window);
    void focus_window(Window* window);
    Window* get_window_at(int x, int y);
    
    // State
    bool is_running();
    void shutdown();
}

#endif // DESKTOP_H
