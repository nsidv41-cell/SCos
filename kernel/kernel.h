// SCos Kernel Header
// Core kernel definitions and declarations

#ifndef KERNEL_H
#define KERNEL_H

#include "memory.h"
#include "string.h"
#include "io.h"

// Multiboot header for GRUB compatibility
#define MULTIBOOT_MAGIC 0x1BADB002
#define MULTIBOOT_FLAGS 0x00000003
#define MULTIBOOT_CHECKSUM -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

// Standard type definitions
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef uint32_t size_t;
typedef int32_t ssize_t;

// Boolean type
typedef uint8_t bool;
#define true 1
#define false 0

// NULL pointer
#define NULL ((void*)0)

// SCos color scheme
#define SCOS_NEON_GREEN    0x0A    // Light green on black
#define SCOS_DARK_GREEN    0x02    // Dark green on black
#define SCOS_BRIGHT_GREEN  0x0F    // Bright white (will use for highlights)

// Kernel functions
extern "C" void kernel_main();
void kernel_init();
void kernel_panic(const char* message);

#endif // KERNEL_H
