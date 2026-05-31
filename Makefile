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

kernel.bin: kernel.o screen.o keyboard.o linker.ld
	ld -m elf_i386 --oformat binary -T linker.ld -o kernel.bin kernel.o screen.o keyboard.o

os-image.img: boot.bin kernel.bin
	cat boot.bin kernel.bin > os-image.img
	truncate -s 1474560 os-image.img

# ==============================================================================
# АВТОМАТИЧЕСКИЙ ДЕПЛОЙ В СЕТЕВУЮ ПАПКУ
# ==============================================================================
run: os-image.img
	# 1. Сносим старый образ из сети, чтобы роутер его не кэшировал
	rm -f $(TFTP_DIR)/os-image.img
	# 2. Копируем свежую сборку
	cp os-image.img $(TFTP_DIR)/os-image.img
	# 3. Перезапускаем dnsmasq, чтобы обновить сетевую сессию
	-sudo systemctl restart dnsmasq

clean:
	rm -f *.bin *.o *.img qemu.log

