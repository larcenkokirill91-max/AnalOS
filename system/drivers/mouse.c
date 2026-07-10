#include <stdint.h>
#include "../include/mouse.h"

// Глобальные переменные для сборки пакета данных от мыши
int mouse_cycle = 0;
uint8_t mouse_bytes[3]; // Массив строго на 3 байта

// Связываем флаг события мыши с главным циклом ядра в kernel.c
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
    while ((inb(0x64) & 2) != 0);
}

void mouse_wait_read() {
    while ((inb(0x64) & 1) == 0);
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

    mouse_wait_write();
    outb(0x64, 0xA8);

    mouse_wait_write();
    outb(0x64, 0x20);
    mouse_wait_read();
    status = inb(0x60);

    status |= 2;

    mouse_wait_write();
    outb(0x64, 0x60);
    mouse_wait_write();
    outb(0x60, status);

    mouse_write(0xF6);
    mouse_read();

    mouse_write(0xF4);
    mouse_read();
}

void mouse_handler_c() {
    uint8_t data = inb(0x60);

    // Записываем пришедший байт в наш буфер пакета
    mouse_bytes[mouse_cycle] = data;
    mouse_cycle++;

    // Мышь PS/2 присылает данные пакетами строго по 3 байта
    if (mouse_cycle == 3) {
        mouse_cycle = 0; // Сбрасываем счетчик для следующего движения

        // Сигнализируем главному циклу ядра в kernel.c, что пакет мыши собран!
        has_mouse_event = 1;
    }

    // Сброс для старого контроллера прерываний PIC (Slave PIC, так как мышь на IRQ12)
    outb(0xA0, 0x20);
    outb(0x20, 0x20);

    // Сброс для локального APIC процессора
    // *(volatile uint32_t*)(0xFEE000B0) = 0;
}
