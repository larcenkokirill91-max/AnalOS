#include <kernel.h>

struct idt_entry idt[256];
__attribute__((interrupt))
void keyboard_handler(struct interrupt_frame* frame) {
    
    current_scancode = inb(0x60);
    io_wait();
    outb(0x20, 0x20);}
void default_handler(struct interrupt_frame* frame) {}
void idt_set_gate(int num, unsigned int handler, unsigned short selector, unsigned char flags) {
	idt[num].offset_lower = handler & 0xFFFF;
	idt[num].selector = selector;
	idt[num].zero = 0;
	idt[num].flags = flags;
	idt[num].offset_higher = handler >> 16;}
void pic_remap(void) {
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();
    outb(0x21, 0x20); io_wait();
    outb(0xA1, 0x28); io_wait();
    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();
    outb(0x21, 0xFD); io_wait();
    outb(0xA1, 0xEF); io_wait();}
void idt_init(void) {
        pic_remap();
	struct idt_ptr ptr;
	ptr.long_idt = 2047;
	ptr.addres = (unsigned int)&idt;
	for (int num = 0; num < 256; num++) {
		idt_set_gate(num, (unsigned int)default_handler, 0x08, 0x8E);
	}
	idt_set_gate(33, (unsigned int)keyboard_handler, 0x08, 0x8E);
	__asm__ __volatile__("lidt (%0)" : : "r"(&ptr));
	__asm__ __volatile__("sti");}
