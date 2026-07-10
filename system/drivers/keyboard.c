#include <stdint.h>

extern volatile int has_keyboard_event;
extern volatile uint8_t last_scancode;

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" :: "a"(data), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

// Переносим функцию сброса сюда, чтобы вызвать её без задержек
void sys_reset_direct(void) {
    outb(0xCF9, 0x02);
    __asm__ volatile("outb %%al, $0x80" : : "a"(0)); // io_wait
    outb(0xCF9, 0x06);
    while(1) { __asm__ volatile("hlt"); }
}

void keyboard_handler_c() {
    uint8_t scancode = inb(0x60);

    // КРИТИЧЕСКИЙ ОТРЕЗОК: Если это скан-код нажатия ESC (0x01) — шлем плату в ребут прямо сейчас!
    if (scancode == 0x01) {
        sys_reset_direct();
    }

    last_scancode = scancode;
    has_keyboard_event = 1;

    // Сброс контроллера прерываний PIC
    outb(0x20, 0x20);
    // *(volatile uint32_t*)(0xFEE000B0) = 0;
}
