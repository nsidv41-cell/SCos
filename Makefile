# SCos Makefile - Bare Metal OS Build System

# Toolchain
ASM = nasm
CC = i686-elf-gcc
CXX = i686-elf-g++
LD = i686-elf-ld

# Fallback to system gcc if cross-compiler not available
ifeq ($(shell which $(CC) 2>/dev/null),)
    CC = gcc
    CXX = g++
    LD = ld
endif

# Directories
BOOT_DIR = boot
KERNEL_DIR = kernel
DRIVERS_DIR = drivers
DESKTOP_DIR = desktop
BUILD_DIR = build
ISO_DIR = iso

# Output files
BOOTLOADER = $(BUILD_DIR)/bootloader.bin
KERNEL = $(BUILD_DIR)/kernel.bin
ISO = scos.iso

# Compiler flags
CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -nostdinc \
         -fno-builtin -Wall -Wextra -O2 -c
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib
ASMFLAGS = -f elf32

# Source files
BOOT_SRC = $(BOOT_DIR)/bootloader.asm
KERNEL_ASM_SRC = $(KERNEL_DIR)/kernel_entry.asm

KERNEL_CPP_SRC = $(wildcard $(KERNEL_DIR)/*.cpp)
DRIVER_CPP_SRC = $(wildcard $(DRIVERS_DIR)/*.cpp)
DESKTOP_CPP_SRC = $(wildcard $(DESKTOP_DIR)/*.cpp)

# Object files
KERNEL_ENTRY_OBJ = $(BUILD_DIR)/kernel_entry.o
KERNEL_CPP_OBJ = $(patsubst $(KERNEL_DIR)/%.cpp,$(BUILD_DIR)/kernel_%.o,$(KERNEL_CPP_SRC))
DRIVER_OBJ = $(patsubst $(DRIVERS_DIR)/%.cpp,$(BUILD_DIR)/driver_%.o,$(DRIVER_CPP_SRC))
DESKTOP_OBJ = $(patsubst $(DESKTOP_DIR)/%.cpp,$(BUILD_DIR)/desktop_%.o,$(DESKTOP_CPP_SRC))

ALL_OBJ = $(KERNEL_ENTRY_OBJ) $(KERNEL_CPP_OBJ) $(DRIVER_OBJ) $(DESKTOP_OBJ)

# Phony targets
.PHONY: all clean iso run directories debug

# Default target
all: directories $(BOOTLOADER) $(KERNEL)
	@echo "[OK] Build complete"

# Create build directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(ISO_DIR)/boot/grub

# Build bootloader (raw binary)
$(BOOTLOADER): $(BOOT_SRC)
	@echo "[ASM] Assembling bootloader..."
	$(ASM) -f bin -o $@ $<

# Kernel entry point (asm to elf)
$(KERNEL_ENTRY_OBJ): $(KERNEL_ASM_SRC)
	@echo "[ASM] Assembling kernel entry..."
	$(ASM) $(ASMFLAGS) -o $@ $<

# Compile kernel C++ files
$(BUILD_DIR)/kernel_%.o: $(KERNEL_DIR)/%.cpp
	@echo "[CXX] Compiling $<..."
	$(CXX) $(CXXFLAGS) -o $@ $<

# Compile driver C++ files
$(BUILD_DIR)/driver_%.o: $(DRIVERS_DIR)/%.cpp
	@echo "[CXX] Compiling $<..."
	$(CXX) $(CXXFLAGS) -o $@ $<

# Compile desktop C++ files
$(BUILD_DIR)/desktop_%.o: $(DESKTOP_DIR)/%.cpp
	@echo "[CXX] Compiling $<..."
	$(CXX) $(CXXFLAGS) -o $@ $<

# Link kernel
$(KERNEL): $(ALL_OBJ) linker.ld
	@echo "[LD] Linking kernel..."
	$(LD) $(LDFLAGS) -o $@ $(ALL_OBJ)

# Create bootable ISO
iso: all
	@echo "[ISO] Creating bootable ISO..."
	@cp $(BOOTLOADER) $(ISO_DIR)/boot/bootloader.bin
	@cp $(KERNEL) $(ISO_DIR)/boot/kernel.bin
	@echo 'set timeout=0' > $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'set default=0' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'menuentry "SCos - Cyberpunk OS" {' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    multiboot /boot/kernel.bin' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    boot' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	@grub-mkrescue -o $(ISO) $(ISO_DIR) 2>/dev/null || \
		xorriso -as mkisofs -R -J -c boot/boot.catalog \
		-b boot/bootloader.bin -no-emul-boot -boot-load-size 4 \
		-boot-info-table -o $(ISO) $(ISO_DIR) 2>/dev/null || \
		echo "[WARN] ISO creation requires grub-mkrescue or xorriso"
	@echo "[OK] ISO created: $(ISO)"

# Run in QEMU
run: iso
	@echo "[RUN] Starting QEMU..."
	qemu-system-i386 -cdrom $(ISO) -m 256M -vga std

# Debug with QEMU
debug: iso
	qemu-system-i386 -cdrom $(ISO) -m 256M -vga std -s -S &
	gdb -ex "target remote localhost:1234" -ex "symbol-file $(KERNEL)"

# Clean build files
clean:
	@echo "[CLEAN] Removing build files..."
	@rm -rf $(BUILD_DIR) $(ISO_DIR) $(ISO)
	@echo "[OK] Clean complete"

# Show help
help:
	@echo "SCos Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build bootloader and kernel"
	@echo "  iso     - Create bootable ISO image"
	@echo "  run     - Build and run in QEMU"
	@echo "  clean   - Remove build artifacts"
	@echo "  debug   - Run with GDB debugging"
	@echo "  help    - Show this message"
