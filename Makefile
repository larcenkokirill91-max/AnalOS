CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib -mpreferred-stack-boundary=2 -fno-tree-vectorize -mno-sse -mno-mmx -mno-80387 -Ikernel/include -Ilibc/include
CXXFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -nostdlib \
           -mpreferred-stack-boundary=2 -fno-tree-vectorize -mno-sse -mno-mmx -mno-80387 \
           -fno-exceptions -fno-rtti \
           -Ikernel/include -Ilibc/include -Igui

all: AnalOS.img

build/boot.bin: boot/bootloader.asm
	nasm -f bin boot/bootloader.asm -o build/boot.bin

build/boot_qemu.bin: boot/boot.asm
	nasm -f bin boot/boot.asm -o build/boot_qemu.bin

build/kernel_entry.o: boot/kernel_entry.asm
	nasm -f elf32 boot/kernel_entry.asm -o build/kernel_entry.o

build/kernel.o: kernel/kernel.c
	gcc $(CFLAGS) -c kernel/kernel.c -o build/kernel.o

build/screen.o: kernel/drivers/screen/screen.c
	gcc $(CFLAGS) -c kernel/drivers/screen/screen.c -o build/screen.o

build/font.o: kernel/include/drivers/font.bin
	objcopy -I binary -O elf32-i386 -B i386 --rename-section .data=.rodata,alloc,load,readonly,data kernel/include/drivers/font.bin build/font.o

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
	
build/gui_bridge.o: gui/gui_bridge.cpp
	g++ $(CXXFLAGS) -c gui/gui_bridge.cpp -o build/gui_bridge.o

build/window.o: gui/window.cpp
	g++ $(CXXFLAGS) -c gui/window.cpp -o build/window.o

build/start_menu.o: gui/start_menu.cpp
	g++ $(CXXFLAGS) -c gui/start_menu.cpp -o build/start_menu.o

OBJ = build/kernel_entry.o build/kernel.o build/screen.o build/font.o build/window.o \
      build/start_menu.o build/keyboard.o build/idt.o build/disk.o build/fs.o \
      build/lib.o build/math.o build/time.o build/gui_bridge.o

build/kernel.bin: $(OBJ) boot/linker.ld
	ld -m elf_i386 --oformat binary -T boot/linker.ld -o build/kernel.bin $(OBJ)

AnalOS.img: build/boot.bin build/kernel.bin
	cat build/boot.bin build/kernel.bin > AnalOS.img
	truncate -s 10485760 AnalOS.img

qemu: AnalOS.img
	qemu-system-i386 \
		-cpu Conroe-v1,-lm \
		-m 512M \
		-vga std \
		-drive file=AnalOS.img,format=raw,if=ide,index=0,media=disk \
		-boot c \
		-audiodev pa,id=snd0 \
		-machine pcspk-audiodev=snd0 \
		-d int \
		-D qemu.log

clean:
	rm -rf build/*.o build/*.bin AnalOS.img AnalOS-qemu.img