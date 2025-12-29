// SCos Taskbar Implementation

#include "taskbar.h"
#include "../drivers/timer.h"
#include "../kernel/string.h"

Taskbar::Taskbar() {
    x = 0;
    y = GFX_HEIGHT - TASKBAR_HEIGHT;
    width = GFX_WIDTH;
    height = TASKBAR_HEIGHT;
    start_menu_open = false;
}

void Taskbar::init() {
    x = 0;
    y = GFX_HEIGHT - TASKBAR_HEIGHT;
    width = GFX_WIDTH;
    height = TASKBAR_HEIGHT;
    start_menu_open = false;
}

void Taskbar::draw() {
    // Draw taskbar background
    VGA::fill_rect(x, y, width, height, TASKBAR_COLOR);
    
    // Draw top border (neon glow effect)
    VGA::draw_line(x, y, x + width - 1, y, TASKBAR_BORDER);
    VGA::draw_line(x, y + 1, x + width - 1, y + 1, 34);  // Darker line below
    
    // Draw start button
    draw_start_button();
    
    // Draw system tray
    draw_system_tray();
    
    // Draw start menu if open
    if (start_menu_open) {
        draw_start_menu();
    }
}

void Taskbar::draw_start_button() {
    int btn_x = 2;
    int btn_y = y + 2;
    int btn_w = 40;
    int btn_h = height - 4;
    
    // Button background
    VGA::fill_rect(btn_x, btn_y, btn_w, btn_h, 1);
    VGA::draw_rect(btn_x, btn_y, btn_w, btn_h, TASKBAR_BORDER);
    
    // Draw "SC" text (simplified)
    // S
    VGA::draw_line(btn_x + 6, btn_y + 3, btn_x + 12, btn_y + 3, TASKBAR_BORDER);
    VGA::draw_line(btn_x + 6, btn_y + 3, btn_x + 6, btn_y + 7, TASKBAR_BORDER);
    VGA::draw_line(btn_x + 6, btn_y + 7, btn_x + 12, btn_y + 7, TASKBAR_BORDER);
    VGA::draw_line(btn_x + 12, btn_y + 7, btn_x + 12, btn_y + 11, TASKBAR_BORDER);
    VGA::draw_line(btn_x + 6, btn_y + 11, btn_x + 12, btn_y + 11, TASKBAR_BORDER);
    
    // C
    VGA::draw_line(btn_x + 16, btn_y + 3, btn_x + 22, btn_y + 3, TASKBAR_BORDER);
    VGA::draw_line(btn_x + 16, btn_y + 3, btn_x + 16, btn_y + 11, TASKBAR_BORDER);
    VGA::draw_line(btn_x + 16, btn_y + 11, btn_x + 22, btn_y + 11, TASKBAR_BORDER);
    
    // Hexagon icon
    int hex_x = btn_x + 28;
    int hex_y = btn_y + 7;
    VGA::draw_line(hex_x, hex_y - 3, hex_x + 3, hex_y - 5, TASKBAR_BORDER);
    VGA::draw_line(hex_x + 3, hex_y - 5, hex_x + 6, hex_y - 3, TASKBAR_BORDER);
    VGA::draw_line(hex_x + 6, hex_y - 3, hex_x + 6, hex_y + 3, TASKBAR_BORDER);
    VGA::draw_line(hex_x + 6, hex_y + 3, hex_x + 3, hex_y + 5, TASKBAR_BORDER);
    VGA::draw_line(hex_x + 3, hex_y + 5, hex_x, hex_y + 3, TASKBAR_BORDER);
    VGA::draw_line(hex_x, hex_y + 3, hex_x, hex_y - 3, TASKBAR_BORDER);
}

void Taskbar::draw_system_tray() {
    int tray_x = width - 60;
    int tray_y = y + 2;
    int tray_w = 58;
    int tray_h = height - 4;
    
    // Tray background
    VGA::fill_rect(tray_x, tray_y, tray_w, tray_h, 1);
    VGA::draw_rect(tray_x, tray_y, tray_w, tray_h, 34);
    
    // Draw clock
    uint32_t secs = Timer::get_seconds();
    uint32_t mins = (secs / 60) % 60;
    uint32_t hours = (secs / 3600) % 24;
    
    // Simple time display using lines to form digits
    int clock_x = tray_x + 8;
    int clock_y = tray_y + 4;
    
    // Draw hour digit 1
    draw_digit(clock_x, clock_y, hours / 10);
    // Draw hour digit 2
    draw_digit(clock_x + 8, clock_y, hours % 10);
    
    // Colon
    VGA::put_pixel(clock_x + 16, clock_y + 2, TASKBAR_BORDER);
    VGA::put_pixel(clock_x + 16, clock_y + 6, TASKBAR_BORDER);
    
    // Draw minute digit 1
    draw_digit(clock_x + 20, clock_y, mins / 10);
    // Draw minute digit 2
    draw_digit(clock_x + 28, clock_y, mins % 10);
}

void Taskbar::draw_digit(int x, int y, int digit) {
    // 7-segment style digit display
    // Each digit is 5x8 pixels
    
    // Segment definitions: top, top-left, top-right, middle, bottom-left, bottom-right, bottom
    const uint8_t segments[10] = {
        0b1110111,  // 0
        0b0010010,  // 1
        0b1011101,  // 2
        0b1011011,  // 3
        0b0111010,  // 4
        0b1101011,  // 5
        0b1101111,  // 6
        0b1010010,  // 7
        0b1111111,  // 8
        0b1111011   // 9
    };
    
    if (digit < 0 || digit > 9) return;
    
    uint8_t seg = segments[digit];
    
    // Top segment
    if (seg & 0b1000000) {
        VGA::draw_line(x + 1, y, x + 4, y, TASKBAR_BORDER);
    }
    // Top-left segment
    if (seg & 0b0100000) {
        VGA::draw_line(x, y + 1, x, y + 3, TASKBAR_BORDER);
    }
    // Top-right segment
    if (seg & 0b0010000) {
        VGA::draw_line(x + 5, y + 1, x + 5, y + 3, TASKBAR_BORDER);
    }
    // Middle segment
    if (seg & 0b0001000) {
        VGA::draw_line(x + 1, y + 4, x + 4, y + 4, TASKBAR_BORDER);
    }
    // Bottom-left segment
    if (seg & 0b0000100) {
        VGA::draw_line(x, y + 5, x, y + 7, TASKBAR_BORDER);
    }
    // Bottom-right segment
    if (seg & 0b0000010) {
        VGA::draw_line(x + 5, y + 5, x + 5, y + 7, TASKBAR_BORDER);
    }
    // Bottom segment
    if (seg & 0b0000001) {
        VGA::draw_line(x + 1, y + 8, x + 4, y + 8, TASKBAR_BORDER);
    }
}

void Taskbar::draw_start_menu() {
    int menu_x = 2;
    int menu_y = y - 100;
    int menu_w = 80;
    int menu_h = 98;
    
    // Menu background
    VGA::fill_rect(menu_x, menu_y, menu_w, menu_h, 1);
    VGA::draw_rect(menu_x, menu_y, menu_w, menu_h, TASKBAR_BORDER);
    
    // Menu items
    int item_y = menu_y + 5;
    int item_h = 14;
    
    // System Monitor
    VGA::draw_rect(menu_x + 4, item_y, menu_w - 8, item_h, 34);
    draw_menu_icon(menu_x + 8, item_y + 3, 0);  // Monitor icon
    
    item_y += item_h + 4;
    
    // Terminal
    VGA::draw_rect(menu_x + 4, item_y, menu_w - 8, item_h, 34);
    draw_menu_icon(menu_x + 8, item_y + 3, 1);  // Terminal icon
    
    item_y += item_h + 4;
    
    // Settings
    VGA::draw_rect(menu_x + 4, item_y, menu_w - 8, item_h, 34);
    draw_menu_icon(menu_x + 8, item_y + 3, 2);  // Gear icon
    
    item_y += item_h + 4;
    
    // Divider
    VGA::draw_line(menu_x + 4, item_y, menu_x + menu_w - 4, item_y, 34);
    
    item_y += 6;
    
    // Shutdown
    VGA::draw_rect(menu_x + 4, item_y, menu_w - 8, item_h, 34);
    draw_menu_icon(menu_x + 8, item_y + 3, 3);  // Power icon
}

void Taskbar::draw_menu_icon(int x, int y, int icon_type) {
    switch (icon_type) {
        case 0:  // Monitor icon
            VGA::draw_rect(x, y, 8, 6, TASKBAR_BORDER);
            VGA::draw_line(x + 3, y + 6, x + 5, y + 8, TASKBAR_BORDER);
            VGA::draw_line(x + 2, y + 8, x + 6, y + 8, TASKBAR_BORDER);
            break;
            
        case 1:  // Terminal icon
            VGA::draw_rect(x, y, 8, 8, TASKBAR_BORDER);
            VGA::draw_line(x + 2, y + 3, x + 4, y + 5, TASKBAR_BORDER);
            VGA::draw_line(x + 2, y + 5, x + 4, y + 3, TASKBAR_BORDER);
            VGA::draw_line(x + 5, y + 6, x + 6, y + 6, TASKBAR_BORDER);
            break;
            
        case 2:  // Gear icon
            VGA::draw_circle(x + 4, y + 4, 3, TASKBAR_BORDER);
            VGA::put_pixel(x + 4, y, TASKBAR_BORDER);
            VGA::put_pixel(x + 4, y + 8, TASKBAR_BORDER);
            VGA::put_pixel(x, y + 4, TASKBAR_BORDER);
            VGA::put_pixel(x + 8, y + 4, TASKBAR_BORDER);
            break;
            
        case 3:  // Power icon
            VGA::draw_circle(x + 4, y + 4, 3, TASKBAR_BORDER);
            VGA::draw_line(x + 4, y, x + 4, y + 4, TASKBAR_BORDER);
            break;
    }
}

void Taskbar::update() {
    // Taskbar updates (animations, etc.) can go here
}

void Taskbar::handle_click(int mx, int my) {
    (void)my;  // y is implicit since we're in taskbar
    
    // Check start button (first 44 pixels)
    if (mx >= 2 && mx <= 44) {
        toggle_start_menu();
        return;
    }
    
    // Check start menu items if open
    if (start_menu_open) {
        int menu_x = 2;
        int menu_y = y - 100;
        int menu_w = 80;
        int item_h = 14;
        
        if (mx >= menu_x && mx <= menu_x + menu_w) {
            int item_y = menu_y + 5;
            
            // System Monitor
            if (my >= item_y && my < item_y + item_h) {
                // Open system monitor
                start_menu_open = false;
                return;
            }
            
            item_y += item_h + 4;
            
            // Terminal
            if (my >= item_y && my < item_y + item_h) {
                // Open terminal
                start_menu_open = false;
                return;
            }
            
            item_y += item_h + 4;
            
            // Settings
            if (my >= item_y && my < item_y + item_h) {
                // Open settings
                start_menu_open = false;
                return;
            }
            
            item_y += item_h + 10;
            
            // Shutdown
            if (my >= item_y && my < item_y + item_h) {
                // Trigger shutdown
                start_menu_open = false;
                // Desktop::shutdown() would be called here
                return;
            }
        }
    }
    
    // Close menu if clicking elsewhere
    start_menu_open = false;
}

bool Taskbar::contains(int px, int py) {
    // Check taskbar area
    if (py >= y && py < y + height) {
        return true;
    }
    
    // Check start menu area if open
    if (start_menu_open) {
        int menu_x = 2;
        int menu_y = y - 100;
        int menu_w = 80;
        int menu_h = 98;
        
        if (px >= menu_x && px < menu_x + menu_w &&
            py >= menu_y && py < menu_y + menu_h) {
            return true;
        }
    }
    
    return false;
}

void Taskbar::toggle_start_menu() {
    start_menu_open = !start_menu_open;
}
