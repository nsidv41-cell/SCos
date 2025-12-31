; ============================================================================
; SCos 1.3.5 - GDT and IDT Assembly Routines
; ============================================================================

[bits 32]

global gdt_flush
global idt_flush

; ISR declarations
global isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7
global isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15
global isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23
global isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31

; IRQ declarations
global irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7
global irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15

; System call entry
global syscall_entry

extern isr_handler
extern irq_handler
extern syscall_handler

; ============================================================================
; GDT Flush - Load new GDT and reload segment registers
; ============================================================================

gdt_flush:
    mov eax, [esp + 4]      ; Get GDT pointer parameter
    lgdt [eax]              ; Load GDT
    
    mov ax, 0x10            ; Data segment selector (index 2, ring 0)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    jmp 0x08:.flush         ; Far jump to code segment (index 1)
.flush:
    ret

; ============================================================================
; IDT Flush - Load new IDT
; ============================================================================

idt_flush:
    mov eax, [esp + 4]      ; Get IDT pointer parameter
    lidt [eax]              ; Load IDT
    ret

; ============================================================================
; ISR Common Stub - Saves state and calls C handler
; ============================================================================

isr_common_stub:
    pusha                   ; Push all general purpose registers
    
    mov ax, ds              ; Save data segment
    push eax
    
    mov ax, 0x10            ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp                ; Push pointer to registers structure
    call isr_handler
    add esp, 4
    
    pop eax                 ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                    ; Restore registers
    add esp, 8              ; Clean up error code and ISR number
    iret                    ; Return from interrupt

; ============================================================================
; IRQ Common Stub - Saves state and calls C handler
; ============================================================================

irq_common_stub:
    pusha                   ; Push all general purpose registers
    
    mov ax, ds              ; Save data segment
    push eax
    
    mov ax, 0x10            ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp                ; Push pointer to registers structure
    call irq_handler
    add esp, 4
    
    pop eax                 ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                    ; Restore registers
    add esp, 8              ; Clean up error code and IRQ number
    iret                    ; Return from interrupt

; ============================================================================
; ISR Handlers (Exceptions 0-31)
; ============================================================================

; ISR without error code - push dummy error code
%macro ISR_NOERRCODE 1
isr%1:
    push dword 0            ; Dummy error code
    push dword %1           ; Interrupt number
    jmp isr_common_stub
%endmacro

; ISR with error code - CPU pushes error code
%macro ISR_ERRCODE 1
isr%1:
    push dword %1           ; Interrupt number
    jmp isr_common_stub
%endmacro

; CPU Exceptions
ISR_NOERRCODE 0         ; Division by zero
ISR_NOERRCODE 1         ; Debug
ISR_NOERRCODE 2         ; Non-maskable interrupt
ISR_NOERRCODE 3         ; Breakpoint
ISR_NOERRCODE 4         ; Overflow
ISR_NOERRCODE 5         ; Bound range exceeded
ISR_NOERRCODE 6         ; Invalid opcode
ISR_NOERRCODE 7         ; Device not available
ISR_ERRCODE   8         ; Double fault
ISR_NOERRCODE 9         ; Coprocessor segment overrun
ISR_ERRCODE   10        ; Invalid TSS
ISR_ERRCODE   11        ; Segment not present
ISR_ERRCODE   12        ; Stack-segment fault
ISR_ERRCODE   13        ; General protection fault
ISR_ERRCODE   14        ; Page fault
ISR_NOERRCODE 15        ; Reserved
ISR_NOERRCODE 16        ; x87 FPU error
ISR_ERRCODE   17        ; Alignment check
ISR_NOERRCODE 18        ; Machine check
ISR_NOERRCODE 19        ; SIMD floating-point
ISR_NOERRCODE 20        ; Virtualization
ISR_NOERRCODE 21        ; Reserved
ISR_NOERRCODE 22        ; Reserved
ISR_NOERRCODE 23        ; Reserved
ISR_NOERRCODE 24        ; Reserved
ISR_NOERRCODE 25        ; Reserved
ISR_NOERRCODE 26        ; Reserved
ISR_NOERRCODE 27        ; Reserved
ISR_NOERRCODE 28        ; Reserved
ISR_NOERRCODE 29        ; Reserved
ISR_ERRCODE   30        ; Security exception
ISR_NOERRCODE 31        ; Reserved

; ============================================================================
; IRQ Handlers (IRQ 0-15 mapped to interrupts 32-47)
; ============================================================================

%macro IRQ 2
irq%1:
    push dword 0            ; Dummy error code
    push dword %2           ; Interrupt number
    jmp irq_common_stub
%endmacro

IRQ 0, 32               ; Timer
IRQ 1, 33               ; Keyboard
IRQ 2, 34               ; Cascade
IRQ 3, 35               ; COM2
IRQ 4, 36               ; COM1
IRQ 5, 37               ; LPT2
IRQ 6, 38               ; Floppy
IRQ 7, 39               ; LPT1
IRQ 8, 40               ; RTC
IRQ 9, 41               ; Free
IRQ 10, 42              ; Free
IRQ 11, 43              ; Free
IRQ 12, 44              ; PS/2 Mouse
IRQ 13, 45              ; FPU
IRQ 14, 46              ; Primary ATA
IRQ 15, 47              ; Secondary ATA

; ============================================================================
; System Call Entry (int 0x80)
; ============================================================================

syscall_entry:
    push dword 0            ; Dummy error code
    push dword 0x80         ; Syscall interrupt number
    
    pusha                   ; Save all registers
    
    mov ax, ds
    push eax
    
    mov ax, 0x10            ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call syscall_handler(num, arg1, arg2, arg3)
    ; Arguments are in eax, ebx, ecx, edx
    push edx                ; arg3
    push ecx                ; arg2
    push ebx                ; arg1
    push eax                ; syscall number
    call syscall_handler
    add esp, 16
    
    ; Return value is in eax, save it
    mov [esp + 32 + 4], eax ; Store return value in saved eax
    
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa
    add esp, 8
    iret
