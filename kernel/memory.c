/* ============================================================================
 * SCos 1.3.5 - Memory Manager
 * ============================================================================ */

#include "../include/scos.h"

/* Heap management */
static uint8_t *heap_start;
static uint8_t *heap_end;
static mem_block_t *first_block;

/* Statistics */
static size_t total_allocated = 0;
static size_t total_blocks = 0;

/* Initialize memory manager */
void memory_init(void) {
    heap_start = (uint8_t *)KERNEL_HEAP_START;
    heap_end = heap_start + KERNEL_HEAP_SIZE;
    
    /* Initialize first block as entire heap */
    first_block = (mem_block_t *)heap_start;
    first_block->size = KERNEL_HEAP_SIZE - sizeof(mem_block_t);
    first_block->is_free = 1;
    first_block->next = NULL;
    first_block->prev = NULL;
    first_block->magic = MEM_MAGIC;
    
    total_blocks = 1;
}

/* Find best fit block for allocation */
static mem_block_t *find_best_fit(size_t size) {
    mem_block_t *current = first_block;
    mem_block_t *best = NULL;
    size_t best_size = KERNEL_HEAP_SIZE + 1;
    
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            if (current->size < best_size) {
                best = current;
                best_size = current->size;
            }
        }
        current = current->next;
    }
    
    return best;
}

/* Split a block if it's large enough */
static void split_block(mem_block_t *block, size_t size) {
    size_t min_block_size = sizeof(mem_block_t) + 16;
    
    if (block->size >= size + min_block_size) {
        mem_block_t *new_block = (mem_block_t *)((uint8_t *)block + 
                                                  sizeof(mem_block_t) + size);
        new_block->size = block->size - size - sizeof(mem_block_t);
        new_block->is_free = 1;
        new_block->next = block->next;
        new_block->prev = block;
        new_block->magic = MEM_MAGIC;
        
        if (block->next != NULL) {
            block->next->prev = new_block;
        }
        
        block->next = new_block;
        block->size = size;
        
        total_blocks++;
    }
}

/* Merge adjacent free blocks */
static void merge_blocks(mem_block_t *block) {
    /* Merge with next block if free */
    while (block->next != NULL && block->next->is_free) {
        block->size += sizeof(mem_block_t) + block->next->size;
        block->next = block->next->next;
        if (block->next != NULL) {
            block->next->prev = block;
        }
        total_blocks--;
    }
    
    /* Merge with previous block if free */
    while (block->prev != NULL && block->prev->is_free) {
        block->prev->size += sizeof(mem_block_t) + block->size;
        block->prev->next = block->next;
        if (block->next != NULL) {
            block->next->prev = block->prev;
        }
        block = block->prev;
        total_blocks--;
    }
}

/* Allocate memory */
void *kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    /* Align size to 4 bytes */
    size = (size + 3) & ~3;
    
    mem_block_t *block = find_best_fit(size);
    
    if (block == NULL) {
        /* Out of memory */
        return NULL;
    }
    
    split_block(block, size);
    block->is_free = 0;
    
    total_allocated += block->size;
    
    return (void *)((uint8_t *)block + sizeof(mem_block_t));
}

/* Allocate aligned memory */
void *kmalloc_aligned(size_t size, size_t alignment) {
    /* Simple implementation - allocate extra and align */
    void *ptr = kmalloc(size + alignment);
    if (ptr == NULL) {
        return NULL;
    }
    
    uint32_t addr = (uint32_t)ptr;
    uint32_t aligned = (addr + alignment - 1) & ~(alignment - 1);
    
    return (void *)aligned;
}

/* Free memory */
void kfree(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    
    mem_block_t *block = (mem_block_t *)((uint8_t *)ptr - sizeof(mem_block_t));
    
    /* Validate magic number */
    if (block->magic != MEM_MAGIC) {
        kernel_panic("Memory corruption detected in kfree");
        return;
    }
    
    if (block->is_free) {
        return;  /* Already free */
    }
    
    total_allocated -= block->size;
    block->is_free = 1;
    
    merge_blocks(block);
}

/* Reallocate memory */
void *krealloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return kmalloc(size);
    }
    
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    mem_block_t *block = (mem_block_t *)((uint8_t *)ptr - sizeof(mem_block_t));
    
    if (block->size >= size) {
        return ptr;  /* Current block is big enough */
    }
    
    void *new_ptr = kmalloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }
    
    memcpy(new_ptr, ptr, block->size);
    kfree(ptr);
    
    return new_ptr;
}

/* Get free memory */
size_t memory_get_free(void) {
    size_t free_mem = 0;
    mem_block_t *current = first_block;
    
    while (current != NULL) {
        if (current->is_free) {
            free_mem += current->size;
        }
        current = current->next;
    }
    
    return free_mem;
}

/* Get used memory */
size_t memory_get_used(void) {
    return total_allocated;
}

/* Debug: Dump memory blocks */
void memory_dump(void) {
    mem_block_t *current = first_block;
    int block_num = 0;
    char buf[80];
    
    vga_puts("Memory Blocks:\n");
    vga_puts("==============\n");
    
    while (current != NULL) {
        sprintf(buf, "#%d: addr=0x%x size=%d %s\n",
                block_num,
                (uint32_t)current,
                current->size,
                current->is_free ? "[FREE]" : "[USED]");
        vga_puts(buf);
        
        current = current->next;
        block_num++;
    }
    
    sprintf(buf, "\nTotal: %d blocks, %d bytes used, %d bytes free\n",
            total_blocks, total_allocated, memory_get_free());
    vga_puts(buf);
}
