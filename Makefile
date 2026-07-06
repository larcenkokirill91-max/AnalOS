CC = x86_64-w64-mingw32-gcc

CFLAGS = -ffreestanding -nostdlib -mno-red-zone -fno-builtin -Wall -O2 \
         -I. \
         -Isystem/include

LDFLAGS = -Wl,--subsystem,10 \
          -Wl,-entry,efi_main \
          -Wl,--dll \
          -s

SRCS = system/kernel/kernel.c boot/gop.c system/drivers/screen.c

all: build

build:
	$(CC) $(CFLAGS) $(LDFLAGS) -o BOOTX64.EFI $(SRCS)
	mkdir -p image/EFI/BOOT
	cp BOOTX64.EFI image/EFI/BOOT/BOOTX64.EFI
	echo "FS0:\\EFI\\BOOT\\BOOTX64.EFI" > image/startup.nsh

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
		-device usb-storage,bus=ehci.0,drive=usbstick

clean:
	rm -rf BOOTX64.EFI image

.PHONY: all build run clean
