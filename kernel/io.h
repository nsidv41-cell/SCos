// SCos I/O Operations
// Low-level port I/O functions

#ifndef IO_H
#define IO_H

#include "kernel.h"

namespace IO {
    // Output a byte to a port
    static inline void outb(uint16_t port, uint8_t value) {
        asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
    }
    
    // Output a word to a port
    static inline void outw(uint16_t port, uint16_t value) {
        asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
    }
    
    // Output a double word to a port
    static inline void outl(uint16_t port, uint32_t value) {
        asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
    }
    
    // Input a byte from a port
    static inline uint8_t inb(uint16_t port) {
        uint8_t value;
        asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
        return value;
    }
    
    // Input a word from a port
    static inline uint16_t inw(uint16_t port) {
        uint16_t value;
        asm volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
        return value;
    }
    
    // Input a double word from a port
    static inline uint32_t inl(uint16_t port) {
        uint32_t value;
        asm volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
        return value;
    }
    
    // I/O wait (small delay)
    static inline void wait() {
        outb(0x80, 0);
    }
}

#endif // IO_H
