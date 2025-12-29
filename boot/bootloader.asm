; SCos Bootloader
; Stage 1 bootloader - loads kernel from disk
; Neon Cyberpunk Theme

[BITS 16]
[ORG 0x7C00]

; Constants
KERNEL_LOAD_SEG     equ 0x1000      ; Segment to load kernel
KERNEL_LOAD_OFF     equ 0x0000      ; Offset within segment
SECTORS_TO_READ     equ 127         ; Number of sectors to read

; Entry point
start:
    ; Set up segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Save boot drive number
    mov [boot_drive], dl

    ; Print boot message
    mov si, msg_boot
    call print_string

    ; Reset disk system
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; Load kernel from disk
    mov si, msg_loading
    call print_string

    ; Set up for disk read
    mov ax, KERNEL_LOAD_SEG
    mov es, ax
    mov bx, KERNEL_LOAD_OFF

    mov ah, 0x02                    ; BIOS read sectors
    mov al, SECTORS_TO_READ         ; Number of sectors
    mov ch, 0                       ; Cylinder 0
    mov cl, 2                       ; Start from sector 2 (after bootloader)
    mov dh, 0                       ; Head 0
    mov dl, [boot_drive]            ; Drive number
    int 0x13
    jc disk_error

    mov si, msg_loaded
    call print_string

    ; Enable A20 line
    call enable_a20

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Switch to protected mode
    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to 32-bit code
    jmp CODE_SEG:protected_mode_start

;-------------------------------------------
; 16-bit helper functions
;-------------------------------------------

print_string:
    pusha
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    popa
    ret

disk_error:
    mov si, msg_disk_error
    call print_string
    cli
    hlt

enable_a20:
    ; Try BIOS method
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

;-------------------------------------------
; GDT (Global Descriptor Table)
;-------------------------------------------

gdt_start:

gdt_null:
    dq 0x0

gdt_code:
    dw 0xFFFF           ; Limit (low)
    dw 0x0000           ; Base (low)
    db 0x00             ; Base (middle)
    db 10011010b        ; Access: present, ring 0, code, executable, readable
    db 11001111b        ; Flags: 4KB granularity, 32-bit + Limit (high)
    db 0x00             ; Base (high)

gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b        ; Access: present, ring 0, data, writable
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1      ; Size
    dd gdt_start                     ; Address

; Segment selectors
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

;-------------------------------------------
; 32-bit Protected Mode
;-------------------------------------------

[BITS 32]

protected_mode_start:
    ; Set up segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Clear screen with green-on-black
    call clear_screen_32

    ; Jump to loaded kernel
    jmp KERNEL_LOAD_SEG * 16 + KERNEL_LOAD_OFF

clear_screen_32:
    mov edi, 0xB8000
    mov ecx, 80 * 25
    mov ax, 0x0A20          ; Green on black, space character
    rep stosw
    ret

;-------------------------------------------
; Data Section
;-------------------------------------------

boot_drive:         db 0
msg_boot:           db '[SCos] Bootloader v1.0', 13, 10, 0
msg_loading:        db '[SCos] Loading kernel...', 13, 10, 0
msg_loaded:         db '[SCos] Kernel loaded', 13, 10, 0
msg_disk_error:     db '[SCos] Disk error!', 13, 10, 0

;-------------------------------------------
; Boot Signature
;-------------------------------------------

times 510 - ($ - $$) db 0
dw 0xAA55
