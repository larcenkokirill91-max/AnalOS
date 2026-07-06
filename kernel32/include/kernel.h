// kernel/include/kernel.h
#pragma once


extern int pitch_dw;
// Системные переменные и типы данных
#include <variable.h>

// Стандартная библиотека и математика
#include <lib.h>
#include <math.h>
#include <time.h>

// Драйверы ядра
#include <drivers/idt.h>
#include <drivers/screen.h>
#include <drivers/keyboard.h>
#include <drivers/window.h>
#include <drivers/font.h>

// Память и диски
#include <memory/disk.h>
#include <memory/fs.h>

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(unsigned short port, unsigned short val) {
    __asm__ __volatile__("outw %0, %1" : : "a"(val), "Nd"(port));
}
static inline unsigned short inw(unsigned short port) {
    unsigned short ret;
    __asm__ __volatile__("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outl(unsigned short port, unsigned int val) {
    __asm__ __volatile__("outl %0, %1" : : "a"(val), "Nd"(port));
}
static inline unsigned int inl(unsigned short port) {
    unsigned int ret;
    __asm__ __volatile__("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void) {
    __asm__ __volatile__("outb %%al, $0x80" : : "a"(0));
}

//заметка:
// hover RGB(45, 165, 255) not hover RGB(0, 120, 212)
