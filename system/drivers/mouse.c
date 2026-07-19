#include <stdint.h>
#include "../include/mouse.h"

// Объявляем внешние переменные, чтобы компилятор не ругался
extern volatile int has_keyboard_event;
extern volatile int has_mouse_event;
extern volatile uint8_t last_scancode;

int mouse_cycle = 0;
int mouse_x = 512;
int mouse_y = 384;
uint8_t mouse_bytes[4];

void draw_mouse(int x, int y, int size, int a);

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" :: "a"(data), "Nd"(port));
}

// Микропауза для шины ввода-вывода
static inline void io_wait(void) {
    outb(0x80, 0);
}

void mouse_wait_write() {
    // Таймаут увеличен до миллиона циклов с микропаузами
    volatile uint32_t timeout = 1000000;
    while ((inb(0x64) & 2) != 0 && --timeout) {
        io_wait();
    }
}

void mouse_wait_read() {
    volatile uint32_t timeout = 1000000;
    while ((inb(0x64) & 1) == 0 && --timeout) {
        io_wait();
    }
}

void mouse_write(uint8_t data) {
    mouse_wait_write();
    outb(0x64, 0xD4); // Сигнал контроллеру: пишем мыши
    mouse_wait_write();
    outb(0x60, data);
}

uint8_t mouse_read() {
    mouse_wait_read();
    return inb(0x60);
}

void init_mouse() {
    uint8_t status;

    // Очищаем буфер от старого мусора перед инициализацией
    volatile uint32_t clean = 100;
    while ((inb(0x64) & 1) && --clean) {
        inb(0x60);
        io_wait();
    }

    mouse_wait_write();
    outb(0x64, 0xA8); // Включить вспомогательный порт мыши

    mouse_wait_write();
    outb(0x64, 0x20); // Прочитать Command Byte контроллера PS/2
    mouse_wait_read();
    status = inb(0x60);

    status |= 2;     // Разрешить прерывания мыши (IRQ 12)
    status &= ~0x20; // Убедиться, что порт мыши НЕ заблокирован

    mouse_wait_write();
    outb(0x64, 0x60); // Записать Command Byte обратно
    mouse_wait_write();
    outb(0x60, status);

    // Инициализация самой мыши дефолтными настройками
    mouse_write(0xF6); // Set Defaults
    mouse_read();      // Читаем ACK (0xFA)

    // Включаем передачу пакетов при движении!
    mouse_write(0xF4); // Enable Data Reporting
    mouse_read();      // Читаем ACK (0xFA)
}

void mouse_handler_c() {
    uint8_t status = inb(0x64);

    // Проверяем, есть ли данные и принадлежат ли они мыши (бит 5 порта 0x64)
    if ((status & 0x01) == 0 || (status & 0x20) == 0) {
        outb(0xA0, 0x20);
        outb(0x20, 0x20);
        return;
    }

    uint8_t data = inb(0x60);

    // Синхронизация
    if (mouse_cycle == 0 && !(data & 0x08)) {
        // Игнорируем байт, так как это не начало пакета
        outb(0xA0, 0x20);
        outb(0x20, 0x20);
        return;
    }

    mouse_bytes[mouse_cycle] = data;
    mouse_cycle++;

    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        has_mouse_event = 1;

        // Автоматическое приведение типов через знаковый байт (int8_t)
        int move_x = (int32_t)((int8_t)mouse_bytes[1]);
        int move_y = (int32_t)((int8_t)mouse_bytes[2]);

        // Обновляем координаты ОС
        mouse_x += move_x;
        mouse_y -= move_y; // Инверсия Y для PS/2 экрана

        // Ограничители экрана (1024x768)
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1024 - 32) mouse_x = 1024 - 32;
        if (mouse_y > 768 - 32)  mouse_y = 768 - 32;

        // Отрисовка курсора
        draw_mouse(mouse_x, mouse_y, 32, 255);
    }

    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
