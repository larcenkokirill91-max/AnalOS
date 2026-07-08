#include <stdint.h>
#include "../include/keyboard.h"

static inline void outb(unsigned char value, unsigned short port) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

static inline void outw(unsigned short value, unsigned short port) {
    asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline unsigned int inl(unsigned short port) {
    unsigned int value;
    asm volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outl(unsigned int value, unsigned short port) {
    asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

void keyboard_handler_c() {
    *(volatile uint32_t*)(0xFEE000B0) = 0;
}
