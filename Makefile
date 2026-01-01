# ============================================================================
# SCos 1.3.5 - Build System
# ============================================================================

# Tools
CC = i686-elf-gcc
AS = nasm
LD = i686-elf-ld
OBJCOPY = i686-elf-objcopy

# Flags
CFLAGS = -ffreestanding -O2 -Wall -Wextra -nostdlib -nostdinc -fno-builtin \
         -fno-stack-protector -nostartfiles -nodefaultlibs \
         -I./include -m32 -march=i686

ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -nostdlib

# Directories
BOOT_DIR = boot
KERNEL_DIR = kernel
USER_DIR = user
BUILD_DIR = build
ISO_DIR = isodir

# Source files
BOOT_SRC = $(BOOT_DIR)/boot.asm
GDT_IDT_SRC = $(KERNEL_DIR)/gdt_idt.asm

KERNEL_C_SRC = $(KERNEL_DIR)/kernel.c \
               $(KERNEL_DIR)/interrupts.c \
               $(KERNEL_DIR)/memory.c \
               $(KERNEL_DIR)/scheduler.c \
               $(KERNEL_DIR)/syscalls.c \
               $(KERNEL_DIR)/fs.c \
               $(KERNEL_DIR)/vga.c \
               $(KERNEL_DIR)/keyboard.c \
               $(KERNEL_DIR)/timer.c \
               $(KERNEL_DIR)/rtc.c \
               $(KERNEL_DIR)/string.c

USER_C_SRC = $(USER_DIR)/shell.c \
             $(USER_DIR)/commands.c \
             $(USER_DIR)/notepad.c \
             $(USER_DIR)/calendar.c \
             $(USER_DIR)/calc.c \
             $(USER_DIR)/ping.c

# Object files
BOOT_OBJ = $(BUILD_DIR)/boot.bin
GDT_IDT_OBJ = $(BUILD_DIR)/gdt_idt.o
KERNEL_C_OBJ = $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/kernel_%.o,$(KERNEL_C_SRC))
USER_C_OBJ = $(patsubst $(USER_DIR)/%.c,$(BUILD_DIR)/user_%.o,$(USER_C_SRC))

ALL_OBJ = $(GDT_IDT_OBJ) $(KERNEL_C_OBJ) $(USER_C_OBJ)

# Output files
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
OS_IMAGE = scos.img
ISO_IMAGE = scos.iso

# Targets
.PHONY: all clean run run-debug iso

all: $(OS_IMAGE)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile bootloader
$(BOOT_OBJ): $(BOOT_SRC) | $(BUILD_DIR)
	$(AS) -f bin $< -o $@

# Compile GDT/IDT assembly
$(GDT_IDT_OBJ): $(GDT_IDT_SRC) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile kernel C files
$(BUILD_DIR)/kernel_%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile user C files
$(BUILD_DIR)/user_%.o: $(USER_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel
$(KERNEL_ELF): $(ALL_OBJ) | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $(ALL_OBJ)

# Create flat binary from ELF
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# Create disk image
$(OS_IMAGE): $(BOOT_OBJ) $(KERNEL_BIN)
	# Create 1.44MB floppy image
	dd if=/dev/zero of=$@ bs=512 count=2880
	# Write bootloader (first 4KB = 8 sectors including stage 2)
	dd if=$(BOOT_OBJ) of=$@ conv=notrunc bs=512
	# Write kernel starting at sector 6
	dd if=$(KERNEL_BIN) of=$@ conv=notrunc bs=512 seek=5
	@echo ""
	@echo "Build complete: $(OS_IMAGE)"
	@echo "Bootloader: $$(wc -c < $(BOOT_OBJ)) bytes"
	@echo "Kernel: $$(wc -c < $(KERNEL_BIN)) bytes"

# Create ISO image
iso: $(OS_IMAGE)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(OS_IMAGE) $(ISO_DIR)/boot/
	echo 'menuentry "SCos 1.3.5" { multiboot /boot/$(OS_IMAGE) }' > $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO_IMAGE) $(ISO_DIR) 2>/dev/null || \
		echo "Note: grub-mkrescue not available, using raw image"

# Run in QEMU
run: $(OS_IMAGE)
	qemu-system-i386 -fda $(OS_IMAGE) -m 32M -monitor stdio

# Run with debug output
run-debug: $(OS_IMAGE)
	qemu-system-i386 -fda $(OS_IMAGE) -m 32M -d int,cpu_reset -no-reboot

# Run with serial output
run-serial: $(OS_IMAGE)
	qemu-system-i386 -fda $(OS_IMAGE) -m 32M -serial stdio

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(OS_IMAGE) $(ISO_IMAGE)
	rm -rf $(ISO_DIR)

# Install cross-compiler (helper target)
install-toolchain:
	@echo "To build SCos, you need an i686-elf cross-compiler."
	@echo "On Ubuntu/Debian:"
	@echo "  sudo apt install build-essential bison flex libgmp3-dev"
	@echo "  sudo apt install libmpc-dev libmpfr-dev texinfo nasm qemu-system-x86"
	@echo ""
	@echo "Then build binutils and GCC for i686-elf target."
	@echo "See: https://wiki.osdev.org/GCC_Cross-Compiler"

# Help
help:
	@echo "SCos 1.3.5 Build System"
	@echo "======================="
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build the OS image"
	@echo "  make run      - Build and run in QEMU"
	@echo "  make run-debug - Run with debug output"
	@echo "  make iso      - Create bootable ISO"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make help     - Show this help"
	@echo ""
	@echo "Requirements:"
	@echo "  - i686-elf-gcc cross-compiler"
	@echo "  - NASM assembler"
	@echo "  - QEMU (for testing)"
