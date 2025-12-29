// SCos VGA Driver Implementation

#include "vga.h"
#include "../kernel/string.h"
#include "../kernel/memory.h"

// Internal state
static uint16_t* text_buffer = (uint16_t*)VGA_TEXT_BUFFER;
static uint8_t* graphics_buffer = (uint8_t*)VGA_GRAPHICS_BUFFER;
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_color = COLOR_GREEN;
static bool graphics_mode = false;

// Double buffer for graphics (optional, in higher memory)
static uint8_t* back_buffer = nullptr;

namespace VGA {

void init() {
    cursor_x = 0;
    cursor_y = 0;
    current_color = COLOR_NEON_GREEN;
    graphics_mode = false;
    
    // Enable cursor
    enable_cursor(14, 15);
    
    // Clear screen
    clear_screen();
}

void clear_screen() {
    if (graphics_mode) {
        clear_graphics();
    } else {
        uint16_t blank = (current_color << 8) | ' ';
        for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
            text_buffer[i] = blank;
        }
        cursor_x = 0;
        cursor_y = 0;
        set_cursor(0, 0);
    }
}

void set_color(uint8_t color) {
    current_color = color;
}

void set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
    
    uint16_t pos = y * VGA_WIDTH + x;
    
    IO::outb(VGA_CTRL_REGISTER, 14);
    IO::outb(VGA_DATA_REGISTER, (pos >> 8) & 0xFF);
    IO::outb(VGA_CTRL_REGISTER, 15);
    IO::outb(VGA_DATA_REGISTER, pos & 0xFF);
}

void enable_cursor(uint8_t start, uint8_t end) {
    IO::outb(VGA_CTRL_REGISTER, 0x0A);
    IO::outb(VGA_DATA_REGISTER, (IO::inb(VGA_DATA_REGISTER) & 0xC0) | start);
    
    IO::outb(VGA_CTRL_REGISTER, 0x0B);
    IO::outb(VGA_DATA_REGISTER, (IO::inb(VGA_DATA_REGISTER) & 0xE0) | end);
}

void disable_cursor() {
    IO::outb(VGA_CTRL_REGISTER, 0x0A);
    IO::outb(VGA_DATA_REGISTER, 0x20);
}

static void scroll() {
    // Move all lines up by one
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        text_buffer[i] = text_buffer[i + VGA_WIDTH];
    }
    
    // Clear the last line
    uint16_t blank = (current_color << 8) | ' ';
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        text_buffer[i] = blank;
    }
    
    cursor_y = VGA_HEIGHT - 1;
}

void putchar(char c) {
    if (graphics_mode) return;
    
    switch (c) {
        case '\n':
            cursor_x = 0;
            cursor_y++;
            break;
            
        case '\r':
            cursor_x = 0;
            break;
            
        case '\t':
            cursor_x = (cursor_x + 8) & ~7;
            break;
            
        case '\b':
            if (cursor_x > 0) {
                cursor_x--;
                text_buffer[cursor_y * VGA_WIDTH + cursor_x] = (current_color << 8) | ' ';
            }
            break;
            
        default:
            if (c >= ' ') {
                text_buffer[cursor_y * VGA_WIDTH + cursor_x] = (current_color << 8) | c;
                cursor_x++;
            }
            break;
    }
    
    // Handle line wrap
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    // Handle scroll
    if (cursor_y >= VGA_HEIGHT) {
        scroll();
    }
    
    set_cursor(cursor_x, cursor_y);
}

void print(const char* str) {
    while (*str) {
        putchar(*str++);
    }
}

void print_int(int value) {
    char buffer[32];
    String::itoa(value, buffer, 10);
    print(buffer);
}

void print_hex(uint32_t value) {
    char buffer[32];
    print("0x");
    String::utoa(value, buffer, 16);
    print(buffer);
}

// Simple printf implementation
void printf(const char* format, ...) {
    // Note: In bare metal, we'd implement va_args manually
    // For simplicity, this is a basic version
    print(format);
}

// ============================================================================
// Graphics Mode Functions
// ============================================================================

// Mode 13h register values
static const uint8_t mode13h_misc = 0x63;
static const uint8_t mode13h_seq[] = { 0x03, 0x01, 0x0F, 0x00, 0x0E };
static const uint8_t mode13h_crtc[] = {
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
    0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3, 0xFF
};
static const uint8_t mode13h_gc[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF };
static const uint8_t mode13h_ac[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x41, 0x00, 0x0F, 0x00, 0x00
};

void set_graphics_mode() {
    // Write MISC register
    IO::outb(VGA_MISC_WRITE, mode13h_misc);
    
    // Write Sequencer registers
    for (int i = 0; i < 5; i++) {
        IO::outb(VGA_SEQ_INDEX, i);
        IO::outb(VGA_SEQ_DATA, mode13h_seq[i]);
    }
    
    // Unlock CRTC registers
    IO::outb(VGA_CTRL_REGISTER, 0x03);
    IO::outb(VGA_DATA_REGISTER, IO::inb(VGA_DATA_REGISTER) | 0x80);
    IO::outb(VGA_CTRL_REGISTER, 0x11);
    IO::outb(VGA_DATA_REGISTER, IO::inb(VGA_DATA_REGISTER) & ~0x80);
    
    // Write CRTC registers
    for (int i = 0; i < 25; i++) {
        IO::outb(VGA_CTRL_REGISTER, i);
        IO::outb(VGA_DATA_REGISTER, mode13h_crtc[i]);
    }
    
    // Write Graphics Controller registers
    for (int i = 0; i < 9; i++) {
        IO::outb(VGA_GC_INDEX, i);
        IO::outb(VGA_GC_DATA, mode13h_gc[i]);
    }
    
    // Write Attribute Controller registers
    IO::inb(VGA_INSTAT_READ);  // Reset flip-flop
    for (int i = 0; i < 21; i++) {
        IO::outb(VGA_AC_INDEX, i);
        IO::outb(VGA_AC_WRITE, mode13h_ac[i]);
    }
    IO::outb(VGA_AC_INDEX, 0x20);  // Lock palette
    
    graphics_mode = true;
    
    // Initialize cyberpunk palette
    init_cyberpunk_palette();
    
    // Clear the screen
    clear_graphics();
}

void set_text_mode() {
    // Return to 80x25 text mode
    // This would require storing original VGA registers
    // For now, just set the flag
    graphics_mode = false;
    init();
}

void put_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= GFX_WIDTH || y < 0 || y >= GFX_HEIGHT) return;
    graphics_buffer[y * GFX_WIDTH + x] = color;
}

uint8_t get_pixel(int x, int y) {
    if (x < 0 || x >= GFX_WIDTH || y < 0 || y >= GFX_HEIGHT) return 0;
    return graphics_buffer[y * GFX_WIDTH + x];
}

void draw_line(int x1, int y1, int x2, int y2, uint8_t color) {
    // Bresenham's line algorithm
    int dx = x2 - x1;
    int dy = y2 - y1;
    int sx = (dx > 0) ? 1 : -1;
    int sy = (dy > 0) ? 1 : -1;
    
    dx = (dx < 0) ? -dx : dx;
    dy = (dy < 0) ? -dy : dy;
    
    int err = dx - dy;
    
    while (true) {
        put_pixel(x1, y1, color);
        
        if (x1 == x2 && y1 == y2) break;
        
        int e2 = 2 * err;
        
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void draw_rect(int x, int y, int width, int height, uint8_t color) {
    draw_line(x, y, x + width - 1, y, color);
    draw_line(x, y + height - 1, x + width - 1, y + height - 1, color);
    draw_line(x, y, x, y + height - 1, color);
    draw_line(x + width - 1, y, x + width - 1, y + height - 1, color);
}

void fill_rect(int x, int y, int width, int height, uint8_t color) {
    for (int py = y; py < y + height; py++) {
        for (int px = x; px < x + width; px++) {
            put_pixel(px, py, color);
        }
    }
}

void draw_circle(int cx, int cy, int radius, uint8_t color) {
    // Midpoint circle algorithm
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        put_pixel(cx + x, cy + y, color);
        put_pixel(cx + y, cy + x, color);
        put_pixel(cx - y, cy + x, color);
        put_pixel(cx - x, cy + y, color);
        put_pixel(cx - x, cy - y, color);
        put_pixel(cx - y, cy - x, color);
        put_pixel(cx + y, cy - x, color);
        put_pixel(cx + x, cy - y, color);
        
        y++;
        err += 1 + 2 * y;
        
        if (2 * (err - x) + 1 > 0) {
            x--;
            err += 1 - 2 * x;
        }
    }
}

void fill_circle(int cx, int cy, int radius, uint8_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                put_pixel(cx + x, cy + y, color);
            }
        }
    }
}

void set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    // VGA palette uses 6-bit colors (0-63)
    IO::outb(0x3C8, index);
    IO::outb(0x3C9, r >> 2);
    IO::outb(0x3C9, g >> 2);
    IO::outb(0x3C9, b >> 2);
}

void init_cyberpunk_palette() {
    // Color 0: Pure black (background)
    set_palette_color(0, 0, 0, 0);
    
    // Color 1: Dark background
    set_palette_color(1, 10, 10, 10);
    
    // Color 2: Dark green
    set_palette_color(2, 0, 80, 0);
    
    // Colors 3-15: Grayscale ramp
    for (int i = 3; i <= 15; i++) {
        uint8_t v = (i - 3) * 20;
        set_palette_color(i, v, v, v);
    }
    
    // Colors 16-31: Neon green gradient (main theme color)
    for (int i = 0; i < 16; i++) {
        uint8_t g = 100 + (i * 10);  // Green from 100 to 250
        uint8_t r = i * 3;           // Slight red tint
        set_palette_color(16 + i, r, g, 0);
    }
    
    // Color 32: Pure neon green (#39FF14)
    set_palette_color(32, 0x39, 0xFF, 0x14);
    
    // Color 33: Bright neon green
    set_palette_color(33, 0x57, 0xFF, 0x20);
    
    // Color 34: Dark neon green
    set_palette_color(34, 0x20, 0x80, 0x0A);
    
    // Colors 35-45: Cyan accents
    for (int i = 0; i < 11; i++) {
        uint8_t v = i * 25;
        set_palette_color(35 + i, 0, v, v);
    }
    
    // Color 46: Main UI neon green
    set_palette_color(46, 0x39, 0xFF, 0x14);
    
    // Color 47: UI highlight
    set_palette_color(47, 0xFF, 0xFF, 0xFF);
    
    // Color 48: UI shadow
    set_palette_color(48, 0x10, 0x30, 0x08);
    
    // Color 49: Window background
    set_palette_color(49, 0x08, 0x08, 0x08);
    
    // Color 50: Taskbar background
    set_palette_color(50, 0x05, 0x0A, 0x05);
}

void clear_graphics() {
    Memory::memset(graphics_buffer, 0, GFX_WIDTH * GFX_HEIGHT);
}

void swap_buffers() {
    if (back_buffer) {
        Memory::memcpy(graphics_buffer, back_buffer, GFX_WIDTH * GFX_HEIGHT);
    }
}

bool is_graphics_mode() {
    return graphics_mode;
}

int get_cursor_x() {
    return cursor_x;
}

int get_cursor_y() {
    return cursor_y;
}

} // namespace VGA
