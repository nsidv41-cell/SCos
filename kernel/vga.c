/* ============================================================================
 * SCos 1.3.5 - VGA Text Mode Driver
 * ============================================================================ */

#include "../include/scos.h"

/* VGA state */
static uint16_t *vga_buffer = (uint16_t *)VGA_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_attr = SCOS_ATTR;

/* Update hardware cursor position */
static void update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

/* Initialize VGA */
void vga_init(void) {
    cursor_x = 0;
    cursor_y = 0;
    current_attr = SCOS_ATTR;
    vga_enable_cursor();
    vga_clear();
}

/* Clear screen */
void vga_clear(void) {
    uint16_t blank = (current_attr << 8) | ' ';
    
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = blank;
    }
    
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

/* Scroll screen up one line */
void vga_scroll(void) {
    if (cursor_y >= VGA_HEIGHT) {
        /* Move everything up one line */
        for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
            vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
        }
        
        /* Clear the last line */
        uint16_t blank = (current_attr << 8) | ' ';
        for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
            vga_buffer[i] = blank;
        }
        
        cursor_y = VGA_HEIGHT - 1;
    }
}

/* Put character at current position */
void vga_putchar(char c) {
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
            if (cursor_x >= VGA_WIDTH) {
                cursor_x = 0;
                cursor_y++;
            }
            break;
            
        case '\b':
            if (cursor_x > 0) {
                cursor_x--;
                vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = 
                    (current_attr << 8) | ' ';
            }
            break;
            
        default:
            if (c >= ' ') {
                vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = 
                    (current_attr << 8) | c;
                cursor_x++;
                
                if (cursor_x >= VGA_WIDTH) {
                    cursor_x = 0;
                    cursor_y++;
                }
            }
            break;
    }
    
    vga_scroll();
    update_cursor();
}

/* Put character at specific position */
void vga_putchar_at(char c, int x, int y, uint8_t attr) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        vga_buffer[y * VGA_WIDTH + x] = (attr << 8) | c;
    }
}

/* Print string */
void vga_puts(const char *str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

/* Print string with specific color */
void vga_put_color(const char *str, uint8_t color) {
    uint8_t old_attr = current_attr;
    current_attr = color;
    vga_puts(str);
    current_attr = old_attr;
}

/* Set cursor position */
void vga_set_cursor(int x, int y) {
    if (x >= 0 && x < VGA_WIDTH) {
        cursor_x = x;
    }
    if (y >= 0 && y < VGA_HEIGHT) {
        cursor_y = y;
    }
    update_cursor();
}

/* Get cursor position */
void vga_get_cursor(int *x, int *y) {
    if (x) *x = cursor_x;
    if (y) *y = cursor_y;
}

/* Set text attribute */
void vga_set_attr(uint8_t attr) {
    current_attr = attr;
}

/* Get text attribute */
uint8_t vga_get_attr(void) {
    return current_attr;
}

/* Enable cursor */
void vga_enable_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | 13);  /* Start line */
    
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | 14);  /* End line */
}

/* Disable cursor */
void vga_disable_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

/* Clear a single line */
void vga_clear_line(int line) {
    if (line >= 0 && line < VGA_HEIGHT) {
        uint16_t blank = (current_attr << 8) | ' ';
        for (int i = 0; i < VGA_WIDTH; i++) {
            vga_buffer[line * VGA_WIDTH + i] = blank;
        }
    }
}
