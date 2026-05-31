#include "../../drivers/keyboard/keyboard.h"

// ЯВНО ПРЕДУПРЕЖДАЕМ КОМПИЛЯТОР: эти функции лежат в другом месте и скоро будут линковаться
void pic_remap(void);
void pic_enable_keyboard(void);

// Структура одного дескриптора прерывания в архитектуре x86
struct idt_entry_struct {
    unsigned short base_low;  
    unsigned short sel;       
    unsigned char  always0;   
    unsigned char  flags;     
    unsigned short base_high; 
} __attribute__((packed));

// Структура указателя IDTR для инструкции lids
struct idt_ptr_struct {
    unsigned short limit;     
    unsigned int   base;      
} __attribute__((packed));

// Массив дескрипторов и указатель IDTR
struct idt_entry_struct idt[256];
struct idt_ptr_struct   idt_ptr;

extern void idt_load(unsigned int idt_ptr_address);
extern void keyboard_handler_asm(void);
extern void exception_handler_asm(void); // Обработчик ошибок процессора
extern void dummy_handler_asm(void);    // Заглушка для аппаратных IRQ

// Функция заполнения конкретного вектора в таблице IDT
void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

// Главная функция инициализации IDT
void init_idt(void) {
    idt_ptr.limit = (sizeof(struct idt_entry_struct) * 256) - 1;
    idt_ptr.base  = (unsigned int)&idt;

    // Первые 32 вектора отдаем под обработку внутренних исключений ЦП
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (unsigned int)exception_handler_asm, 0x08, 0x8E);
    }

    // Все остальные векторы (32-255) забиваем стандартной аппаратной заглушкой
    for (int i = 32; i < 256; i++) {
        idt_set_gate(i, (unsigned int)dummy_handler_asm, 0x08, 0x8E);
    }

    // Теперь GCC точно знает эти функции! Ремапим PIC (IRQ1 клавиатуры станет вектором 33)
    pic_remap();

    // Ставим обработчик клавиатуры на вектор 33 (0x21)
    idt_set_gate(33, (unsigned int)keyboard_handler_asm, 0x08, 0x8E); 

    // Загружаем IDT в процессор
    idt_load((unsigned int)&idt_ptr);

    // Разрешаем только клавиатуру
    pic_enable_keyboard();
}

