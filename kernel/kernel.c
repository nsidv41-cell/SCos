/* ============================================================================
 * SCos 1.3.5 - Main Kernel
 * ============================================================================ */

#include "../include/scos.h"

/* Version information */
#define SCOS_VERSION "1.3.5"
#define SCOS_BUILD_DATE __DATE__
#define SCOS_CODENAME "Terminal"

/* GDT entries */
static gdt_entry_t gdt_entries[5];
static gdt_ptr_t gdt_ptr;

/* IDT entries */
static idt_entry_t idt_entries[256];
static idt_ptr_t idt_ptr;

/* Boot log function */
static void boot_log(const char *message) {
    vga_puts("[");
    vga_put_color(" OK ", 0x0A);
    vga_puts("] ");
    vga_puts(message);
    vga_puts("\n");
}

static void boot_log_info(const char *message) {
    vga_puts("[");
    vga_put_color("INFO", 0x0B);
    vga_puts("] ");
    vga_puts(message);
    vga_puts("\n");
}

/* Set up a GDT entry */
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, 
                         uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;
    
    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    
    gdt_entries[num].access = access;
}

/* Initialize GDT */
static void gdt_init(void) {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;
    
    gdt_set_gate(0, 0, 0, 0, 0);                    /* Null segment */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);    /* Code segment */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);    /* Data segment */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);    /* User code segment */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);    /* User data segment */
    
    gdt_flush((uint32_t)&gdt_ptr);
}

/* Set up an IDT entry */
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].selector = sel;
    idt_entries[num].zero = 0;
    idt_entries[num].flags = flags;
}

/* Initialize IDT */
static void idt_init(void) {
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_ptr.base = (uint32_t)&idt_entries;
    
    memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);
    
    /* Remap PIC */
    outb(0x20, 0x11);  /* Initialize master PIC */
    outb(0xA0, 0x11);  /* Initialize slave PIC */
    outb(0x21, 0x20);  /* Master PIC vector offset (IRQ 0-7 -> INT 32-39) */
    outb(0xA1, 0x28);  /* Slave PIC vector offset (IRQ 8-15 -> INT 40-47) */
    outb(0x21, 0x04);  /* Tell master about slave at IRQ2 */
    outb(0xA1, 0x02);  /* Tell slave its cascade identity */
    outb(0x21, 0x01);  /* 8086 mode for master */
    outb(0xA1, 0x01);  /* 8086 mode for slave */
    outb(0x21, 0x0);   /* Enable all IRQs on master */
    outb(0xA1, 0x0);   /* Enable all IRQs on slave */
    
    /* CPU Exceptions (ISR 0-31) */
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);
    
    /* Hardware Interrupts (IRQ 0-15) */
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
    
    /* System call interrupt */
    idt_set_gate(0x80, (uint32_t)syscall_entry, 0x08, 0xEE);
    
    idt_flush((uint32_t)&idt_ptr);
}

/* Print boot banner */
static void print_boot_banner(void) {
    vga_puts("\n");
    vga_put_color("  ____   ____           ", 0x0A);
    vga_puts("\n");
    vga_put_color(" / ___| / ___|___  ___  ", 0x0A);
    vga_puts("\n");
    vga_put_color(" \\___ \\| |   / _ \\/ __| ", 0x0A);
    vga_puts("\n");
    vga_put_color("  ___) | |__| (_) \\__ \\ ", 0x0A);
    vga_puts("\n");
    vga_put_color(" |____/ \\____\\___/|___/ ", 0x0A);
    vga_puts("  Version ");
    vga_puts(SCOS_VERSION);
    vga_puts("\n\n");
}

/* Kernel panic */
void kernel_panic(const char *message) {
    cli();
    
    vga_set_attr(0x4F);  /* White on red */
    vga_clear();
    
    vga_puts("\n\n");
    vga_puts("  *** KERNEL PANIC ***\n\n");
    vga_puts("  ");
    vga_puts(message);
    vga_puts("\n\n");
    vga_puts("  System halted. Please restart your computer.\n");
    
    while (1) {
        hlt();
    }
}

/* Main kernel entry point */
void kernel_main(void) {
    /* Initialize VGA first for output */
    vga_init();
    vga_set_attr(SCOS_ATTR);
    vga_clear();
    
    print_boot_banner();
    
    boot_log_info("SCos 1.3.5 \"Terminal\" starting...");
    boot_log_info("Build date: " SCOS_BUILD_DATE);
    vga_puts("\n");
    
    /* Initialize GDT */
    boot_log("Initializing GDT...");
    gdt_init();
    
    /* Initialize IDT */
    boot_log("Initializing IDT...");
    idt_init();
    
    /* Initialize memory manager */
    boot_log("Initializing memory manager...");
    memory_init();
    
    /* Initialize timer (100Hz) */
    boot_log("Initializing timer (100Hz)...");
    timer_init(100);
    
    /* Initialize keyboard */
    boot_log("Initializing keyboard driver...");
    keyboard_init();
    
    /* Initialize RTC */
    boot_log("Initializing RTC...");
    rtc_init();
    
    /* Enable interrupts */
    boot_log("Enabling interrupts...");
    sti();
    
    /* Initialize scheduler */
    boot_log("Initializing process scheduler...");
    scheduler_init();
    
    /* Initialize system calls */
    boot_log("Initializing system calls...");
    syscalls_init();
    
    /* Initialize filesystem */
    boot_log("Mounting root filesystem...");
    fs_init();
    
    vga_puts("\n");
    boot_log_info("All subsystems initialized successfully");
    
    /* Display memory info */
    char buf[64];
    sprintf(buf, "Free memory: %d KB", memory_get_free() / 1024);
    boot_log_info(buf);
    
    vga_puts("\n");
    boot_log("Starting init process...");
    vga_puts("\n");
    
    /* Small delay for boot messages to be visible */
    timer_sleep(500);
    
    /* Clear screen and start shell */
    vga_clear();
    
    /* Display login banner */
    vga_puts("\n");
    vga_put_color("SCos 1.3.5", 0x0A);
    vga_puts(" - Terminal Operating System\n");
    vga_puts("Type 'help' for available commands.\n\n");
    
    /* Initialize and run shell */
    shell_init();
    shell_run();
    
    /* Should never reach here */
    kernel_panic("Shell exited unexpectedly");
}
