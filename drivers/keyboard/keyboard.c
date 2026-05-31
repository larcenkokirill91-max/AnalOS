#include "keyboard.h"

void io_wait(void) {
    asm volatile("outb %%al, $0x80" : : "a"(0));
}

unsigned char inb(unsigned short port) {
    unsigned char result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outb(unsigned short port, unsigned char val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Проверяет, появились ли данные в буфере (0-й бит порта 0x64)
int keyboard_has_data(void) {
    return (inb(0x64) & 1);
}

// Чистый возврат байта из порта данных 0x60
unsigned char keyboard_read(void) {
    return inb(0x60);
}

char get_ascii_char(unsigned char scancode) {
    static char keymap[] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };
    if (scancode < sizeof(keymap)) {
        return keymap[scancode];
    }
    return 0;
}

