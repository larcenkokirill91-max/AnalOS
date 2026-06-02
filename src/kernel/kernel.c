#define SCREEN_WIDTH  1280  
#define SCREEN_HEIGHT 1024  
#include "../../drivers/screen/screen.h"
#include "../../drivers/keyboard/keyboard.h"
unsigned int screen_pitch = 0;
__attribute__((section(".text.entry")))
void kernel_main(void) {
	screen_pitch = 5120;
	unsigned char* video_memory = (unsigned char*)0xD0000000;
	mouse_init();
	draw_rect(video_memory, 0, 0, 1280, 1024, 69, 178, 253);
	draw_rect(video_memory, 0, 990, SCREEN_WIDTH, 40, 229, 236, 253);
	draw_os(video_memory , 640, 512, 255, 255, 255);
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
	unsigned char data = inb(0x60);
	unsigned char scancode = data;
	while (keyboard_has_data()) { 
		keyboard_read(); 
		io_wait(); 
	}
	while(1) {
		unsigned char status = inb(0x64);
		io_wait();
		if (status & 1) {
			data = inb(0x60);
			scancode = data;
			if (status & 0x20) {
				mouse_packet[mouse_cycle] = data;
				mouse_cycle++;
				if (mouse_cycle == 3) {
					if ((mouse_packet[0] & 0x08) == 0) {
						mouse_cycle = 0;
						continue;
					}
					draw_rect(video_memory, mouse_x, mouse_y, 8, 8, 69, 178, 253);
					mouse_cycle = 0;
					signed char move_x = (signed char) mouse_packet[1];
					signed char move_y = (signed char) mouse_packet[2];
					mouse_x += move_x;
					mouse_y -= move_y;
					if (mouse_x < 0) {
						mouse_x = 0;
					}
					if (mouse_x > 1272) {
						mouse_x = 1272;
					}
                                        if (mouse_y < 0) {
                                                mouse_y = 0;
                                        }
                                        if (mouse_y > 1016) {
                                                mouse_y = 1016;
                                        }
					draw_rect(video_memory, mouse_x, mouse_y, 8, 8, 255, 255, 255);
					draw_rect(video_memory, 0, 990, SCREEN_WIDTH, 40, 229, 236, 253);
					draw_os(video_memory , 640, 512, 255, 255, 255);
				}
			}
			else {
				if (scancode == 56) {
					alt_is_pressed = 1;
				}
				else if (scancode == 184) {
					alt_is_pressed = 0;
				}
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
					else if (scancode == 62 && alt_is_pressed == 1) {
						draw_off(video_memory , 640, 512, 255, 255, 255);
						off_menu_open = 1;
					}
					if (off_menu_open == 1) {
						if (scancode == 1) {
							draw_rect(video_memory, 0, 0, 1280, 1024, 69, 178, 253);
							draw_rect(video_memory, 0, 990, SCREEN_WIDTH, 40, 229, 236, 253);
							draw_os(video_memory, 640, 512, 255, 255, 255);
							off_menu_open = 0;
						}
						else if (scancode == 28) {
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
			io_wait(); 
		}
	}
}
