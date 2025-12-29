// SCos Window System Implementation

#include "window.h"
#include "../kernel/string.h"

#define TITLEBAR_HEIGHT 12
#define BORDER_WIDTH 1

Window::Window() {
    x = 0;
    y = 0;
    width = 100;
    height = 80;
    title = "Window";
    style = WS_DEFAULT;
    visible = true;
    focused = false;
    dragging = false;
    drag_offset_x = 0;
    drag_offset_y = 0;
}

Window::~Window() {
}

void Window::init(int wx, int wy, int ww, int wh, const char* wtitle, uint8_t wstyle) {
    x = wx;
    y = wy;
    width = ww;
    height = wh;
    title = wtitle;
    style = wstyle;
    visible = true;
    focused = false;
}

void Window::draw() {
    if (!visible) return;
    
    draw_frame();
    draw_content();
}

void Window::draw_frame() {
    uint8_t border_color = focused ? WIN_BORDER : WIN_INACTIVE;
    
    // Draw window background
    VGA::fill_rect(x, y, width, height, WIN_BG);
    
    // Draw border
    if (style & WS_BORDER) {
        VGA::draw_rect(x, y, width, height, border_color);
        
        // Draw glow effect for focused window
        if (focused) {
            VGA::draw_rect(x - 1, y - 1, width + 2, height + 2, 34);
        }
    }
    
    // Draw title bar
    if (style & WS_TITLEBAR) {
        VGA::fill_rect(x + 1, y + 1, width - 2, TITLEBAR_HEIGHT, WIN_TITLEBAR);
        VGA::draw_line(x + 1, y + TITLEBAR_HEIGHT, x + width - 2, y + TITLEBAR_HEIGHT, border_color);
        
        // Draw title (simple, character by character)
        // In a full implementation, we'd have a bitmap font
        int title_x = x + 4;
        int title_y = y + 3;
        
        // Draw a simple indicator for the title
        for (int i = 0; i < 3; i++) {
            VGA::put_pixel(title_x + i, title_y + 3, WIN_TITLE_TEXT);
        }
        
        // Draw close button
        if (style & WS_CLOSABLE) {
            int close_x = x + width - 10;
            int close_y = y + 3;
            
            // X symbol
            VGA::draw_line(close_x, close_y, close_x + 6, close_y + 6, WIN_CLOSE_BTN);
            VGA::draw_line(close_x + 6, close_y, close_x, close_y + 6, WIN_CLOSE_BTN);
        }
    }
}

void Window::draw_content() {
    // Override in subclasses to draw window content
    // Default: empty content area
}

void Window::handle_mouse(int mx, int my, bool left, bool right) {
    (void)right;  // Unused for now
    
    if (!left) {
        dragging = false;
        return;
    }
    
    // Check close button
    if ((style & WS_CLOSABLE) && close_button_contains(mx, my)) {
        close();
        return;
    }
    
    // Check title bar for dragging
    if ((style & WS_MOVABLE) && title_bar_contains(mx, my)) {
        if (!dragging) {
            dragging = true;
            drag_offset_x = mx - x;
            drag_offset_y = my - y;
        }
    }
    
    // Handle dragging
    if (dragging) {
        int new_x = mx - drag_offset_x;
        int new_y = my - drag_offset_y;
        
        // Keep window on screen
        if (new_x < 0) new_x = 0;
        if (new_y < 0) new_y = 0;
        if (new_x + width > GFX_WIDTH) new_x = GFX_WIDTH - width;
        if (new_y + height > GFX_HEIGHT - 20) new_y = GFX_HEIGHT - 20 - height;  // Account for taskbar
        
        set_position(new_x, new_y);
    }
}

void Window::handle_key(const KeyEvent& key) {
    // Override in subclasses to handle keyboard input
    (void)key;
}

void Window::update() {
    // Override in subclasses for per-frame updates
}

bool Window::contains(int px, int py) {
    return px >= x && px < x + width && py >= y && py < y + height;
}

bool Window::title_bar_contains(int px, int py) {
    if (!(style & WS_TITLEBAR)) return false;
    return px >= x && px < x + width && py >= y && py < y + TITLEBAR_HEIGHT;
}

bool Window::close_button_contains(int px, int py) {
    if (!(style & WS_CLOSABLE)) return false;
    int close_x = x + width - 12;
    int close_y = y + 2;
    return px >= close_x && px < close_x + 10 && py >= close_y && py < close_y + 10;
}

void Window::set_position(int nx, int ny) {
    x = nx;
    y = ny;
}

void Window::set_size(int nw, int nh) {
    width = nw;
    height = nh;
}

void Window::set_title(const char* new_title) {
    title = new_title;
}

void Window::close() {
    visible = false;
    // Window will be removed by desktop manager
}
