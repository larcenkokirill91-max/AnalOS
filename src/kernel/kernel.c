#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 1024
#define WIN_HH 25
#define WIN_WW 5
#include "../../drivers/screen/screen.h"
#include "../../drivers/keyboard/keyboard.h"
#include "../../drivers/disk/disk.h"
#include "variable.h"
#include "../../drivers/screen/font.h"
#include "../fs/fs.h"
#include "window.h"
#include "../math.h"
#include "time.h"
#include "../start_menu.h"
#include "idt.h"
#include "../../lib/lib.h"

static inline void outl(unsigned short port, unsigned int val) {
    __asm__ __volatile__("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned int inl(unsigned short port) {
    unsigned int ret;
    __asm__ __volatile__("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void (*draw_digit[10])(unsigned char*, int, int, unsigned char, unsigned char, unsigned char) = {
        draw_zero, draw_one, draw_two, draw_three, draw_four,
        draw_five, draw_six, draw_seven, draw_eight, draw_nine
};

void draw_time(unsigned char* video_memory) {
        get_rtc_time(&h, &m, &s);
        if (m != last_m || h != last_h) {
                last_m = m;
                last_h = h;
                int fh = floor_div(h, 10);
                int lh = tenth_digit(h, 10);
                int fm = floor_div(m, 10);
                int lm = tenth_digit(m, 10);
                draw_rect(video_memory, 1180, 1000, 20, 20, 229, 236, 253); 
                draw_digit[fh](video_memory, 1180, 1000, 0, 0, 0);
                draw_rect(video_memory, 1196, 1000, 20, 20, 229, 236, 253); 
                draw_digit[lh](video_memory, 1196, 1000, 0, 0, 0);
                draw_rect(video_memory, 1212, 1002, 4, 4, 0, 0, 0);
                draw_rect(video_memory, 1212, 1012, 4, 4, 0, 0, 0);
                draw_rect(video_memory, 1216, 1000, 20, 20, 229, 236, 253); 
                draw_digit[fm](video_memory, 1216, 1000, 0, 0, 0);
                draw_rect(video_memory, 1232, 1000, 20, 20, 229, 236, 253); 
                draw_digit[lm](video_memory, 1232, 1000, 0, 0, 0);
        }
}

void kernel_main(void);
extern unsigned char current_scancode;
unsigned int screen_pitch = 5120;
void sys_shutdown(void) {
    // 1. Жёсткое выключение по спецификации Intel ICH7/ICH8 через PCI порты
    // Задаем адрес регистра управления питанием в Южном мосту
    outl(0xCF8, 0x8000F840); 
    io_wait();
    
    // Считываем базовый адрес портов управления питанием (PMBASE)
    unsigned int pmbase = inl(0xCFC) & 0xFFFE;
    io_wait();
    
    if (pmbase != 0) {
        // Вычисляем точный адрес порта PM1_CNT (PMBASE + 0x04)
        unsigned short pm1_cnt_port = (unsigned short)(pmbase + 0x04);
        
        // Маска S5 (Power Off) для чипсетов Intel: взводим 10, 11, 12 биты (состояние S5) 
        // и 13 бит (SLP_EN - запустить укладывание спать)
        // Число 0x3C00 - это чистый, железобетонный канон Intel для полного отключения БП!
        outw(pm1_cnt_port, 0x3C00);
        io_wait();
        
        // Подстраховка: если плата требует старый формат маски
        outw(pm1_cnt_port, 0x2000 | 0x1C00);
        io_wait();
    }

    // 2. Если PCI-метод не сработал, бьем по APM контроллеру питания
    outw(0xB2, 0x07);
    io_wait();
    
    // На случай, если тестируешь в эмуляторе
    outw(0x604, 0x2000);
    io_wait();

    // Финальный барьер останова процессора
    __asm__ __volatile__("cli; hlt");
}


void render(unsigned char* video_memory, struct superblock* fs) {
	draw_rect(video_memory, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 69, 178, 253);
	draw_rect(video_memory, 0, 990, SCREEN_WIDTH, 40, 229, 236, 253);
	draw_start(video_memory, 650, 995);
	if (fs->magic == 0x1337) {
		draw_os(video_memory, 640, 512, 0, 0, 0);     
	} else {
		draw_os(video_memory, 640, 512, 255, 0, 0);
	}
}

__attribute__((section(".text.entry")))
void kernel_main(void) {
	unsigned char* video_memory = (unsigned char*)0xD0000000; 
	render(video_memory, (struct superblock*)0x15800);
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			unsigned int* vram32 = (unsigned int*)video_memory;
			int screen_offset = ((mouse_y + y) * (screen_pitch / 4)) + (mouse_x + x);
			mouse_bg_buffer[y * 16 + x] = vram32[screen_offset];
		}
	}
	struct window win[3];
        win[0].lt.x = 450;  win[0].lt.y = 400;
        win[0].width = 450; win[0].height = 200;
        win[1].lt.x = 450;  win[1].lt.y = 300;
        win[1].width = 400; win[1].height = 250;
        win[2].lt.x = 900;  win[2].lt.y = 600;
        win[2].width = 300; win[2].height = 150;
	draw_cursor(video_memory, mouse_x, mouse_y);
	unsigned char selected_option;
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
					if ((mouse_packet[0] & 0x08) == 0) continue;
					for (int x = 0; x < 16; x++) {
						for (int y = 0; y < 16; y++) {
							unsigned int* vram32 = (unsigned int*)video_memory;
							int screen_offset = ((mouse_y + y) * (screen_pitch / 4)) + (mouse_x + x);
							vram32[screen_offset] = mouse_bg_buffer[y * 16 + x]; 
						}
					}
					signed char move_x = mouse_packet[1];
					signed char move_y = mouse_packet[2];
					mouse_x += move_x;
					mouse_y -= move_y;
					if (mouse_x < 0)    mouse_x = 0;
					if (mouse_x > 1264) mouse_x = 1264;
					if (mouse_y < 0)    mouse_y = 0;
					if (mouse_y > 1008) mouse_y = 1008;
					for (int x = 0; x < 16; x++) {
						for (int y = 0; y < 16; y++) {
							unsigned int* vram32 = (unsigned int*)video_memory;
							int screen_offset = ((mouse_y + y) * (screen_pitch / 4)) + (mouse_x + x);
							mouse_bg_buffer[y * 16 + x] = vram32[screen_offset]; 
						}
					}
					draw_cursor(video_memory, mouse_x, mouse_y);
				}
			}
		} if (current_scancode != 0) {
			data = current_scancode;
			current_scancode = 0;
			if (data == 59) {
				draw_os(video_memory, 640, 512, 0, 0, 255);
				install_to_disk();
				draw_os(video_memory, 640, 512, 255, 0, 0);
			} else if (data == 56) {
				alt_pressed = 1;
			} else if (data == 184) {
				alt_pressed = 0;
			} if (data == 62 && alt_pressed == 1) {
				menu_open = 1;
				selected_option = 0;
				draw_window(video_memory, &win[0]);
				draw_off_menu(video_memory);
				draw_rect(video_memory, 500, 565, 100, 4, 0, 120, 212);
			} if (menu_open == 1) {
				if (data == 28) {
                                        if (selected_option == 1) {
                                                sys_shutdown();
                                        } else {
                                                while (inb(0x64) & 2) { io_wait(); }
                                                outb(0x64, 0xFE);
                                        }
				} else if (data == 1) {
					menu_open = 0;
					render(video_memory, (struct superblock*)0x15800);
					draw_cursor(video_memory, mouse_x, mouse_y);
				} else if (data == 77) {
				        selected_option = 1;
					draw_rect(video_memory, 500, 565, 100, 4, 255, 255, 255);
					draw_rect(video_memory, 750, 565, 100, 4, 0, 120, 212);
				} else if (data == 75) {
				        selected_option = 0;
					draw_rect(video_memory, 750, 565, 100, 4, 255, 255, 255);
					draw_rect(video_memory, 500, 565, 100, 4, 0, 120, 212);
				}
			}
		}
		draw_time(video_memory);
		io_wait(); 
	}
}
