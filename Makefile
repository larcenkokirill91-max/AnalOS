all: os-image.img

boot.bin: boot/bootloader.asm
	nasm -f bin boot/bootloader.asm -o boot.bin

kernel.o: src/kernel/kernel.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c src/kernel/kernel.c -o kernel.o

screen.o: drivers/screen/screen.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c drivers/screen/screen.c -o screen.o

keyboard.o: drivers/keyboard/keyboard.c
	gcc -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -c drivers/keyboard/keyboard.c -o keyboard.o

# ЖЕЛЕЗНО ИСПРАВЛЕНО: Добавлен флаг --oformat binary для сборки чистого плоского бинарника
kernel.bin: kernel.o screen.o keyboard.o linker.ld
	ld -m elf_i386 --oformat binary -T linker.ld -o kernel.bin kernel.o screen.o keyboard.o

os-image.img: boot.bin kernel.bin
	cat boot.bin kernel.bin > os-image.img
	truncate -s 1474560 os-image.img

run: os-image.img
	# Запуск с графическим окном, логированием CPU и прерываний в файл qemu.log
	qemu-system-i386 -fda os-image.img -vga std -display sdl -d cpu,int,guest_errors -D qemu.log

clean:
	rm -f *.bin *.o *.img qemu.log

