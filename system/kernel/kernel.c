#include "../include/kernel.h"

void init_screen_driver(BootInfo* info);
void fill_screen(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void draw_taskbar(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int rad, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void swap_buffers(void* gop);

void __attribute__((ms_abi)) kernel_main(BootInfo* info) {
    if (!info) {
        while(1) { __asm__ __volatile__("hlt"); }
    }

    init_screen_driver(info);
    init_idt();
    init_ioapic();
    init_mouse();
    fill_screen(20, 30, 50, 255);
    draw_taskbar(50, 700, 923, 40, 8, 255, 255, 255, 200);

    swap_buffers(0);

    volatile int keep_running = 1;
    while (keep_running) {
        __asm__ __volatile__("hlt");
    }
}
