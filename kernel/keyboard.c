/* ============================================================================
 * SCos 1.3.5 - Keyboard Driver
 * ============================================================================ */

#include "../include/scos.h"

/* Keyboard state */
static volatile char key_buffer[256];
static volatile int buffer_start = 0;
static volatile int buffer_end = 0;
static uint8_t shift_pressed = 0;
static uint8_t ctrl_pressed = 0;
static uint8_t alt_pressed = 0;
static uint8_t caps_lock = 0;

/* US keyboard layout - normal */
static const char scancode_to_ascii[] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0
};

/* US keyboard layout - shifted */
static const char scancode_to_ascii_shift[] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0
};

/* Add character to buffer */
static void buffer_add(char c) {
    int next_end = (buffer_end + 1) % 256;
    if (next_end != buffer_start) {
        key_buffer[buffer_end] = c;
        buffer_end = next_end;
    }
}

/* Keyboard interrupt handler */
void keyboard_handler(registers_t *regs) {
    (void)regs;
    
    uint8_t scancode = inb(0x60);
    
    /* Key release */
    if (scancode & 0x80) {
        scancode &= 0x7F;
        
        switch (scancode) {
            case KEY_LSHIFT:
            case KEY_RSHIFT:
                shift_pressed = 0;
                break;
            case KEY_LCTRL:
                ctrl_pressed = 0;
                break;
            case KEY_LALT:
                alt_pressed = 0;
                break;
        }
        return;
    }
    
    /* Key press */
    switch (scancode) {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            shift_pressed = 1;
            return;
        case KEY_LCTRL:
            ctrl_pressed = 1;
            return;
        case KEY_LALT:
            alt_pressed = 1;
            return;
        case KEY_CAPSLOCK:
            caps_lock = !caps_lock;
            return;
    }
    
    /* Handle special keys */
    if (scancode == KEY_UP) {
        buffer_add('\x1B');  /* ESC */
        buffer_add('[');
        buffer_add('A');
        return;
    }
    if (scancode == KEY_DOWN) {
        buffer_add('\x1B');
        buffer_add('[');
        buffer_add('B');
        return;
    }
    if (scancode == KEY_RIGHT) {
        buffer_add('\x1B');
        buffer_add('[');
        buffer_add('C');
        return;
    }
    if (scancode == KEY_LEFT) {
        buffer_add('\x1B');
        buffer_add('[');
        buffer_add('D');
        return;
    }
    if (scancode == KEY_HOME) {
        buffer_add('\x1B');
        buffer_add('[');
        buffer_add('H');
        return;
    }
    if (scancode == KEY_END) {
        buffer_add('\x1B');
        buffer_add('[');
        buffer_add('F');
        return;
    }
    if (scancode == KEY_DEL) {
        buffer_add('\x7F');
        return;
    }
    if (scancode == KEY_ESC) {
        buffer_add('\x1B');
        return;
    }
    
    /* Convert scancode to ASCII */
    char c = 0;
    if (scancode < sizeof(scancode_to_ascii)) {
        if (shift_pressed) {
            c = scancode_to_ascii_shift[scancode];
        } else {
            c = scancode_to_ascii[scancode];
        }
        
        /* Handle caps lock */
        if (caps_lock && c >= 'a' && c <= 'z') {
            c = shift_pressed ? c : (c - 32);
        } else if (caps_lock && c >= 'A' && c <= 'Z') {
            c = shift_pressed ? c : (c + 32);
        }
        
        /* Handle Ctrl+key combinations */
        if (ctrl_pressed && c >= 'a' && c <= 'z') {
            c = c - 'a' + 1;  /* Ctrl+A = 1, Ctrl+B = 2, etc. */
        }
        
        if (c != 0) {
            buffer_add(c);
        }
    }
}

/* Initialize keyboard */
void keyboard_init(void) {
    buffer_start = 0;
    buffer_end = 0;
    shift_pressed = 0;
    ctrl_pressed = 0;
    alt_pressed = 0;
    caps_lock = 0;
    
    /* Install keyboard handler */
    irq_install_handler(1, keyboard_handler);
    
    /* Clear keyboard buffer */
    while (inb(0x64) & 1) {
        inb(0x60);
    }
}

/* Get character (blocking) */
char keyboard_getchar(void) {
    while (buffer_start == buffer_end) {
        hlt();
    }
    
    char c = key_buffer[buffer_start];
    buffer_start = (buffer_start + 1) % 256;
    return c;
}

/* Get character (non-blocking) */
char keyboard_getchar_nonblock(void) {
    if (buffer_start == buffer_end) {
        return 0;
    }
    
    char c = key_buffer[buffer_start];
    buffer_start = (buffer_start + 1) % 256;
    return c;
}

/* Read line from keyboard */
int keyboard_gets(char *buffer, int max_len) {
    int pos = 0;
    
    while (pos < max_len - 1) {
        char c = keyboard_getchar();
        
        if (c == '\n' || c == '\r') {
            buffer[pos] = '\0';
            return pos;
        } else if (c == '\b') {
            if (pos > 0) {
                pos--;
                vga_putchar('\b');
            }
        } else if (c >= ' ' && c <= '~') {
            buffer[pos++] = c;
            vga_putchar(c);
        }
    }
    
    buffer[pos] = '\0';
    return pos;
}

/* Check if key is pressed */
int keyboard_is_key_pressed(uint8_t scancode) {
    (void)scancode;
    /* This would require tracking key states */
    return 0;
}

/* Set keyboard LEDs */
void keyboard_set_leds(uint8_t leds) {
    while (inb(0x64) & 2);
    outb(0x60, 0xED);
    while (inb(0x64) & 2);
    outb(0x60, leds);
}
