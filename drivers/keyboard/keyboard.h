#ifndef KEYBOARD_H
#define KEYBOARD_H

void io_wait(void);
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char val);
int keyboard_has_data(void);
unsigned char keyboard_read(void);
char get_ascii_char(unsigned char scancode);
void mouse_init(void);
int mouse_has_data(void);
unsigned short inw(unsigned short port);
static inline void outw(unsigned short port, unsigned short data) {
    __asm__ __volatile__("outw %0, %1" : : "a"(data), "Nd"(port));
}
void cpu_halt(void);

#endif
