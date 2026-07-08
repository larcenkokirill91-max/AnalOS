#include <stdint.h>
#include "../include/idt.h"

struct IDTEntry {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t  ist;
    uint8_t  attributes;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct IDTPointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct IDTEntry idt[256];

// Объявляем ВСЕ внешние ассемблерные функции
extern void dummy_handler_asm(void);
extern void mouse_handler_asm(void);
extern void keyboard_handler_asm(void);

void init_idt() {
    uint64_t dummy_address = (uint64_t)dummy_handler_asm;

    // 1. Заполняем всю таблицу безопасными заглушками
    for(int i = 0; i < 256; i++) {
        idt[i].offset_low  = (uint16_t)(dummy_address & 0xFFFF);
        idt[i].offset_mid  = (uint16_t)((dummy_address >> 16) & 0xFFFF);
        idt[i].offset_high = (uint32_t)((dummy_address >> 32) & 0xFFFFFFFF);
        idt[i].segment_selector = 0x38; // Селектор UEFI из дампа
        idt[i].ist = 0;
        idt[i].attributes = 0x8E;
        idt[i].reserved = 0;
    }

    // 2. Настраиваем клавиатуру (Вектор 33)
    uint64_t address = (uint64_t)keyboard_handler_asm;
    idt[33].offset_low  = (uint16_t)(address & 0xFFFF);
    idt[33].offset_mid  = (uint16_t)((address >> 16) & 0xFFFF);
    idt[33].offset_high = (uint32_t)((address >> 32) & 0xFFFFFFFF);
    idt[33].segment_selector = 0x38; // МЕНЯЕМ С 0x08 НА 0x38!
    idt[33].ist = 0;
    idt[33].attributes = 0x8E;
    idt[33].reserved = 0;

    // 3. Настраиваем мышь (Вектор 44)
    uint64_t mouse_address = (uint64_t)mouse_handler_asm;
    idt[44].offset_low  = (uint16_t)(mouse_address & 0xFFFF);
    idt[44].offset_mid  = (uint16_t)((mouse_address >> 16) & 0xFFFF);
    idt[44].offset_high = (uint32_t)((mouse_address >> 32) & 0xFFFFFFFF);
    idt[44].segment_selector = 0x38; // МЕНЯЕМ С 0x08 НА 0x38!
    idt[44].ist = 0;
    idt[44].attributes = 0x8E;
    idt[44].reserved = 0;

    // 4. Загружаем указатель таблицы в процессор
    struct IDTPointer idtr;
    idtr.limit = (sizeof(struct IDTEntry) * 256) - 1;
    idtr.base = (uint64_t)&idt;

    __asm__ volatile("lidt %0" :: "m"(idtr));
    __asm__ volatile("sti"); // Включаем прерывания
}
