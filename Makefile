# Компиляторы и утилиты
CC = x86_64-w64-mingw32-gcc
CXX = x86_64-w64-mingw32-g++
ASM = nasm

# Флаги компиляции для C
CFLAGS = -ffreestanding -nostdlib -mno-red-zone -fno-builtin -Wall -O2 \
         -I. \
         -Isystem/include

# Флаги компиляции для C++ (ОБЯЗАТЕЛЬНО отключаем исключения и RTTI для ядра ОС)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti

ASMFLAGS = -f elf64

# Флаги линковщика (собираем EFI-приложение)
LDFLAGS = -Wl,--subsystem,10 \
          -Wl,-entry,efi_main \
          -Wl,--dll \
          -s

# Список всех исходников (УДАЛИЛИ mouse.cpp из CXX_SRCS)
# Список всех исходников
C_SRCS = boot/bootloader.c \
         system/kernel/kernel.c \
         system/drivers/screen.c \
         system/drivers/idt.c \
         system/drivers/keyboard.c \
         system/drivers/mouse.c

CXX_SRCS = system/drivers/mouse.cpp

ASM_SRCS = system/drivers/interrupts.asm



# Автоматическая генерация имен объектных файлов в папке build/
C_OBJS = $(addprefix build/, $(notdir $(C_SRCS:.c=.o)))
ASM_OBJS = $(addprefix build/, $(notdir $(ASM_SRCS:.asm=.o)))
# Для C++ файлов делаем суффикс _cpp.o, чтобы избежать конфликтов с .c файлами
CXX_OBJS = $(addprefix build/, $(notdir $(CXX_SRCS:.cpp=_cpp.o)))

OBJS = $(C_OBJS) $(CXX_OBJS) $(ASM_OBJS)

all: build

build: | build_dir $(OBJS)
	# Собираем через CXX, чтобы линкер понимал C++
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o BOOTX64.EFI $(OBJS)
	mkdir -p image/EFI/BOOT
	cp BOOTX64.EFI image/EFI/BOOT/BOOTX64.EFI
	echo "FS0:\\EFI\\BOOT\\BOOTX64.EFI" > image/startup.nsh

# --- АВТОМАТИЧЕСКИЕ ПРАВИЛА КОМПИЛЯЦИИ ---

# Правило для всех .c файлов
build/%.o: boot/%.c | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: system/kernel/%.c | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: system/drivers/%.c | build_dir
	$(CC) $(CFLAGS) -c $< -o $@

# Правило для всех .cpp файлов (создает %_cpp.o)
build/%_cpp.o: system/drivers/%.cpp | build_dir
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Правило для всех .asm файлов
build/%.o: system/drivers/%.asm | build_dir
	$(ASM) $(ASMFLAGS) $< -o $@

# Вспомогательное правило для папки
build_dir:
	mkdir -p build

run:
	$(MAKE) clean
	$(MAKE) build
	qemu-system-x86_64 \
		-bios ./OVMF.fd \
		-net none \
		-m 512M \
		-vga std \
		-global VGA.xres=1024 \
		-global VGA.yres=768 \
		-display gtk \
		-serial stdio \
		-drive if=none,id=usbstick,format=raw,file=fat:rw:image \
		-device usb-ehci,id=ehci \
		-device usb-storage,bus=ehci.0,drive=usbstick \
		-machine pc \
		-device isa-applesmc,osk="insertoskhereuphere" \
		-d int \
		-D qemu.log



clean:
	rm -f BOOTX64.EFI
	rm -rf image
	rm -rf build

.PHONY: all build run clean build_dir
