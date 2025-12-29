// SCos Kernel Header
// Core kernel definitions and declarations

#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"
#include "io.h"
#include "string.h"
#include "memory.h"

// ============================================================================
// SCos color scheme (VGA text mode attributes)
// ============================================================================

#define SCOS_NEON_GREEN    0x0A    // Light green on black
#define SCOS_DARK_GREEN    0x02    // Dark green on black
#define SCOS_BRIGHT_GREEN  0x0F    // Bright white (for highlights)

// ============================================================================
// Kernel functions
// ============================================================================

extern "C" void kernel_main(uint32_t magic, uint32_t* multiboot_info);
void kernel_init();
void kernel_panic(const char* message);

#endif // KERNEL_H
