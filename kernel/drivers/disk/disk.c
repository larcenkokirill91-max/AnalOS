#include <kernel.h>
#include <kernel.h>
#include <kernel.h>

static inline void outsl(int port, const void *addr, int cnt) {
    __asm__ __volatile__("cld; rep outsl" : "+S"(addr), "+c"(cnt) : "d"(port));
}

static inline void insl(int port, void *addr, int cnt) {
    __asm__ __volatile__("cld; rep insl" : "+D"(addr), "+c"(cnt) : "d"(port) : "memory");
}

void ata_delay(void) {
    inb(0x3F6); inb(0x3F6); inb(0x3F6); inb(0x3F6);
}

void ata_wait_ready(void) {
    while(1) {
        int status = inb(0x1F7);
        if ((status & 0x80) == 0 && (status & 0x40) != 0) {
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
	outb(0x1F6, ((unsigned char)(lba >> 24) & 0x0F) | 0xF0);
}

void ata_write_sector(unsigned int lba, unsigned short* buffer) {
    ata_send_lba(lba);
    outb(0x1F7, 0x30);
    ata_delay();
    int status = inb(0x1F7);
    if ((status & 0x01) != 0) { return; }
    outsl(0x1F0, buffer, 128);
    ata_wait_ready();
}

void ata_read_sector(unsigned int lba, unsigned short* buffer) {
	ata_send_lba(lba);
	outb(0x1F7, 0x20);
	ata_wait_ready();
	insl(0x1F0, buffer, 128);
}

void install_to_disk(void) {
    __asm__ __volatile__("cli");
    unsigned short* boot_ptr = (unsigned short*)0x7C00;
    unsigned char* kernel_buffer_start = (unsigned char*)0x1000;
    ata_write_sector(0, boot_ptr);
    ata_delay();
    for (int i = 1; i < 13; i++) {
        unsigned short* core_ptr = (unsigned short*)(kernel_buffer_start + ((i - 1) * 512));
        ata_write_sector(i, core_ptr);
        ata_delay();
    }
    ata_wait_ready();
    outb(0x1F7, 0xE7);
    ata_wait_ready();
    __asm__ __volatile__("sti");
}
