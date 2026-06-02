#include "../keyboard/keyboard.h"
void ata_wait_ready() {
	while(1) {
		int status = inb(0x1F7);
		if ((status & 0x80) == 0 && (status & 0x08) != 0) {
			break;
		}
		io_wait();
	}
}
void ata_send_lba(unsigned int lba) {
	ata_wait_ready();
	outb(0x1F2, 1);
	outb(0x1F3, lba);
	outb(0x1F4, lba >> 8);
	outb(0x1F5, lba >> 16);
	outb(0x1F6, (lba >> 24) & 0x0F | 0xE0);
}
void ata_read_sector(unsigned int lba, unsigned short* buffer) {
	ata_send_lba(lba);
	outb(0x1F7, 0x20);
	ata_wait_ready();
	for (int i = 0; i < 256; i++) {
		buffer[i] = inw(0x1F0);
	}
}
