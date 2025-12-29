; SCos Kernel Entry Point
; Multiboot compliant entry that calls the C++ kernel

[BITS 32]

; Multiboot header constants
MBOOT_PAGE_ALIGN    equ 1 << 0
MBOOT_MEM_INFO      equ 1 << 1
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

; Multiboot header (must be in first 8KB of kernel)
section .multiboot
align 4
    dd MBOOT_HEADER_MAGIC
    dd MBOOT_HEADER_FLAGS
    dd MBOOT_CHECKSUM

; Kernel entry point
section .text
global _start
global kernel_start
extern kernel_main

_start:
kernel_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Clear EFLAGS
    push 0
    popf
    
    ; Push multiboot info (ebx = multiboot info pointer, eax = magic)
    push ebx
    push eax
    
    ; Call C++ kernel
    call kernel_main
    
    ; If kernel returns, halt
.hang:
    cli
    hlt
    jmp .hang

; Stack
section .bss
align 16
stack_bottom:
    resb 16384      ; 16 KB stack
stack_top:
