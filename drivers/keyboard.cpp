// SCos Keyboard Driver Implementation

#include "keyboard.h"
#include "../kernel/memory.h"

// Keyboard state
static uint8_t modifiers = 0;
static bool key_buffer_ready = false;
static KeyEvent key_buffer;

// Scancode to ASCII mapping (US layout)
static const char scancode_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

// Shifted scancode to ASCII mapping
static const char scancode_ascii_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

namespace Keyboard {

void init() {
    // Wait for keyboard controller to be ready
    while (IO::inb(KB_STATUS_PORT) & 0x02);
    
    // Enable keyboard
    IO::outb(KB_COMMAND_PORT, 0xAE);
    
    // Clear any pending data
    while (IO::inb(KB_STATUS_PORT) & 0x01) {
        IO::inb(KB_DATA_PORT);
    }
    
    // Reset keyboard state
    modifiers = 0;
    key_buffer_ready = false;
    
    // Set default LED state
    set_leds(false, true, false);
}

void handle_interrupt() {
    uint8_t scancode = IO::inb(KB_DATA_PORT);
    
    // Check if key released (bit 7 set)
    bool released = (scancode & 0x80) != 0;
    scancode &= 0x7F;  // Clear release bit
    
    KeyEvent event;
    event.scancode = scancode;
    event.pressed = !released;
    event.modifiers = modifiers;
    event.ascii = 0;
    event.is_special = false;
    
    // Handle modifier keys
    switch (scancode) {
        case KEY_LSHIFT:
        case KEY_RSHIFT:
            if (released) modifiers &= ~MOD_SHIFT;
            else modifiers |= MOD_SHIFT;
            return;
            
        case KEY_LCTRL:
            if (released) modifiers &= ~MOD_CTRL;
            else modifiers |= MOD_CTRL;
            return;
            
        case KEY_LALT:
            if (released) modifiers &= ~MOD_ALT;
            else modifiers |= MOD_ALT;
            return;
            
        case KEY_CAPSLOCK:
            if (!released) {
                modifiers ^= MOD_CAPSLOCK;
                set_leds(modifiers & MOD_SCROLLLOCK, 
                        modifiers & MOD_NUMLOCK, 
                        modifiers & MOD_CAPSLOCK);
            }
            return;
            
        case KEY_NUMLOCK:
            if (!released) {
                modifiers ^= MOD_NUMLOCK;
                set_leds(modifiers & MOD_SCROLLLOCK, 
                        modifiers & MOD_NUMLOCK, 
                        modifiers & MOD_CAPSLOCK);
            }
            return;
            
        case KEY_SCROLLLOCK:
            if (!released) {
                modifiers ^= MOD_SCROLLLOCK;
                set_leds(modifiers & MOD_SCROLLLOCK, 
                        modifiers & MOD_NUMLOCK, 
                        modifiers & MOD_CAPSLOCK);
            }
            return;
    }
    
    // Check for special keys
    if (scancode >= KEY_F1 && scancode <= KEY_F12) {
        event.is_special = true;
    } else if (scancode == KEY_UP || scancode == KEY_DOWN ||
               scancode == KEY_LEFT || scancode == KEY_RIGHT ||
               scancode == KEY_HOME || scancode == KEY_END ||
               scancode == KEY_PAGEUP || scancode == KEY_PAGEDOWN ||
               scancode == KEY_INSERT || scancode == KEY_DELETE) {
        event.is_special = true;
    }
    
    // Convert to ASCII if not a special key
    if (!event.is_special && scancode < sizeof(scancode_ascii)) {
        bool use_shift = (modifiers & MOD_SHIFT) != 0;
        bool caps = (modifiers & MOD_CAPSLOCK) != 0;
        
        // Caps lock only affects letters
        char c = scancode_ascii[scancode];
        if (c >= 'a' && c <= 'z') {
            use_shift = use_shift ^ caps;
        }
        
        if (use_shift) {
            event.ascii = scancode_ascii_shift[scancode];
        } else {
            event.ascii = scancode_ascii[scancode];
        }
    }
    
    event.modifiers = modifiers;
    
    // Only store key press events (not releases) in buffer
    if (!released) {
        key_buffer = event;
        key_buffer_ready = true;
    }
}

bool has_key() {
    // Check if there's data in the keyboard buffer
    if (IO::inb(KB_STATUS_PORT) & 0x01) {
        handle_interrupt();
    }
    return key_buffer_ready;
}

KeyEvent get_key() {
    // Wait for a key
    while (!has_key()) {
        asm volatile("hlt");
    }
    
    key_buffer_ready = false;
    return key_buffer;
}

char get_char() {
    KeyEvent event = get_key();
    return event.ascii;
}

bool is_shift_pressed() {
    return (modifiers & MOD_SHIFT) != 0;
}

bool is_ctrl_pressed() {
    return (modifiers & MOD_CTRL) != 0;
}

bool is_alt_pressed() {
    return (modifiers & MOD_ALT) != 0;
}

bool is_caps_lock() {
    return (modifiers & MOD_CAPSLOCK) != 0;
}

uint8_t get_modifiers() {
    return modifiers;
}

void set_leds(bool scroll, bool num, bool caps) {
    uint8_t leds = 0;
    if (scroll) leds |= 0x01;
    if (num) leds |= 0x02;
    if (caps) leds |= 0x04;
    
    // Wait for keyboard controller
    while (IO::inb(KB_STATUS_PORT) & 0x02);
    IO::outb(KB_DATA_PORT, 0xED);
    
    while (IO::inb(KB_STATUS_PORT) & 0x02);
    IO::outb(KB_DATA_PORT, leds);
}

} // namespace Keyboard
