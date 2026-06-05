#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 1024
#define WIN_HH 25
#define WIN_WW 5
#include "../../drivers/screen/screen.h"
#include "../../drivers/keyboard/keyboard.h"
#include "../../drivers/disk/disk.h"
#include "../fs/fs.h"
#include "window.h"
#include "idt.h"
extern unsigned char current_scancode;
unsigned int screen_pitch = 5120;
void render(unsigned char* video_memory, struct superblock* fs) {
	draw_rect(video_memory, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 69, 178, 253);
	draw_rect(video_memory, 0, 990, SCREEN_WIDTH, 40, 229, 236, 253);

	if (fs->magic == 0x1337) {
		draw_os(video_memory, 640, 512, 0, 255, 0);     // Зеленый (Успех)
	} else {
		draw_os(video_memory, 640, 512, 255, 0, 0);
	}
}
__attribute__((section(".text.entry")))
void kernel_main(void) {
    unsigned char* video_memory = (unsigned char*)0xD0000000; 
    int alt_pressed = 0;
    int menu_open = 0;
    int mouse_x = 640;
    int mouse_y = 512;
    int mouse_cycle = 0;
    signed char mouse_packet[3];
    unsigned char data = 0;
    struct window win;
    win.lt.x = 25;  win.lt.y = 25;
    win.rt.x = 525; win.rt.y = 25;
    win.lb.x = 25;  win.lb.y = 225;
    win.width = 500;
    win.height = 200;
    unsigned int mouse_bg_buffer[64];
    render(video_memory, (struct superblock*)0x15800);
    draw_window(video_memory, &win);
    for (int x = 0; x < 8; x++) {
    	for (int y = 0; y < 8; y++) {
        	unsigned int* vram32 = (unsigned int*)video_memory;
                int screen_offset = ((mouse_y + y) * (screen_pitch / 4)) + (mouse_x + x);
                mouse_bg_buffer[y * 8 + x] = vram32[screen_offset];
	}
    }
    draw_cursor(video_memory, mouse_x, mouse_y);
    mouse_init();
    idt_init();
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
                    if ((mouse_packet[0] & 0x08) == 0) { continue; }
		    for (int x = 0; x < 8; x++) {
			for (int y = 0; y < 8; y++) {
			    unsigned int* vram32 = (unsigned int*)video_memory;
			    int screen_offset = ((mouse_y + y) * (screen_pitch / 4)) + (mouse_x + x);
			    vram32[screen_offset] = mouse_bg_buffer[y * 8 + x]; 
			}
		    }
                    signed char move_x = mouse_packet[1];
                    signed char move_y = mouse_packet[2];
                    mouse_x += move_x;
                    mouse_y -= move_y;
                    if (mouse_x < 0)    mouse_x = 0;
                    if (mouse_x > 1272) mouse_x = 1272;
                    if (mouse_y < 0)    mouse_y = 0;
                    if (mouse_y > 1016) mouse_y = 1016;
		    for (int x = 0; x < 8; x++) {
 			for (int y = 0; y < 8; y++) {
				unsigned int* vram32 = (unsigned int*)video_memory;
				int screen_offset = ((mouse_y + y) * (screen_pitch / 4)) + (mouse_x + x);
				mouse_bg_buffer[y * 8 + x] = vram32[screen_offset]; 
			}
		    }
                    draw_cursor(video_memory, mouse_x, mouse_y);
                }
            }
        } if (current_scancode != 0) {
	    data = current_scancode;
            current_scancode = 0;
            if (data == 56) {
                alt_pressed = 1;
            } else if (data == 184) {
                alt_pressed = 0;
            } if (data == 62 && alt_pressed == 1) {
                menu_open = 1;
                draw_rect(video_memory, 640, 512, 50, 50, 255, 255, 255);
            } if (menu_open == 1) {
                if (data == 28) {
                    while (inb(0x64) & 2) { io_wait(); }
                    outb(0x64, 0xFE);
                } else if (data == 1) {
                    menu_open = 0;
                    render(video_memory, (struct superblock*)0x15800);
                    draw_window(video_memory, &win);
                    draw_cursor(video_memory, mouse_x, mouse_y);
                }
            }
	}
        io_wait(); 
    }
}

