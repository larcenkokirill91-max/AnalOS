#ifndef IDT_H
#define IDT_H
struct interrupt_frame;
struct idt_entry {
	unsigned short offset_lower;
	unsigned short selector;
	unsigned char zero;
	unsigned char flags;
	unsigned short offset_higher;
}__attribute__((packed));
extern struct idt_entry idt[256];
struct idt_ptr{
	unsigned short long_idt;
	unsigned int addres;
}__attribute__((packed));
void keyboard_handler(struct interrupt_frame* frame);
void default_handler(struct interrupt_frame* frame);
void idt_set_gate(int num, unsigned int handler, unsigned short selector, unsigned char flags);
void idt_init(void);
#endif
