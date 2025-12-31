/* ============================================================================
 * SCos 1.3.5 - Interrupt Handlers
 * ============================================================================ */

#include "../include/scos.h"

/* IRQ handler function pointers */
static void (*irq_handlers[16])(registers_t *) = { 0 };

/* Exception messages */
static const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

/* Initialize interrupts subsystem */
void interrupts_init(void) {
    /* Clear all handlers */
    for (int i = 0; i < 16; i++) {
        irq_handlers[i] = NULL;
    }
}

/* Install an IRQ handler */
void irq_install_handler(int irq, void (*handler)(registers_t *)) {
    if (irq >= 0 && irq < 16) {
        irq_handlers[irq] = handler;
    }
}

/* Uninstall an IRQ handler */
void irq_uninstall_handler(int irq) {
    if (irq >= 0 && irq < 16) {
        irq_handlers[irq] = NULL;
    }
}

/* ISR handler - called from assembly */
void isr_handler(registers_t *regs) {
    if (regs->int_no < 32) {
        /* CPU Exception */
        vga_set_attr(0x4F);  /* White on red */
        vga_clear();
        
        vga_puts("\n\n  *** EXCEPTION ***\n\n");
        vga_puts("  Exception: ");
        vga_puts(exception_messages[regs->int_no]);
        vga_puts("\n\n");
        
        char buf[64];
        sprintf(buf, "  Error Code: 0x%x\n", regs->err_code);
        vga_puts(buf);
        sprintf(buf, "  EIP: 0x%x\n", regs->eip);
        vga_puts(buf);
        sprintf(buf, "  CS:  0x%x\n", regs->cs);
        vga_puts(buf);
        sprintf(buf, "  EFLAGS: 0x%x\n", regs->eflags);
        vga_puts(buf);
        sprintf(buf, "  EAX: 0x%x  EBX: 0x%x\n", regs->eax, regs->ebx);
        vga_puts(buf);
        sprintf(buf, "  ECX: 0x%x  EDX: 0x%x\n", regs->ecx, regs->edx);
        vga_puts(buf);
        
        vga_puts("\n  System halted.\n");
        
        cli();
        while (1) {
            hlt();
        }
    }
}

/* IRQ handler - called from assembly */
void irq_handler(registers_t *regs) {
    /* Call the registered handler if one exists */
    uint32_t irq = regs->int_no - 32;
    
    if (irq < 16 && irq_handlers[irq] != NULL) {
        irq_handlers[irq](regs);
    }
    
    /* Send EOI (End of Interrupt) to PICs */
    if (regs->int_no >= 40) {
        /* Send EOI to slave PIC */
        outb(0xA0, 0x20);
    }
    /* Send EOI to master PIC */
    outb(0x20, 0x20);
}
