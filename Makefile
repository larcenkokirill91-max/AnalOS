# Компиляторы и утилиты
CC = x86_64-w64-mingw32-gcc
ASM = nasm

# Флаги компиляции
CFLAGS = -ffreestanding -nostdlib -mno-red-zone -fno-builtin -Wall -O2 \
         -I. \
         -Isystem/include

ASMFLAGS = -f elf64

# Флаги линковщика (собираем EFI-приложение)
LDFLAGS = -Wl,--subsystem,10 \
          -Wl,-entry,efi_main \
          -Wl,--dll \
          -s

# Все исходные файлы на C (Включили mouse.c)
C_SRCS = boot/bootloader.c \
         system/kernel/kernel.c \
         system/drivers/screen.c \
         system/drivers/idt.c \
         system/drivers/keyboard.c \
         system/drivers/mouse.c

# Все исходные файлы на Ассемблере
ASM_SRCS = system/drivers/interrupts.asm

# Магия: берем только имена файлов (без старых папок) и переселяем их в папку build/
C_OBJS = $(addprefix build/, $(notdir $(C_SRCS:.c=.o)))
ASM_OBJS = $(addprefix build/, $(notdir $(ASM_SRCS:.asm=.o)))
OBJS = $(C_OBJS) $(ASM_OBJS)

all: build

# Правило для сборки EFI из всех объектных файлов
build: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o BOOTX64.EFI $(OBJS)
	mkdir -p image/EFI/BOOT
	cp BOOTX64.EFI image/EFI/BOOT/BOOTX64.EFI
	echo "FS0:\\EFI\\BOOT\\BOOTX64.EFI" > image/startup.nsh

# Правила для компиляции файлов .c в папку build/
build/bootloader.o: boot/bootloader.c | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.o: system/kernel/kernel.c | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

build/screen.o: system/drivers/screen.c | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

build/idt.o: system/drivers/idt.c | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

build/keyboard.o: system/drivers/keyboard.c | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

build/mouse.o: system/drivers/mouse.c | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

# Правило для компиляции файлов .asm в папку build/
build/interrupts.o: system/drivers/interrupts.asm | build_dir
	$(ASM) $(ASMFLAGS) $< -o $@

# Вспомогательное правило, гарантирующее, что папка build существует перед компиляцией
build_dir:
	mkdir -p build

run: build
	qemu-system-x86_64 \
		-bios ./OVMF.fd \
		-net none \
		-m 512M \
		-vga std \
		-global VGA.xres=1024 \
		-global VGA.yres=768 \
		-display sdl \
		-serial stdio \
		-drive if=none,id=usbstick,format=raw,file=fat:rw:image \
		-device usb-ehci,id=ehci \
		-device usb-storage,bus=ehci.0,drive=usbstick \
		-d int \
		-D qemu.log

clean:
	rm -rf BOOTX64.EFI image build

.PHONY: all build run clean build_dir
