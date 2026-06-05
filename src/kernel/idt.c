#include "idt.h"
#include "../../drivers/keyboard/keyboard.h"
#include "../../drivers/screen/screen.h"

struct idt_entry idt[256];

unsigned char current_scancode = 0;

__attribute__((interrupt))
void keyboard_handler(struct interrupt_frame* frame) {
    
    current_scancode = inb(0x60);
    io_wait();
    outb(0x20, 0x20);
}


void default_handler(struct interrupt_frame* frame) {}

void idt_set_gate(int num, unsigned int handler, unsigned short selector, unsigned char flags) {
	idt[num].offset_lower = handler & 0xFFFF;
	idt[num].selector = selector;
	idt[num].zero = 0;
	idt[num].flags = flags;
	idt[num].offset_higher = handler >> 16;
}

void pic_remap(void) {
    // 1. Запускаем инициализацию обоих чипов PIC (команда 0x11)
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();

    // 2. ВАЖНО: Переносим базовые векторы!
    outb(0x21, 0x20); io_wait(); // Мастер-чип теперь начинается с карточки 32 (0x20)
    outb(0xA1, 0x28); io_wait(); // Слейв-чип теперь начинается с карточки 40 (0x28)

    // 3. Рассказываем чипам, как они соединены друг с другом
    outb(0x21, 0x04); io_wait(); // Мастер подключен к Слейву через IRQ 2
    outb(0xA1, 0x02); io_wait(); // Слейв знает, что он ведомый

    // 4. Включаем 32-битный режим 8086 для обоих чипов (команда 0x01)
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();

    // 5. Открываем маски: разрешаем Таймер (IRQ 0), Клавиатуру (IRQ 1) и Каскад (IRQ 2)
    outb(0x21, 0xFD); io_wait(); // Оставляем открытыми биты 0, 1, 2
    outb(0xA1, 0xEF); io_wait(); // Слейв пока полностью маскируем
}


void idt_init(void) {
        pic_remap();
	struct idt_ptr ptr;
	ptr.long_idt = 2047;
	ptr.addres = (unsigned int)&idt;
	for (int num = 0; num < 256; num++) {
		idt_set_gate(num, (unsigned int)default_handler, 0x08, 0x8E);
	}
	idt_set_gate(33, (unsigned int)keyboard_handler, 0x08, 0x8E);
	__asm__ __volatile__("lidt (%0)" : : "r"(&ptr));
	__asm__ __volatile__("sti");
}
