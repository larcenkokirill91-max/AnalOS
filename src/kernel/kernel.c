#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600
#include "../../drivers/screen/screen.h"
#include "../../drivers/keyboard/keyboard.h"
unsigned int screen_pitch = 0;
__attribute__((section(".text.entry")))
void kernel_main(unsigned int fb_address, unsigned int pitch) {
    unsigned char* video_memory = (unsigned char*)fb_address;
    screen_pitch = pitch;
    draw_rect(video_memory, 0, 0, 800, 600, 102, 255, 0);
    draw_rect(video_memory, 350, 250, 100, 100, 0, 255, 0);
    draw_A(video_memory , 392,300, 255, 255, 255);
    draw_B(video_memory , 400,300, 255, 255, 255);
    draw_C(video_memory , 408,300, 255, 255, 255);
    int start_x = 0;
    int start_y = 308;
    int input_buffer = 0;
    unsigned char last_scancode = 0;
    while (1) {
        unsigned char scancode = inb(0x60);
        if (scancode != last_scancode && scancode < 0x80) {
            if (scancode == 30) {
                draw_A(video_memory , start_x, start_y, 255, 255, 255);
                start_x += 8; // Исправлено: теперь координата смещается
                input_buffer++;
            }
            else if (scancode == 46) {
                draw_C(video_memory , start_x, start_y, 255, 255, 255);
                start_x += 8; // Исправлено
                input_buffer++;
            }
            else if (scancode == 48) {
                draw_B(video_memory , start_x, start_y, 255, 255, 255);
                start_x += 8; // Исправлено
                input_buffer++;
            }
            if (input_buffer >= 100) {
                input_buffer = 0;
                start_x = 0;
                start_y += 8;
            }
            last_scancode = scancode; // Лишняя скобка перед этой строкой удалена
        }
        else if (scancode >= 0x80) {
            last_scancode = 0;
        }
    }
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
