// SCos VGA Driver
// Text and graphics mode support

#ifndef VGA_H
#define VGA_H

#include "../kernel/types.h"
#include "../kernel/io.h"

// VGA ports
#define VGA_CTRL_REGISTER   0x3D4
#define VGA_DATA_REGISTER   0x3D5
#define VGA_MISC_WRITE      0x3C2
#define VGA_SEQ_INDEX       0x3C4
#define VGA_SEQ_DATA        0x3C5
#define VGA_GC_INDEX        0x3CE
#define VGA_GC_DATA         0x3CF
#define VGA_AC_INDEX        0x3C0
#define VGA_AC_READ         0x3C1
#define VGA_AC_WRITE        0x3C0
#define VGA_INSTAT_READ     0x3DA

// VGA memory addresses
#define VGA_TEXT_BUFFER     0xB8000
#define VGA_GRAPHICS_BUFFER 0xA0000

// Text mode dimensions
#define VGA_WIDTH           80
#define VGA_HEIGHT          25

// Graphics mode dimensions (Mode 13h - 320x200x256)
#define GFX_WIDTH           320
#define GFX_HEIGHT          200

// Color definitions (Cyberpunk theme)
#define COLOR_BLACK         0x00
#define COLOR_DARK_GREEN    0x02
#define COLOR_GREEN         0x0A
#define COLOR_BRIGHT_GREEN  0x0F
#define COLOR_NEON_GREEN    0x0A

// VGA color palette index for graphics mode
#define PAL_BLACK           0
#define PAL_NEON_GREEN      46
#define PAL_DARK_GREEN      2
#define PAL_BRIGHT_WHITE    15

namespace VGA {
    // Initialization
    void init();
    
    // Text mode functions
    void clear_screen();
    void set_color(uint8_t color);
    void set_cursor(int x, int y);
    void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
    void disable_cursor();
    
    // Text output
    void putchar(char c);
    void print(const char* str);
    void print_int(int value);
    void print_hex(uint32_t value);
    
    // Graphics mode functions
    void set_graphics_mode();
    void set_text_mode();
    void put_pixel(int x, int y, uint8_t color);
    uint8_t get_pixel(int x, int y);
    void draw_line(int x1, int y1, int x2, int y2, uint8_t color);
    void draw_rect(int x, int y, int width, int height, uint8_t color);
    void fill_rect(int x, int y, int width, int height, uint8_t color);
    void draw_circle(int cx, int cy, int radius, uint8_t color);
    void fill_circle(int cx, int cy, int radius, uint8_t color);
    
    // Palette functions
    void set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    void init_cyberpunk_palette();
    
    // Buffer operations
    void swap_buffers();
    void clear_graphics();
    
    // State
    bool is_graphics_mode();
    int get_cursor_x();
    int get_cursor_y();
}

#endif // VGA_H
