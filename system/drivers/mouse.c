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

    // Проверяем, есть ли вообще данные в буфере PS/2
    if ((status & 0x01) == 0) {
        // ОБЯЗАТЕЛЬНО: перед выходом отправляем EOI, если прерывание сработало,
        // иначе контроллер прерываний заблокирует систему.
        // Если у вас APIC: *(volatile uint32_t*)0xFEE000B0 = 0;
        // Если у вас PIC (как на вашем первом скрине):
        outb(0xA0, 0x20);
        outb(0x20, 0x20);
        return;
    }

    // Читаем байт МЫШИ. Мы ОБЯЗАНЫ его вычитать, чтобы очистить буфер порта 0x60!
    uint8_t data = inb(0x60);

    // Синхронизация: первый байт пакета мыши ВСЕГДА должен иметь бит 3 (0x08)
    if (mouse_cycle == 0 && !(data & 0x08)) {
        // Если фаза сбилась, мы всё равно отправляем EOI и выходим, готовые к следующему байту
        outb(0xA0, 0x20);
        outb(0x20, 0x20);
        return;
    }

    mouse_bytes[mouse_cycle] = data;
    mouse_cycle++;

    // Когда собрали 3 байта стандартного пакета PS/2
    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        has_mouse_event = 1;

        int move_x = mouse_bytes[1];
        int move_y = mouse_bytes[2];

        // Восстановление знака для относительного смещения
        if (mouse_bytes[0] & 0x10) move_x |= ~0xFF;
        if (mouse_bytes[0] & 0x20) move_y |= ~0xFF;

        // Обновляем координаты на экране
        mouse_x += move_x;
        mouse_y -= move_y;

        // Ограничиваем курсор размерами вашего QEMU (1024x768)
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1024 - 32) mouse_x = 1024 - 32;
        if (mouse_y > 768 - 32)  mouse_y = 768 - 32;

        // Отрисовка курсора (ваша C++ функция)
        draw_mouse(mouse_x, mouse_y, 32, 255);
    }

    // ОБЯЗАТЕЛЬНО: Отправляем сигнал End of Interrupt (EOI) в самом конце обработчика!
    // Если вы используете современный APIC, замените эти две строчки на:
    // volatile uint32_t* lapic_eoi = (volatile uint32_t*)0xFEE000B0;
    // *lapic_eoi = 0;
    outb(0xA0, 0x20); // Slave PIC
    outb(0x20, 0x20); // Master PIC
}
