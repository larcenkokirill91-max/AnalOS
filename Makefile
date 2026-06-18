TFTP_DIR = /srv/tftp

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -mpreferred-stack-boundary=2 -fno-tree-vectorize -Ikernel/include -Ilibc/include

all: AnalOS.img

build/boot.bin: boot/bootloader.asm
	nasm -f bin boot/bootloader.asm -o build/boot.bin

build/kernel_entry.o: boot/kernel_entry.asm
	nasm -f elf32 boot/kernel_entry.asm -o build/kernel_entry.o

build/boot_qemu.bin: boot/boot.asm
	nasm -f bin boot/boot.asm -o build/boot_qemu.bin

build/kernel.o: kernel/kernel.c
	gcc $(CFLAGS) -c kernel/kernel.c -o build/kernel.o

build/screen.o: kernel/drivers/screen/screen.c
	gcc $(CFLAGS) -c kernel/drivers/screen/screen.c -o build/screen.o

build/font.o: kernel/drivers/screen/font.c
	gcc $(CFLAGS) -c kernel/drivers/screen/font.c -o build/font.o

build/window.o: kernel/drivers/screen/window.c
	gcc $(CFLAGS) -c kernel/drivers/screen/window.c -o build/window.o

build/start_menu.o: kernel/drivers/screen/start_menu.c
	gcc $(CFLAGS) -c kernel/drivers/screen/start_menu.c -o build/start_menu.o

build/keyboard.o: kernel/drivers/keyboard/keyboard.c
	gcc $(CFLAGS) -c kernel/drivers/keyboard/keyboard.c -o build/keyboard.o

build/idt.o: kernel/drivers/keyboard/idt.c
	gcc $(CFLAGS) -mno-sse -mno-mmx -mno-80387 -c kernel/drivers/keyboard/idt.c -o build/idt.o

build/disk.o: kernel/drivers/disk/disk.c
	gcc $(CFLAGS) -c kernel/drivers/disk/disk.c -o build/disk.o

build/fs.o: kernel/drivers/disk/fs.c
	gcc $(CFLAGS) -c kernel/drivers/disk/fs.c -o build/fs.o

build/lib.o: libc/lib.c
	gcc $(CFLAGS) -c libc/lib.c -o build/lib.o

build/math.o: libc/math.c
	gcc $(CFLAGS) -c libc/math.c -o build/math.o

build/time.o: libc/time.c
	gcc $(CFLAGS) -c libc/time.c -o build/time.o

OBJ = build/kernel.o build/screen.o build/font.o build/window.o build/start_menu.o \
      build/keyboard.o build/idt.o build/disk.o build/fs.o \
      build/lib.o build/math.o build/time.o

build/kernel.bin: $(OBJ) boot/linker.ld
	ld -m elf_i386 --oformat binary -T boot/linker.ld -o build/kernel.bin $(OBJ)

AnalOS.img: build/boot.bin build/kernel.bin
	cat build/boot.bin build/kernel.bin > AnalOS.img
	truncate -s 1474560 AnalOS.img

os-qemu.img: build/boot_qemu.bin build/kernel.bin
	cat build/boot_qemu.bin build/kernel.bin > AnalOS-qemu.img
	truncate -s 10485760 AnalOS-qemu.img

qemu: os-qemu.img
	qemu-system-i386 \
		-machine type=pc,accel=tcg \
		-cpu pentium3 \
		-vga std \
		-display sdl \
		-m 512M \
		-drive file=AnalOS-qemu.img,format=raw,media=disk,index=0 \
		-k ru \
		-boot c \
		-d int,cpu_reset -D qemu.log

clean:
	rm -rf build/*.o build/*.bin AnalOS.img AnalOS-qemu.img