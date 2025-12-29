// SCos Memory Management Implementation

#include "memory.h"
#include "string.h"

// Heap configuration
#define HEAP_START  0x200000    // 2MB - Start of heap
#define HEAP_SIZE   0x400000    // 4MB - Heap size
#define HEAP_END    (HEAP_START + HEAP_SIZE)

// Alignment
#define ALIGN_SIZE  8
#define ALIGN(size) (((size) + (ALIGN_SIZE - 1)) & ~(ALIGN_SIZE - 1))

// Global memory state
static MemoryBlock* heap_start = nullptr;
static size_t total_memory = HEAP_SIZE;
static size_t used_memory = 0;

namespace Memory {

void init() {
    // Initialize the first memory block as one large free block
    heap_start = (MemoryBlock*)HEAP_START;
    heap_start->size = HEAP_SIZE - sizeof(MemoryBlock);
    heap_start->used = false;
    heap_start->next = nullptr;
    heap_start->prev = nullptr;
    
    used_memory = sizeof(MemoryBlock);
}

void* malloc(size_t size) {
    if (size == 0) return nullptr;
    
    // Align the size
    size = ALIGN(size);
    
    // Find a free block (first-fit algorithm)
    MemoryBlock* current = heap_start;
    
    while (current != nullptr) {
        if (!current->used && current->size >= size) {
            // Found a suitable block
            
            // Check if we should split the block
            if (current->size >= size + sizeof(MemoryBlock) + ALIGN_SIZE) {
                // Split the block
                MemoryBlock* new_block = (MemoryBlock*)((uint8_t*)current + sizeof(MemoryBlock) + size);
                new_block->size = current->size - size - sizeof(MemoryBlock);
                new_block->used = false;
                new_block->next = current->next;
                new_block->prev = current;
                
                if (current->next) {
                    current->next->prev = new_block;
                }
                
                current->next = new_block;
                current->size = size;
                
                used_memory += sizeof(MemoryBlock);
            }
            
            current->used = true;
            used_memory += current->size;
            
            // Return pointer to usable memory (after the header)
            return (void*)((uint8_t*)current + sizeof(MemoryBlock));
        }
        
        current = current->next;
    }
    
    // No suitable block found
    return nullptr;
}

void* calloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = malloc(total);
    
    if (ptr) {
        memset(ptr, 0, total);
    }
    
    return ptr;
}

void* realloc(void* ptr, size_t size) {
    if (ptr == nullptr) {
        return malloc(size);
    }
    
    if (size == 0) {
        free(ptr);
        return nullptr;
    }
    
    // Get the block header
    MemoryBlock* block = (MemoryBlock*)((uint8_t*)ptr - sizeof(MemoryBlock));
    
    // If the current block is large enough, just return it
    if (block->size >= size) {
        return ptr;
    }
    
    // Allocate a new block
    void* new_ptr = malloc(size);
    
    if (new_ptr) {
        // Copy old data
        memcpy(new_ptr, ptr, block->size);
        free(ptr);
    }
    
    return new_ptr;
}

void free(void* ptr) {
    if (ptr == nullptr) return;
    
    // Get the block header
    MemoryBlock* block = (MemoryBlock*)((uint8_t*)ptr - sizeof(MemoryBlock));
    
    if (!block->used) return;  // Already free
    
    block->used = false;
    used_memory -= block->size;
    
    // Coalesce with next block if it's free
    if (block->next && !block->next->used) {
        block->size += sizeof(MemoryBlock) + block->next->size;
        block->next = block->next->next;
        
        if (block->next) {
            block->next->prev = block;
        }
        
        used_memory -= sizeof(MemoryBlock);
    }
    
    // Coalesce with previous block if it's free
    if (block->prev && !block->prev->used) {
        block->prev->size += sizeof(MemoryBlock) + block->size;
        block->prev->next = block->next;
        
        if (block->next) {
            block->next->prev = block->prev;
        }
        
        used_memory -= sizeof(MemoryBlock);
    }
}

void* memset(void* dest, int value, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    uint8_t v = (uint8_t)value;
    
    // Optimize for aligned memory
    while (count >= 4 && ((size_t)d & 3)) {
        *d++ = v;
        count--;
    }
    
    // Set 4 bytes at a time
    if (count >= 4) {
        uint32_t v32 = v | (v << 8) | (v << 16) | (v << 24);
        uint32_t* d32 = (uint32_t*)d;
        
        while (count >= 4) {
            *d32++ = v32;
            count -= 4;
        }
        
        d = (uint8_t*)d32;
    }
    
    // Set remaining bytes
    while (count--) {
        *d++ = v;
    }
    
    return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    
    // Check for overlap (use memmove instead)
    if (d == s || count == 0) return dest;
    
    // Optimize for aligned memory
    while (count >= 4 && (((size_t)d | (size_t)s) & 3)) {
        *d++ = *s++;
        count--;
    }
    
    // Copy 4 bytes at a time
    if (count >= 4) {
        uint32_t* d32 = (uint32_t*)d;
        const uint32_t* s32 = (const uint32_t*)s;
        
        while (count >= 4) {
            *d32++ = *s32++;
            count -= 4;
        }
        
        d = (uint8_t*)d32;
        s = (const uint8_t*)s32;
    }
    
    // Copy remaining bytes
    while (count--) {
        *d++ = *s++;
    }
    
    return dest;
}

void* memmove(void* dest, const void* src, size_t count) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    
    if (d == s || count == 0) return dest;
    
    // Check for overlap and copy accordingly
    if (d < s || d >= s + count) {
        // No overlap or dest before src, copy forward
        return memcpy(dest, src, count);
    } else {
        // Overlap with dest after src, copy backward
        d += count;
        s += count;
        
        while (count--) {
            *--d = *--s;
        }
    }
    
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t count) {
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;
    
    while (count--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    
    return 0;
}

size_t get_total_memory() {
    return total_memory;
}

size_t get_used_memory() {
    return used_memory;
}

size_t get_free_memory() {
    return total_memory - used_memory;
}

} // namespace Memory

// Global new/delete operators
void* operator new(size_t size) {
    return Memory::malloc(size);
}

void* operator new[](size_t size) {
    return Memory::malloc(size);
}

void operator delete(void* ptr) {
    Memory::free(ptr);
}

void operator delete[](void* ptr) {
    Memory::free(ptr);
}

void operator delete(void* ptr, size_t) {
    Memory::free(ptr);
}

void operator delete[](void* ptr, size_t) {
    Memory::free(ptr);
}
