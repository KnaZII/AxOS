# AxOS Makefile

# Компиляторы и инструменты
CC = gcc
AS = "C:\Program Files\NASM\nasm.exe"
LD = ld

# Флаги компиляции
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -c
ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -nostdlib

# Исходные файлы
C_SOURCES = kernel.c console.c keyboard.c string.c
ASM_SOURCES = kernel_entry.asm
BOOT_SOURCE = boot.asm

# Объектные файлы
C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
BOOT_OBJECT = boot.bin

# Цели
all: axos.img

# Создание образа диска
axos.img: $(BOOT_OBJECT) kernel.bin
	@echo "Creating disk image..."
	python create_image.py

# Сборка ядра
kernel.bin: $(C_OBJECTS) $(ASM_OBJECTS) linker.ld
	@echo "Linking kernel..."
	$(CC) -m32 -nostdlib -T linker.ld -o kernel.elf $(ASM_OBJECTS) $(C_OBJECTS)
	@echo "Converting to binary..."
	objcopy -O binary kernel.elf kernel.bin

# Компиляция C файлов
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $< -o $@

# Компиляция ASM файлов для ядра
%.o: %.asm
	@echo "Assembling $<..."
	$(AS) $(ASFLAGS) $< -o $@

# Сборка загрузчика
$(BOOT_OBJECT): $(BOOT_SOURCE)
	@echo "Assembling bootloader..."
	$(AS) -f bin $(BOOT_SOURCE) -o $(BOOT_OBJECT)

# Запуск в QEMU
run: axos.img
	@echo "Starting AxOS in QEMU..."
	"C:\Program Files\qemu\qemu-system-i386.exe" -drive format=raw,file=axos.img,if=floppy

# Запуск в QEMU с отладкой
debug: axos.img
	@echo "Starting AxOS in QEMU with debugging..."
	qemu-system-i386 -fda axos.img -s -S

# Очистка
clean:
	@echo "Cleaning up..."
	-del /Q *.o *.bin *.elf *.img 2>nul || echo "No files to clean"

# Установка зависимостей (для Ubuntu/Debian)
install-deps:
	@echo "Installing dependencies..."
	sudo apt-get update
	sudo apt-get install gcc nasm qemu-system-x86 build-essential

# Помощь
help:
	@echo "AxOS Build System"
	@echo "Available targets:"
	@echo "  all          - Build the complete OS image"
	@echo "  kernel.bin   - Build only the kernel"
	@echo "  boot.bin     - Build only the bootloader"
	@echo "  run          - Build and run in QEMU"
	@echo "  debug        - Build and run in QEMU with debugging"
	@echo "  clean        - Remove all build files"
	@echo "  install-deps - Install required dependencies"
	@echo "  help         - Show this help message"

.PHONY: all run debug clean install-deps help