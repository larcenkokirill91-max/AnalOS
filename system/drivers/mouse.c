#include <stdint.h>
#include "../include/mouse.h"

int mouse_cycle = 0;
int mouse_x = 0, mouse_y = 0;
uint8_t mouse_bytes[4];

extern volatile int has_mouse_event;

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" :: "a"(data), "Nd"(port));
}

void mouse_wait_write() {
    volatile uint32_t timeout = 100000;
    while ((inb(0x64) & 2) != 0 && --timeout);
}

void mouse_wait_read() {
    volatile uint32_t timeout = 100000;
    while ((inb(0x64) & 1) == 0 && --timeout);
}

void mouse_write(uint8_t data) {
    mouse_wait_write();
    outb(0x64, 0xD4);
    mouse_wait_write();
    outb(0x60, data);
}

uint8_t mouse_read() {
    mouse_wait_read();
    return inb(0x60);
}

void init_mouse() {
    uint8_t status;

    volatile uint32_t clean = 100;
    while ((inb(0x64) & 1) && --clean) { inb(0x60); }

    mouse_wait_write();
    outb(0x64, 0xA8); // Включить мышь

    mouse_wait_write();
    outb(0x64, 0x20); // Прочитать Comand Byte
    mouse_wait_read();
    status = inb(0x60);

    status |= 2;    // Разрешить прерывания мыши
    status &= ~0x20; // Сбросить бит

    mouse_wait_write();
    outb(0x64, 0x60); // Записать Comand Byte обратно
    mouse_wait_write();
    outb(0x60, status);

    mouse_write(0xF6);
    mouse_read();

    mouse_write(0xF4);
    mouse_read();
}

void mouse_handler_c() {
    uint8_t data = inb(0x60);

    mouse_bytes[mouse_cycle] = data;
    mouse_cycle++;

    if (mouse_cycle == 3) {
        undraw_mouse(mouse_x, mouse_y);
        mouse_cycle = 0;
        has_mouse_event = 1;

        mouse_bytes[1] = int move_x;
        mouse_bytes[2] = int move_y;
        mouse_x += move_x;
        mouse_y -= move_y;

        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1280 - 16) mouse_x = 1280 - 16;
        if (mouse_y > 1024 - 16) mouse_y = 1024 - 16;
        
        draw_mouse(mouse_x, mouse_y);
    }

    outb(0xA0, 0x20); // Slave
    outb(0x20, 0x20); // Master
}
