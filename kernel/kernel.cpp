// SCos Kernel Main
// Core kernel implementation

#include "kernel.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "../drivers/mouse.h"
#include "../desktop/desktop.h"

// Kernel entry point - called from kernel_entry.asm
extern "C" void kernel_main(uint32_t magic, uint32_t* multiboot_info) {
    // Suppress unused parameter warnings
    UNUSED(magic);
    UNUSED(multiboot_info);
    
    // Initialize kernel subsystems
    kernel_init();
    
    // Start the desktop environment
    Desktop::init();
    Desktop::run();
    
    // Should never reach here
    kernel_panic("Desktop environment exited unexpectedly");
}

void kernel_init() {
    // Initialize VGA driver
    VGA::init();
    VGA::clear_screen();
    
    // Display boot message
    VGA::set_color(SCOS_NEON_GREEN);
    VGA::print("================================================\n");
    VGA::print("     SCos - Cyberpunk Operating System v1.0     \n");
    VGA::print("================================================\n\n");
    
    // Initialize hardware drivers
    VGA::print("[INIT] Initializing Timer...\n");
    Timer::init(1000);  // 1000 Hz timer
    
    VGA::print("[INIT] Initializing Keyboard...\n");
    Keyboard::init();
    
    VGA::print("[INIT] Initializing Mouse...\n");
    Mouse::init();
    
    VGA::print("[INIT] Setting up memory manager...\n");
    Memory::init();
    
    VGA::print("\n[OK] All systems initialized\n");
    VGA::print("[INFO] Starting desktop environment...\n\n");
    
    // Small delay for visual effect
    for (volatile int i = 0; i < 30000000; i++);
}

void kernel_panic(const char* message) {
    // Disable interrupts
    asm volatile("cli");
    
    // Display panic message
    VGA::set_color(0x4F);  // White on red
    VGA::clear_screen();
    
    VGA::print("\n\n");
    VGA::print("  ============================================\n");
    VGA::print("             !!! KERNEL PANIC !!!             \n");
    VGA::print("  ============================================\n\n");
    VGA::print("  Error: ");
    VGA::print(message);
    VGA::print("\n\n");
    VGA::print("  System halted. Please restart your computer.\n");
    VGA::print("  ============================================\n");
    
    // Halt the system
    while (true) {
        asm volatile("hlt");
    }
}
