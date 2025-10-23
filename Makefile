CC = gcc
AS = "C:\Program Files\NASM\nasm.exe"
LD = ld
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -c -I include
ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -nostdlib
C_SOURCES = src/kernel/kernel.c src/kernel/idt.c src/kernel/pic.c src/kernel/pit.c src/kernel/task.c src/kernel/paging.c src/kernel/syscall.c src/kernel/gdt.c src/kernel/user.c src/kernel/exec.c src/drivers/console.c src/drivers/keyboard.c src/drivers/serial.c src/drivers/disk.c src/fs/vfs.c src/fs/fat.c src/lib/string.c
ASM_SOURCES = src/arch/x86/kernel_entry.asm src/arch/x86/isr.asm
BOOT_SOURCE = src/arch/x86/boot.asm
C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
BOOT_OBJECT = boot.bin
all: axos.img
axos.img: $(BOOT_OBJECT) kernel.bin
	python create_image.py
kernel.bin: $(C_OBJECTS) $(ASM_OBJECTS) linker.ld
	$(CC) -m32 -nostdlib -T linker.ld -o kernel.elf $(ASM_OBJECTS) $(C_OBJECTS)
	objcopy -O binary kernel.elf kernel.bin
%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@
$(BOOT_OBJECT): $(BOOT_SOURCE)
	$(AS) -f bin $(BOOT_SOURCE) -o $(BOOT_OBJECT)
run: axos.img
	"C:\Program Files\qemu\qemu-system-i386.exe" -boot a -drive format=raw,file=axos.img,if=floppy -drive format=raw,file=fs.img,if=ide
debug: axos.img
	qemu-system-i386 -fda axos.img -s -S
clean:
	-del /Q *.o *.bin *.elf *.img 2>nul || echo "No files to clean"
install-deps:
	sudo apt-get update
	sudo apt-get install gcc nasm qemu-system-x86 build-essential
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