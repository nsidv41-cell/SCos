; SCos Bootloader
; Bare-metal bootloader for x86 systems
; Neon Cyberpunk Theme

[BITS 16]
[ORG 0x7C00]

; Constants
KERNEL_SEGMENT  equ 0x1000
KERNEL_OFFSET   equ 0x0000
SECTORS_TO_READ equ 64

start:
    ; Set up segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Save boot drive
    mov [boot_drive], dl

    ; Display boot message
    mov si, msg_boot
    call print_string

    ; Load kernel from disk
    call load_kernel

    ; Enable A20 line
    call enable_a20

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enter protected mode
    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to protected mode
    jmp CODE_SEG:protected_mode

; =============================================================================
; 16-bit Functions
; =============================================================================

print_string:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    mov bl, 0x0A          ; Light green color
    int 0x10
    jmp .loop
.done:
    popa
    ret

load_kernel:
    mov si, msg_loading
    call print_string

    mov ax, KERNEL_SEGMENT
    mov es, ax
    mov bx, KERNEL_OFFSET
    mov al, SECTORS_TO_READ
    mov ch, 0              ; Cylinder 0
    mov cl, 2              ; Sector 2 (1-indexed, sector 1 is bootloader)
    mov dh, 0              ; Head 0
    mov dl, [boot_drive]
    mov ah, 0x02           ; BIOS read sectors function
    int 0x13
    jc disk_error

    mov si, msg_loaded
    call print_string
    ret

disk_error:
    mov si, msg_error
    call print_string
    cli
    hlt

enable_a20:
    ; Try BIOS method first
    mov ax, 0x2401
    int 0x15
    jnc .done

    ; Fast A20 method
    in al, 0x92
    or al, 2
    out 0x92, al

.done:
    ret

; =============================================================================
; GDT (Global Descriptor Table)
; =============================================================================

gdt_start:

gdt_null:
    dd 0x0
    dd 0x0

gdt_code:
    dw 0xFFFF              ; Limit (bits 0-15)
    dw 0x0                 ; Base (bits 0-15)
    db 0x0                 ; Base (bits 16-23)
    db 10011010b           ; Access byte
    db 11001111b           ; Flags + Limit (bits 16-19)
    db 0x0                 ; Base (bits 24-31)

gdt_data:
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; =============================================================================
; 32-bit Protected Mode
; =============================================================================

[BITS 32]

protected_mode:
    ; Set up data segments
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov esp, 0x90000

    ; Set up VGA text mode with green on black
    call setup_vga_colors

    ; Jump to kernel
    jmp KERNEL_SEGMENT:KERNEL_OFFSET

setup_vga_colors:
    ; Set VGA text mode colors
    mov edi, 0xB8000
    mov ecx, 2000          ; 80x25 = 2000 characters
    mov ax, 0x0A20         ; Light green on black, space character
.clear:
    mov [edi], ax
    add edi, 2
    loop .clear
    ret

; =============================================================================
; Data Section
; =============================================================================

boot_drive:     db 0
msg_boot:       db '[SCos] Bootloader initializing...', 13, 10, 0
msg_loading:    db '[SCos] Loading kernel...', 13, 10, 0
msg_loaded:     db '[SCos] Kernel loaded successfully', 13, 10, 0
msg_error:      db '[SCos] Disk error!', 13, 10, 0

; =============================================================================
; Boot Sector Padding and Signature
; =============================================================================

times 510 - ($ - $$) db 0
dw 0xAA55
