#include <stdint.h>
#include "../include/mouse.h"

int mouse_cycle = 0;
uint8_t mouse_bytes[3];

extern volatile int has_mouse_event;

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" :: "a"(data), "Nd"(port));
}

// ИСПРАВЛЕНО: Безопасное ожидание с таймаутом, чтобы не зависнуть намертво
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

    // Очистим буфер от мусора UEFI, если он там есть
    volatile uint32_t clean = 100;
    while ((inb(0x64) & 1) && --clean) { inb(0x60); }

    mouse_wait_write();
    outb(0x64, 0xA8); // Включить мышь

    mouse_wait_write();
    outb(0x64, 0x20); // Прочитать Comand Byte
    mouse_wait_read();
    status = inb(0x60);

    status |= 2;    // Разрешить прерывания IRQ12 мыши
    status &= ~0x20; // Сбросить бит отключения мыши

    mouse_wait_write();
    outb(0x64, 0x60); // Записать Comand Byte обратно
    mouse_wait_write();
    outb(0x60, status);

    mouse_write(0xF6); // Set default
    mouse_read();

    mouse_write(0xF4); // Enable data reporting
    mouse_read();
}

void mouse_handler_c() {
    uint8_t data = inb(0x60);

    mouse_bytes[mouse_cycle] = data;
    mouse_cycle++;

    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        has_mouse_event = 1;
    }

    // ИСПРАВЛЕНО: Для Slave прерываний (IRQ 8-15) шлем EOI в ОБА контроллера PIC!
    outb(0xA0, 0x20); // Slave PIC
    outb(0x20, 0x20); // Master PIC
}
