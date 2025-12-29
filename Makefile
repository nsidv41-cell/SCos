# SCos Makefile - Bare Metal OS Build System

# Toolchain
ASM = nasm
CC = gcc
CXX = g++
LD = ld

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
         -fno-builtin -fno-exceptions -fno-rtti -Wall -Wextra -O2
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib
ASMFLAGS = -f bin

# Source files
BOOT_SRC = $(BOOT_DIR)/bootloader.asm
KERNEL_SRC = $(wildcard $(KERNEL_DIR)/*.cpp)
DRIVER_SRC = $(wildcard $(DRIVERS_DIR)/*.cpp)
DESKTOP_SRC = $(wildcard $(DESKTOP_DIR)/*.cpp)

# Object files
KERNEL_OBJ = $(patsubst $(KERNEL_DIR)/%.cpp,$(BUILD_DIR)/kernel_%.o,$(KERNEL_SRC))
DRIVER_OBJ = $(patsubst $(DRIVERS_DIR)/%.cpp,$(BUILD_DIR)/driver_%.o,$(DRIVER_SRC))
DESKTOP_OBJ = $(patsubst $(DESKTOP_DIR)/%.cpp,$(BUILD_DIR)/desktop_%.o,$(DESKTOP_SRC))

ALL_OBJ = $(KERNEL_OBJ) $(DRIVER_OBJ) $(DESKTOP_OBJ)

# Phony targets
.PHONY: all clean iso run directories

# Default target
all: directories $(BOOTLOADER) $(KERNEL)
	@echo "[OK] Build complete"

# Create build directories
directories:
	@mkdir -p $(BUILD_DIR) $(ISO_DIR)/boot/grub

# Build bootloader
$(BOOTLOADER): $(BOOT_SRC)
	@echo "[ASM] Assembling bootloader..."
	$(ASM) -f bin -o $@ $<

# Compile kernel C++ files
$(BUILD_DIR)/kernel_%.o: $(KERNEL_DIR)/%.cpp
	@echo "[CXX] Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile driver C++ files
$(BUILD_DIR)/driver_%.o: $(DRIVERS_DIR)/%.cpp
	@echo "[CXX] Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile desktop C++ files
$(BUILD_DIR)/desktop_%.o: $(DESKTOP_DIR)/%.cpp
	@echo "[CXX] Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link kernel
$(KERNEL): $(ALL_OBJ)
	@echo "[LD] Linking kernel..."
	$(LD) $(LDFLAGS) -o $@ $^

# Create bootable ISO
iso: all
	@echo "[ISO] Creating bootable ISO..."
	@cp $(BOOTLOADER) $(ISO_DIR)/boot/bootloader.bin
	@cp $(KERNEL) $(ISO_DIR)/boot/kernel.bin
	@echo 'set timeout=0' > $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'set default=0' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo 'menuentry "SCos" {' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    multiboot /boot/kernel.bin' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '    boot' >> $(ISO_DIR)/boot/grub/grub.cfg
	@echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISO_DIR) 2>/dev/null || \
		xorriso -as mkisofs -R -J -b boot/bootloader.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		-o $(ISO) $(ISO_DIR)
	@echo "[OK] ISO created: $(ISO)"

# Run in QEMU
run: iso
	qemu-system-i386 -cdrom $(ISO) -m 256M

# Clean build files
clean:
	@echo "[CLEAN] Removing build files..."
	@rm -rf $(BUILD_DIR) $(ISO_DIR) $(ISO)
	@echo "[OK] Clean complete"
