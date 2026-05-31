#define SCREEN_WIDTH  1280  
#define SCREEN_HEIGHT 1024  
#include "../../drivers/screen/screen.h"
#include "../../drivers/keyboard/keyboard.h"

unsigned int screen_pitch = 0;

__attribute__((section(".text.entry")))
void kernel_main(void) { // Убрали аргументы из скобок!
    
    // ЖЕЛЕЗНО ИСПРАВЛЕНО ДЛЯ LAN: Зашиваем параметры SyncMaster напрямую в код ядра!
    screen_pitch = 5120; // Точный шаг строки для режима 1280x1024 на встроенном видео Intel
    
    // Стандартный базовый адрес графики Intel тех лет (обычно 0xE0000000 или 0xD0000000)
    // Мы берем его из безопасной области, которую выставил чипсет
    unsigned char* video_memory = (unsigned char*)0xD0000000; 
    
    // Рисуем интерфейс AnalOS 
    draw_rect(video_memory, 0, 0, 1280, 1024, 102, 255, 0); // Зеленый фон
    draw_rect(video_memory, 590, 462, 100, 100, 0, 255, 0); // Синий квадрат
    
    draw_A(video_memory , 632, 512, 255, 255, 255);
    draw_B(video_memory , 640, 512, 255, 255, 255);
    draw_C(video_memory , 648, 512, 255, 255, 255);
    
    int start_x = 0;
    int start_y = 540; // Сместили пониже, чтобы буквы не улетали вверх
    int input_buffer = 0;
    unsigned char last_scancode = 0;
    
    while (keyboard_has_data()) { 
        keyboard_read(); 
        io_wait(); 
    }
    
    while (1) {
        if (keyboard_has_data()) {
            unsigned char scancode = keyboard_read();
            io_wait();

            if (scancode != last_scancode && scancode < 0x80) {
                if (scancode == 30) { 
                    draw_A(video_memory , start_x, start_y, 255, 255, 255);
                    start_x += 16;
                    input_buffer++;
                }
                else if (scancode == 46) { 
                    draw_C(video_memory , start_x, start_y, 255, 255, 255);
                    start_x += 16;
                    input_buffer++;
                }
                else if (scancode == 48) { 
                    draw_B(video_memory , start_x, start_y, 255, 255, 255);
                    start_x += 16;
                    input_buffer++;
                }
                
                if (input_buffer >= 80) {
                    input_buffer = 0;
                    start_x = 0;
                    start_y += 16;
                }
                last_scancode = scancode;
            }
            else if (scancode >= 0x80) {
                last_scancode = 0;
            }
        }
        io_wait(); 
    }
}

