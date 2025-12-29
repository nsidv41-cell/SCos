// SCos Type Definitions
// Core types for bare-metal environment - NO external dependencies

#ifndef TYPES_H
#define TYPES_H

// ============================================================================
// Fixed-width integer types
// ============================================================================

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;

// ============================================================================
// Size types
// ============================================================================

typedef uint32_t    size_t;
typedef int32_t     ssize_t;
typedef int32_t     ptrdiff_t;
typedef uint32_t    uintptr_t;
typedef int32_t     intptr_t;

// ============================================================================
// Boolean type (don't redefine if using C++)
// ============================================================================

#ifndef __cplusplus
typedef uint8_t bool;
#define true  1
#define false 0
#endif

// ============================================================================
// NULL pointer
// ============================================================================

#ifndef NULL
    #ifdef __cplusplus
        #define NULL nullptr
    #else
        #define NULL ((void*)0)
    #endif
#endif

// ============================================================================
// Useful macros
// ============================================================================

#define UNUSED(x)       ((void)(x))
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))
#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#define MAX(a, b)       ((a) > (b) ? (a) : (b))
#define ABS(x)          ((x) < 0 ? -(x) : (x))

// Alignment macros
#define ALIGN_UP(x, align)   (((x) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))
#define IS_ALIGNED(x, align) (((x) & ((align) - 1)) == 0)

#endif // TYPES_H
