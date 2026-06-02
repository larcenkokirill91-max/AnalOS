#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 1024
#include "../../drivers/screen/screen.h"
#include "../../drivers/keyboard/keyboard.h"
#include "../../drivers/disk/disk.h"
#include "../../src/fs/fs.h"
unsigned int screen_pitch = 0;
void render(unsigned char* video_memory, struct superblock* fs) {
    draw_rect(video_memory, 0, 0, 1280, 1024, 69, 178, 253);
    draw_rect(video_memory, 0, 990, SCREEN_WIDTH, 40, 229, 236, 253);
    draw_circle(video_memory, 640, 512, 100, 255, 255, 255);
    if (fs->magic == 0x1337) {
        draw_os(video_memory, 640, 512, 0, 255, 0);
    } else {
        draw_os(video_memory, 640, 512, 255, 0, 0);
    }
}
__attribute__((section(".text.entry")))
void kernel_main(void) {
    screen_pitch = 5120;
    unsigned char* video_memory = (unsigned char*)0xD0000000; 
    struct superblock my_fs;
    unsigned char* src = (unsigned char*)0x15800;
    unsigned char* dest = (unsigned char*)&my_fs;
    for (unsigned int i = 0; i < sizeof(struct superblock); i++) {
        dest[i] = src[i];
    }
    int start_x = 20;
    int start_y = 20;
    int input_buffer = 0;
    int alt_is_pressed = 0;
    int off_menu_open = 0;
    unsigned char last_scancode = 0;
    int mouse_x = 640;
    int mouse_y = 512;
    int mouse_cycle = 0;
    unsigned char mouse_packet[3];
    unsigned char data = 0;
    unsigned int mouse_bg_buffer[64];
    render(video_memory, &my_fs);
    unsigned int* vram32 = (unsigned int*)video_memory;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int screen_offset = ((mouse_y + y) * (screen_pitch / 4)) + (mouse_x + x);
            mouse_bg_buffer[y * 8 + x] = vram32[screen_offset];
        }
    }
    draw_cursor(video_memory, mouse_x, mouse_y);
    while (keyboard_has_data()) {
        keyboard_read();
        io_wait();
    }
    mouse_init();
    while(1) {
        unsigned char status = inb(0x64);
        io_wait();
        if (status & 0x01) {
            data = inb(0x60);
            io_wait();
            if (status & 0x20) {
                if (mouse_cycle == 0 && (data & 0x08) == 0) { continue; }
                mouse_packet[mouse_cycle] = data;
                mouse_cycle++;
                if (mouse_cycle == 3) {
                    mouse_cycle = 0;
                    if ((mouse_packet[0] & 0x08) == 0) {
                        continue; 
                    }
                    vram32 = (unsigned int*)video_memory;
                    for (int y = 0; y < 8; y++) {
                        for (int x = 0; x < 8; x++) {
                            int screen_offset = ((mouse_y + y) * (screen_pitch / 4)) + (mouse_x + x);
                            vram32[screen_offset] = mouse_bg_buffer[y * 8 + x];
                        }
                    }
                    signed char move_x = (signed char)mouse_packet[1];
                    signed char move_y = (signed char)mouse_packet[2];
                    mouse_x += move_x;
                    mouse_y -= move_y;
                    if (mouse_x < 0)    mouse_x = 0;
                    if (mouse_x > 1272) mouse_x = 1272;
                    if (mouse_y < 0)    mouse_y = 0;
                    if (mouse_y > 1016) mouse_y = 1016;
                    for (int y = 0; y < 8; y++) {
                        for (int x = 0; x < 8; x++) {
                            int screen_offset = ((mouse_y + y) * (screen_pitch / 4)) + (mouse_x + x);
                            mouse_packet[0] = 0;
                            mouse_bg_buffer[y * 8 + x] = vram32[screen_offset];
                        }
                    }
                    draw_cursor(video_memory, mouse_x, mouse_y);
                }
            } else {
                unsigned char scancode = data;
                if (scancode == 56)       alt_is_pressed = 1;
                else if (scancode == 184) alt_is_pressed = 0;
                if (scancode != last_scancode && scancode < 0x80) {
                    if (scancode == 30) { 
                        draw_A(video_memory, start_x, start_y, 255, 255, 255);
                        start_x += 16; input_buffer++;
                    }
                    else if (scancode == 46) { 
                        draw_C(video_memory, start_x, start_y, 255, 255, 255);
                        start_x += 16; input_buffer++;
                    }
                    else if (scancode == 48) { 
                        draw_B(video_memory, start_x, start_y, 255, 255, 255);
                        start_x += 16; input_buffer++;
                    }
                    else if (scancode == 62 && alt_is_pressed == 1) { // Alt + F4
                        draw_off(video_memory, 640, 512, 255, 255, 255);
                        off_menu_open = 1;
                    }
                    if (off_menu_open == 1) {
                        if (scancode == 1) { // ESC
                            off_menu_open = 0;
                            render(video_memory, &my_fs);
                        }
                        else if (scancode == 28) { // Enter (Перезагрузка)
                            while (inb(0x64) & 2) { io_wait(); }
                            outb(0x64, 0xFE); 
                        }
                    }
                    if (input_buffer >= 80) {
                        input_buffer = 0;
                        start_x = 20;
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
