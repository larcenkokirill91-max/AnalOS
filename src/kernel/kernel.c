#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 1024
#include "../../drivers/screen/screen.h"
#include "../../drivers/keyboard/keyboard.h"
#include "../../drivers/disk/disk.h"
#include "../../src/fs/fs.h"

unsigned int screen_pitch = 0;

__attribute__((section(".text.entry")))
void kernel_main(void) {
    // 1. Настройка параметров графики высокого разрешения
    screen_pitch = 5120;
    unsigned char* video_memory = (unsigned char*)0xD0000000;

    // 2. Первичная отрисовка интерфейса операционной системы
    draw_rect(video_memory, 0, 0, 1280, 1024, 69, 178, 253);
    draw_rect(video_memory, 0, 990, SCREEN_WIDTH, 40, 229, 236, 253);
    
    // 3. Копирование суперблока сетевой файловой системы из адреса 0x10200
    struct superblock my_fs;
    unsigned char* src = (unsigned char*)0x10200;
    unsigned char* dest = (unsigned char*)&my_fs;
    
    for (unsigned int i = 0; i < sizeof(struct superblock); i++) {
        dest[i] = src[i];
    }

    // 4. Проверка магии ФС и отрисовка логотипа нужным цветом
    if (my_fs.magic == 0x1337) {
        draw_os(video_memory, 640, 512, 0, 255, 0);     // Зеленый (Успех)
    } else {
        draw_os(video_memory, 640, 512, 255, 0, 0);     // Красный (Ошибка)
    }

    // 5. Объявление всех рабочих переменных ядра
    int start_x = 0;
    int start_y = 0;
    int input_buffer = 0;
    int alt_is_pressed = 0;
    int off_menu_open = 0;
    unsigned char last_scancode = 0;
    
    int mouse_x = 640;
    int mouse_y = 512;
    int mouse_cycle = 0;
    unsigned char mouse_packet[3];

    // Отрисовка курсора в стартовой позиции
    draw_cursor(video_memory, mouse_x, mouse_y);

    // 6. Полная очистка старого буфера клавиатуры ПЕРЕД запуском мыши
    while (keyboard_has_data()) {
        keyboard_read();
        io_wait();
    }

    // 7. Железная активация мыши на чипе контроллера
    mouse_init();

    // 8. ГЛАВНЫЙ БЕСКОНЕЧНЫЙ ЦИКЛ ОПРОСА УСТРОЙСТВ ВВОДА
    while(1) {
        unsigned char status = inb(0x64);
        io_wait();
        
        // Проверяем, прилетел ли вообще хоть какой-то байт в порт 0x60
        if (status & 0x01) {
            unsigned char data = inb(0x60);
            io_wait();
            
            // Если горит 5-й бит статуса — этот байт 100% прислала МЫШЬ
            if (status & 0x20) {
                if (mouse_cycle == 0 && (data & 0x08) == 0) { continue; }
                mouse_packet[mouse_cycle] = data;
                mouse_cycle++;
                
                if (mouse_cycle == 3) {
                    mouse_cycle = 0;
                    
                    // Жесткая проверка фазы пакета (3-й бит обязан быть равен 1)
                    if ((mouse_packet[0] & 0x08) == 0) {
                        continue; 
                    }
                    
                    // Стираем курсор в старой точке
                    undraw_cursor(video_memory, mouse_x, mouse_y);
                    
                    // Извлекаем знаковые смещения
                    signed char move_x = (signed char)mouse_packet[1];
                    signed char move_y = (signed char)mouse_packet[2];
                    
                    mouse_x += move_x;
                    mouse_y -= move_y; // Ось Y инвертирована в стандарте PS/2
                    
                    // Защитные барьеры от вылета за границы экрана 1280x1024
                    if (mouse_x < 0)    mouse_x = 0;
                    if (mouse_x > 1272) mouse_x = 1272;
                    if (mouse_y < 0)    mouse_y = 0;
                    if (mouse_y > 1016) mouse_y = 1016;
                    
                    // Рисуем курсор в новой точке
                    draw_cursor(video_memory, mouse_x, mouse_y);
                    
                    // Моментальное восстановление элементов интерфейса при пересечении
                    if (mouse_y >= 982) {
                        draw_rect(video_memory, 0, 990, SCREEN_WIDTH, 40, 229, 236, 253);
                    }
                    if (mouse_x >= 632 && mouse_x <= 656 && mouse_y >= 504 && mouse_y <= 528) {
                        if (my_fs.magic == 0x1337) {
                            draw_os(video_memory, 640, 512, 0, 255, 0);
                        } else {
                            draw_os(video_memory, 640, 512, 255, 0, 0);
                        }
                    }
                }
            }
            // Если 5-й бит равен 0 — этот байт 100% прислала КЛАВИАТУРА
            else {
                unsigned char scancode = data;
                
                if (scancode == 56) {
                    alt_is_pressed = 1;
                }
                else if (scancode == 184) {
                    alt_is_pressed = 0;
                }
                
                if (scancode != last_scancode && scancode < 0x80) {
                    if (scancode == 30) { 
                        draw_A(video_memory, start_x, start_y, 255, 255, 255);
                        start_x += 16;
                        input_buffer++;
                    }
                    else if (scancode == 46) { 
                        draw_C(video_memory, start_x, start_y, 255, 255, 255);
                        start_x += 16;
                        input_buffer++;
                    }
                    else if (scancode == 48) { 
                        draw_B(video_memory, start_x, start_y, 255, 255, 255);
                        start_x += 16;
                        input_buffer++;
                    }
                    else if (scancode == 62 && alt_is_pressed == 1) { // Alt + F4
                        draw_off(video_memory, 640, 512, 255, 255, 255);
                        off_menu_open = 1;
                    }
                    
                    if (off_menu_open == 1) {
                        if (scancode == 1) { // ESC
                            draw_rect(video_memory, 0, 0, 1280, 1024, 69, 178, 253);
                            draw_rect(video_memory, 0, 990, SCREEN_WIDTH, 40, 229, 236, 253);
                            if (my_fs.magic == 0x1337) draw_os(video_memory, 640, 512, 0, 255, 0);
                            else draw_os(video_memory, 640, 512, 255, 0, 0);
                            off_menu_open = 0;
                        }
                        else if (scancode == 28) { // Enter
                            while (inb(0x64) & 2) { io_wait(); }
                            outb(0x64, 0xFE); 
                        }
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
        }
        io_wait(); 
    }
}
