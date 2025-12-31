; ============================================================================
; SCos 1.3.5 Bootloader
; Two-stage bootloader: Stage 1 (MBR) loads Stage 2, Stage 2 loads kernel
; ============================================================================

[bits 16]
[org 0x7C00]

; ============================================================================
; Stage 1: Master Boot Record (512 bytes)
; ============================================================================

KERNEL_LOAD_SEG     equ 0x1000      ; Kernel loaded at 0x10000
KERNEL_LOAD_OFF     equ 0x0000
KERNEL_SECTORS      equ 128         ; 64KB kernel max
STAGE2_SEG          equ 0x0800      ; Stage 2 at 0x8000
STAGE2_OFF          equ 0x0000
STAGE2_SECTORS      equ 4           ; 2KB for stage 2

stage1_start:
    ; Set up segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00              ; Stack below bootloader

    ; Save boot drive
    mov [boot_drive], dl

    ; Clear screen
    mov ax, 0x0003
    int 0x10

    ; Set green text color
    mov ah, 0x0B
    mov bh, 0x00
    mov bl, 0x00                ; Black background
    int 0x10

    ; Print loading message
    mov si, msg_loading
    call print_string_16

    ; Load Stage 2
    mov ax, STAGE2_SEG
    mov es, ax
    mov bx, STAGE2_OFF
    mov ah, 0x02                ; BIOS read sectors
    mov al, STAGE2_SECTORS
    mov ch, 0                   ; Cylinder 0
    mov cl, 2                   ; Sector 2 (after MBR)
    mov dh, 0                   ; Head 0
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; Jump to Stage 2
    jmp STAGE2_SEG:STAGE2_OFF

disk_error:
    mov si, msg_disk_err
    call print_string_16
    jmp halt_16

halt_16:
    cli
    hlt
    jmp halt_16

; Print string in 16-bit mode (SI = string pointer)
print_string_16:
    pusha
    mov ah, 0x0E
    mov bx, 0x000A              ; Green color
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    popa
    ret

; Data
boot_drive:     db 0
msg_loading:    db "SCos 1.3.5 Bootloader", 13, 10, 0
msg_disk_err:   db "Disk error!", 13, 10, 0

; Pad to 510 bytes and add boot signature
times 510 - ($ - $$) db 0
dw 0xAA55

; ============================================================================
; Stage 2: Extended Bootloader
; ============================================================================

[bits 16]

stage2_start:
    ; Set up segments again
    mov ax, 0x0800
    mov ds, ax
    xor ax, ax
    mov es, ax

    ; Print stage 2 message
    push ds
    xor ax, ax
    mov ds, ax
    mov si, msg_stage2
    call print_string_16_stage2
    pop ds

    ; Enable A20 line
    call enable_a20

    ; Load kernel from disk
    push ds
    xor ax, ax
    mov ds, ax
    mov si, msg_load_kernel
    call print_string_16_stage2
    pop ds

    ; Load kernel to 0x10000
    mov ax, KERNEL_LOAD_SEG
    mov es, ax
    mov bx, KERNEL_LOAD_OFF

    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, 0                   ; Cylinder 0
    mov cl, 6                   ; Sector 6 (after stage 2)
    mov dh, 0                   ; Head 0
    mov dl, [0x7C00 + boot_drive - stage1_start]  ; Boot drive
    int 0x13
    jc stage2_disk_error

    ; Print protected mode message
    push ds
    xor ax, ax
    mov ds, ax
    mov si, msg_pmode
    call print_string_16_stage2
    pop ds

    ; Disable interrupts
    cli

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to 32-bit code
    jmp 0x08:protected_mode_start

stage2_disk_error:
    push ds
    xor ax, ax
    mov ds, ax
    mov si, msg_disk_err2
    call print_string_16_stage2
    pop ds
.halt:
    cli
    hlt
    jmp .halt

print_string_16_stage2:
    pusha
    mov ah, 0x0E
    mov bx, 0x000A
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    popa
    ret

; Enable A20 Line
enable_a20:
    ; Try BIOS method first
    mov ax, 0x2401
    int 0x15
    jnc .done

    ; Try keyboard controller method
    call .wait_input
    mov al, 0xAD
    out 0x64, al

    call .wait_input
    mov al, 0xD0
    out 0x64, al

    call .wait_output
    in al, 0x60
    push ax

    call .wait_input
    mov al, 0xD1
    out 0x64, al

    call .wait_input
    pop ax
    or al, 2
    out 0x60, al

    call .wait_input
    mov al, 0xAE
    out 0x64, al

    call .wait_input
.done:
    ret

.wait_input:
    in al, 0x64
    test al, 2
    jnz .wait_input
    ret

.wait_output:
    in al, 0x64
    test al, 1
    jz .wait_output
    ret

; GDT for protected mode
gdt_start:
    ; Null descriptor
    dq 0

gdt_code:
    ; Code segment: base=0, limit=4GB, executable, readable
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10011010b    ; Access: present, ring 0, code, executable, readable
    db 11001111b    ; Flags: 4KB granularity, 32-bit, limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)

gdt_data:
    ; Data segment: base=0, limit=4GB, writable
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10010010b    ; Access: present, ring 0, data, writable
    db 11001111b    ; Flags: 4KB granularity, 32-bit, limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size
    dd gdt_start + 0x8000       ; GDT address (adjusted for segment)

; Messages
msg_stage2:      db "Stage 2 loaded", 13, 10, 0
msg_load_kernel: db "Loading kernel...", 13, 10, 0
msg_pmode:       db "Entering protected mode...", 13, 10, 0
msg_disk_err2:   db "Kernel load failed!", 13, 10, 0

; Pad stage 2 to 2KB (4 sectors)
times 2048 - ($ - stage2_start) db 0

; ============================================================================
; 32-bit Protected Mode Entry
; ============================================================================

[bits 32]

protected_mode_start:
    ; Set up segment registers for flat model
    mov ax, 0x10            ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000        ; Stack at 576KB

    ; Clear screen with green attribute
    mov edi, 0xB8000
    mov ecx, 2000           ; 80*25 characters
    mov ax, 0x0A20          ; Green space
    rep stosw

    ; Print boot message
    mov esi, msg_booting
    mov edi, 0xB8000
    mov ah, 0x0A            ; Green on black
.print_loop:
    lodsb
    test al, al
    jz .print_done
    stosw
    jmp .print_loop
.print_done:

    ; Jump to kernel at 0x10000
    jmp 0x10000

msg_booting: db "Booting SCos kernel...", 0

; Fill rest of bootloader space
times 4096 - ($ - stage2_start) db 0
