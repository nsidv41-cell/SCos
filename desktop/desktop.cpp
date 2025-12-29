// SCos Desktop Environment Implementation

#include "desktop.h"
#include "../kernel/string.h"
#include "../kernel/memory.h"
#include "../drivers/timer.h"

// Desktop state
static bool desktop_running = false;
static Window* windows[16];
static int window_count = 0;
static Window* focused_window = nullptr;

// Cursor state
static int cursor_x = 160;
static int cursor_y = 100;

// Cursor bitmap (8x8)
static const uint8_t cursor_bitmap[8] = {
    0b11000000,
    0b11110000,
    0b01111100,
    0b01111111,
    0b00111111,
    0b00011110,
    0b00110110,
    0b01100011
};

// Taskbar instance
static Taskbar taskbar;

// System monitor window
static SystemMonitor* sysmon = nullptr;

namespace Desktop {

void init() {
    // Switch to graphics mode
    VGA::set_graphics_mode();
    
    // Initialize mouse bounds
    Mouse::set_bounds(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
    Mouse::set_position(GFX_WIDTH / 2, GFX_HEIGHT / 2);
    
    cursor_x = Mouse::get_x();
    cursor_y = Mouse::get_y();
    
    // Initialize windows array
    for (int i = 0; i < 16; i++) {
        windows[i] = nullptr;
    }
    window_count = 0;
    focused_window = nullptr;
    
    // Initialize taskbar
    taskbar.init();
    
    // Create system monitor window
    sysmon = new SystemMonitor();
    sysmon->init(50, 30, 120, 80);
    add_window(sysmon);
    
    desktop_running = true;
    
    // Initial draw
    draw();
}

void run() {
    while (desktop_running) {
        // Handle input
        handle_mouse();
        handle_keyboard();
        
        // Update windows
        for (int i = 0; i < window_count; i++) {
            if (windows[i]) {
                windows[i]->update();
            }
        }
        
        // Update taskbar
        taskbar.update();
        
        // Redraw
        draw();
        
        // Small delay to prevent busy loop
        for (volatile int i = 0; i < 10000; i++);
    }
}

void draw() {
    // Draw background
    draw_background();
    
    // Draw windows (back to front)
    for (int i = 0; i < window_count; i++) {
        if (windows[i] && windows[i]->is_visible()) {
            windows[i]->draw();
        }
    }
    
    // Draw taskbar (always on top)
    taskbar.draw();
    
    // Draw clock
    draw_clock();
    
    // Draw cursor last
    draw_cursor();
}

void draw_background() {
    // Fill with black
    VGA::clear_graphics();
    
    // Draw cyberpunk grid pattern
    for (int y = 0; y < GFX_HEIGHT; y += 20) {
        for (int x = 0; x < GFX_WIDTH; x++) {
            VGA::put_pixel(x, y, DESKTOP_GRID);
        }
    }
    
    for (int x = 0; x < GFX_WIDTH; x += 20) {
        for (int y = 0; y < GFX_HEIGHT; y++) {
            VGA::put_pixel(x, y, DESKTOP_GRID);
        }
    }
    
    // Draw "SCos" logo in top-left
    int logo_x = 5;
    int logo_y = 5;
    
    // S
    VGA::draw_line(logo_x, logo_y, logo_x + 6, logo_y, DESKTOP_ACCENT);
    VGA::draw_line(logo_x, logo_y, logo_x, logo_y + 4, DESKTOP_ACCENT);
    VGA::draw_line(logo_x, logo_y + 4, logo_x + 6, logo_y + 4, DESKTOP_ACCENT);
    VGA::draw_line(logo_x + 6, logo_y + 4, logo_x + 6, logo_y + 8, DESKTOP_ACCENT);
    VGA::draw_line(logo_x, logo_y + 8, logo_x + 6, logo_y + 8, DESKTOP_ACCENT);
    
    // C
    logo_x += 10;
    VGA::draw_line(logo_x, logo_y, logo_x + 6, logo_y, DESKTOP_ACCENT);
    VGA::draw_line(logo_x, logo_y, logo_x, logo_y + 8, DESKTOP_ACCENT);
    VGA::draw_line(logo_x, logo_y + 8, logo_x + 6, logo_y + 8, DESKTOP_ACCENT);
    
    // o
    logo_x += 10;
    VGA::draw_rect(logo_x, logo_y, 6, 8, DESKTOP_ACCENT);
    
    // s
    logo_x += 10;
    VGA::draw_line(logo_x, logo_y, logo_x + 5, logo_y, DESKTOP_ACCENT);
    VGA::draw_line(logo_x, logo_y, logo_x, logo_y + 4, DESKTOP_ACCENT);
    VGA::draw_line(logo_x, logo_y + 4, logo_x + 5, logo_y + 4, DESKTOP_ACCENT);
    VGA::draw_line(logo_x + 5, logo_y + 4, logo_x + 5, logo_y + 8, DESKTOP_ACCENT);
    VGA::draw_line(logo_x, logo_y + 8, logo_x + 5, logo_y + 8, DESKTOP_ACCENT);
}

void draw_cursor() {
    cursor_x = Mouse::get_x();
    cursor_y = Mouse::get_y();
    
    // Draw cursor (arrow shape)
    for (int cy = 0; cy < 8; cy++) {
        for (int cx = 0; cx < 8; cx++) {
            if (cursor_bitmap[cy] & (0x80 >> cx)) {
                int px = cursor_x + cx;
                int py = cursor_y + cy;
                if (px >= 0 && px < GFX_WIDTH && py >= 0 && py < GFX_HEIGHT) {
                    VGA::put_pixel(px, py, 47);  // White
                }
            }
        }
    }
    
    // Inner cursor (smaller, green)
    for (int cy = 1; cy < 6; cy++) {
        for (int cx = 1; cx < 4; cx++) {
            if (cursor_bitmap[cy] & (0x80 >> cx)) {
                int px = cursor_x + cx;
                int py = cursor_y + cy;
                if (px >= 0 && px < GFX_WIDTH && py >= 0 && py < GFX_HEIGHT) {
                    VGA::put_pixel(px, py, DESKTOP_ACCENT);
                }
            }
        }
    }
}

void draw_clock() {
    // Draw time in top-right corner
    uint32_t secs = Timer::get_seconds();
    uint32_t mins = secs / 60;
    uint32_t hours = mins / 60;
    
    UNUSED(secs);  // We only display hours:mins
    mins = mins % 60;
    hours = hours % 24;
    
    // Simple digital clock display
    int clock_x = GFX_WIDTH - 45;
    int clock_y = 3;
    
    // Draw background
    VGA::fill_rect(clock_x - 2, clock_y - 1, 44, 10, 1);
    VGA::draw_rect(clock_x - 2, clock_y - 1, 44, 10, DESKTOP_ACCENT);
    
    UNUSED(hours);
    UNUSED(mins);
}

void handle_mouse() {
    // Check for mouse events (polling)
    if (IO::inb(0x64) & 0x21) {
        if (IO::inb(0x64) & 0x20) {
            Mouse::handle_interrupt();
        }
    }
    
    int mx = Mouse::get_x();
    int my = Mouse::get_y();
    bool left = Mouse::is_left_pressed();
    bool right = Mouse::is_right_pressed();
    
    // Check for window interaction
    if (left) {
        // Check taskbar first
        if (taskbar.contains(mx, my)) {
            taskbar.handle_click(mx, my);
            return;
        }
        
        // Check windows (front to back)
        for (int i = window_count - 1; i >= 0; i--) {
            if (windows[i] && windows[i]->contains(mx, my)) {
                focus_window(windows[i]);
                windows[i]->handle_mouse(mx, my, left, right);
                return;
            }
        }
    }
}

void handle_keyboard() {
    if (Keyboard::has_key()) {
        KeyEvent key = Keyboard::get_key();
        
        // Handle global shortcuts
        if (key.modifiers & MOD_CTRL) {
            if (key.ascii == 'q' || key.ascii == 'Q') {
                shutdown();
                return;
            }
        }
        
        // Pass to focused window
        if (focused_window) {
            focused_window->handle_key(key);
        }
    }
}

void add_window(Window* window) {
    if (window_count < 16) {
        windows[window_count++] = window;
        focus_window(window);
    }
}

void remove_window(Window* window) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i] == window) {
            // Shift remaining windows
            for (int j = i; j < window_count - 1; j++) {
                windows[j] = windows[j + 1];
            }
            window_count--;
            windows[window_count] = nullptr;
            
            // Update focus
            if (focused_window == window) {
                focused_window = (window_count > 0) ? windows[window_count - 1] : nullptr;
            }
            return;
        }
    }
}

void focus_window(Window* window) {
    if (window == nullptr || window == focused_window) return;
    
    // Find and move to end of array (top of z-order)
    for (int i = 0; i < window_count; i++) {
        if (windows[i] == window) {
            // Shift windows
            for (int j = i; j < window_count - 1; j++) {
                windows[j] = windows[j + 1];
            }
            windows[window_count - 1] = window;
            break;
        }
    }
    
    if (focused_window) {
        focused_window->set_focused(false);
    }
    
    focused_window = window;
    window->set_focused(true);
}

Window* get_window_at(int x, int y) {
    for (int i = window_count - 1; i >= 0; i--) {
        if (windows[i] && windows[i]->contains(x, y)) {
            return windows[i];
        }
    }
    return nullptr;
}

bool is_running() {
    return desktop_running;
}

void shutdown() {
    desktop_running = false;
    
    // Clean up windows
    for (int i = 0; i < window_count; i++) {
        if (windows[i]) {
            delete windows[i];
            windows[i] = nullptr;
        }
    }
    window_count = 0;
}

} // namespace Desktop
