# ==============================================================================
# НАСТРОЙКИ СЕТЕВОЙ РАЗРАБОТКИ (PXE / LAN)
# ==============================================================================
TFTP_DIR = /srv/tftp

all: run

boot.bin: boot/bootloader.asm
	nasm -f bin boot/bootloader.asm -o boot.bin

kernel.o: src/kernel/kernel.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c src/kernel/kernel.c -o kernel.o

screen.o: drivers/screen/screen.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c drivers/screen/screen.c -o screen.o

keyboard.o: drivers/keyboard/keyboard.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c drivers/keyboard/keyboard.c -o keyboard.o

disk.o: drivers/disk/disk.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c drivers/disk/disk.c -o disk.o

window.o: src/kernel/window.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c src/kernel/window.c -o window.o

idt.o: src/kernel/idt.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -mno-sse -mno-mmx -mno-80387 -c src/kernel/idt.c -o idt.o

time.o: src/kernel/time.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c src/kernel/time.c -o time.o

font.o: drivers/screen/font.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c drivers/screen/font.c -o font.o

math.o: src/math.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c src/math.c -o math.o

start_menu.o: src/start_menu.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c src/start_menu.c -o start_menu.o

lib.o: lib/lib.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c lib/lib.c -o lib.o

kernel.bin: kernel.o screen.o keyboard.o disk.o window.o idt.o time.o font.o math.o start_menu.o lib.o linker.ld
	ld -m elf_i386 --oformat binary -T linker.ld -o kernel.bin kernel.o screen.o keyboard.o disk.o window.o idt.o time.o font.o math.o start_menu.o lib.o
	sudo cp kernel.bin /srv/tftp/kernel.bin

os-image.img: boot.bin kernel.bin
	cat boot.bin kernel.bin > os-image.img
	python3 mkfs.py
	truncate -s 1474560 os-image.img


# ==============================================================================
# АВТОМАТИЧЕСКИЙ ДЕПЛОЙ В СЕТЕВУЮ ПАПКУ
# ==============================================================================
run: os-image.img
	# 1. Сносим старый образ из сети, чтобы роутер его не кэшировал
	rm -f $(TFTP_DIR)/os-image.img
	# 2. Копируем свежую сборку образа и ЧИСТОГО ядра для флешки!
	cp os-image.img $(TFTP_DIR)/os-image.img
	# 3. Перезапускаем dnsmasq, чтобы обновить сетевую сессию
	-sudo systemctl restart dnsmasq

clean:
	rm -f *.bin *.o *.img qemu.log
