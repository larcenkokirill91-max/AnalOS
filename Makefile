all: os-image.bin

boot.bin: boot/bootloader.asm
	nasm -f bin boot/bootloader.asm -o boot.bin

kernel.o: src/kernel/kernel.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c src/kernel/kernel.c -o kernel.o

kernel.bin: kernel.o linker.ld
	ld -m elf_i386 -T linker.ld -o kernel.bin kernel.o

os-image.bin: boot.bin kernel.bin
	cat boot.bin kernel.bin > os-image.bin
	truncate -s 1474560 os-image.bin

run: os-image.bin
	# Запуск с графическим окном, логированием CPU и прерываний в файл qemu.log
	qemu-system-i386 -fda os-image.bin -vga std -display sdl -d cpu,int,guest_errors -D qemu.log

clean:
	rm -f *.bin *.o qemu.log

