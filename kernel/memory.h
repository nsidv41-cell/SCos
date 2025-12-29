// SCos Memory Management
// Custom memory allocation and manipulation functions

#ifndef MEMORY_H
#define MEMORY_H

#include "kernel.h"

// Memory block header for allocation tracking
struct MemoryBlock {
    size_t size;
    bool used;
    MemoryBlock* next;
    MemoryBlock* prev;
};

namespace Memory {
    // Initialize memory manager
    void init();
    
    // Memory allocation
    void* malloc(size_t size);
    void* calloc(size_t num, size_t size);
    void* realloc(void* ptr, size_t size);
    void free(void* ptr);
    
    // Memory manipulation
    void* memset(void* dest, int value, size_t count);
    void* memcpy(void* dest, const void* src, size_t count);
    void* memmove(void* dest, const void* src, size_t count);
    int memcmp(const void* s1, const void* s2, size_t count);
    
    // Memory info
    size_t get_total_memory();
    size_t get_used_memory();
    size_t get_free_memory();
}

// Global new/delete operators
void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* ptr);
void operator delete[](void* ptr);
void operator delete(void* ptr, size_t size);
void operator delete[](void* ptr, size_t size);

#endif // MEMORY_H
