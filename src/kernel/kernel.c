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
    outl(0xCF8, 0x8000F840); 
    io_wait();
    unsigned int pmbase = inl(0xCFC) & 0xFFFE;
    io_wait();
    if (pmbase != 0) {
        unsigned short pm1_cnt_port = (unsigned short)(pmbase + 0x04);
        outw(pm1_cnt_port, 0x3C00);
        io_wait();
        outw(pm1_cnt_port, 0x2000 | 0x1C00);
        io_wait();
    }
    outw(0xB2, 0x07);
    io_wait();
    outw(0x604, 0x2000);
    io_wait();
    __asm__ __volatile__("cli; hlt");
}

void render(unsigned char* video_memory, struct superblock* fs) {
	draw_rect(video_memory, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 69, 178, 253);
	draw_rect(video_memory, 0, 990, SCREEN_WIDTH, 40, 229, 236, 253);
	draw_start(video_memory, 650, 992, 0, 120, 212);
	if (fs->magic == 0x15800) {
		draw_os(video_memory, 640, 512, 0, 0, 0);     
	} else {
		draw_os(video_memory, 640, 512, 255, 0, 0);
	}
}

// Глобальный теневой буфер (вне стека), чтобы избежать Stack Overflow
// 1280 * 1024 * 4 байта = 5,242,880 байт
// Высокоскоростное копирование памяти через ассемблер (копируем по 4 байта за такт)
// Быстрый перенос конкретного прямоугольного участка из теневого буфера в VRAM
// Функции для чтения и записи MSR-регистров процессора
static inline unsigned long long rdmsr(unsigned int msr) {
    unsigned int low, high;
    __asm__ __volatile__("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((unsigned long long)high << 32) | low;
}

static inline void wrmsr(unsigned int msr, unsigned long long val) {
    unsigned int low = (unsigned int)val;
    unsigned int high = (unsigned int)(val >> 32);
    __asm__ __volatile__("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

// Включение режима Write-Combining для базового адреса видеопамяти VRAM
void enable_write_combining(unsigned int vram_base) {
    unsigned long long base_val = (vram_base & 0xFFFFF000) | 0x01; 
    unsigned long long mask_val = 0xF000000000000000ULL | 0x00000000FF800000ULL | 0x800;

    __asm__ __volatile__("cli");
    wrmsr(0x200, base_val);
    wrmsr(0x201, mask_val);
    
    unsigned long long def_type = rdmsr(0x2FF);
    wrmsr(0x2FF, def_type | 0x400); 
    __asm__ __volatile__("sti");
}

// ОСТАВЛЯЕМ ТОЛЬКО ОДИН ЭКЗЕМПЛЯР ЭТОЙ ФУНКЦИИ В ФАЙЛЕ:
static inline void flip_buffers(void* dest, const void* src, int dwords) {
    __asm__ __volatile__ (
        "cld\n\t"
        "rep movsd"
        : "+D"(dest), "+S"(src), "+c"(dwords)
        :
        : "memory"
    );
}

__attribute__((section(".text.entry")))
void kernel_main(void) {
	unsigned int* video_memory32 = (unsigned int*)0xD0000000; 
	struct superblock* fs = (struct superblock*)0x15800;
	
	// Первичный вывод обоев и интерфейса напрямую на экран
	render((unsigned char*)video_memory32, fs);
	
	// Выделяем теневой бэк-буфер для сохранения подложки окон (макс размер 500x300)
	unsigned int* window_bg_buffer = (unsigned int*)0x01000000;
	// Буфер для сохранения фона под 16x16 курсором мыши
	unsigned int* mouse_bg_buffer_local = (unsigned int*)0x01100000;
	
	int win_bg_saved = 0; 
	int pitch_dw = screen_pitch / 4; 

	struct window win[3];
	win[0].lt.x = 450;  win[0].lt.y = 400;
	win[0].width = 450; win[0].height = 200;
	win[0].is_visible = 0; 

	win[1].lt.x = 450;  win[1].lt.y = 300;
	win[1].width = 400; win[1].height = 250;
	win[1].is_visible = 0;

	win[2].lt.x = 900;  win[2].lt.y = 600;
	win[2].width = 300; win[2].height = 150;
	win[2].is_visible = 0;
	
	// Первичное сохранение подложки мыши на старте
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
			mouse_bg_buffer_local[y * 16 + x] = video_memory32[(mouse_y + y) * pitch_dw + (mouse_x + x)];
		}
	}
	draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);

	unsigned char selected_option = 0;
	static unsigned char prev_left_button = 0; 
	
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
					
					// 1. ПРЕФЛИП МЫШИ: Мгновенно восстанавливаем чистый фон под старой позицией мыши
					for (int y = 0; y < 16; y++) {
						for (int x = 0; x < 16; x++) {
							video_memory32[(mouse_y + y) * pitch_dw + (mouse_x + x)] = mouse_bg_buffer_local[y * 16 + x];
						}
					}
					
					signed char move_x = mouse_packet[1];
					signed char move_y = mouse_packet[2];
					unsigned char left_button = (mouse_packet[0] & 0x01);
					
					// Запоминаем старые координаты окна для отката подложки
					int old_win_x = 0, old_win_y = 0, win_w = 0, win_h = 0;
					int window_moved = 0;

					if (left_button == 1) {
						if (prev_left_button == 0 && dragged_window_idx == -1 && start_menu_open == 0) {
							for (int i = 0; i < 3; i++) {
								if (win[i].is_visible && mouse_x >= win[i].lt.x && mouse_x <= (win[i].lt.x + win[i].width) && mouse_y >= win[i].lt.y && mouse_y <= (win[i].lt.y + WIN_HH)) {
									dragged_window_idx = i;
									win_bg_saved = 0; // Сброс буфера для нового захвата
									break;  
								}
							}
						}
						
						if (dragged_window_idx != -1) {
							old_win_x = win[dragged_window_idx].lt.x;
							old_win_y = win[dragged_window_idx].lt.y;
							win_w = win[dragged_window_idx].width;
							win_h = win[dragged_window_idx].height + WIN_HH;
							window_moved = 1;

							// Если на прошлом шаге под окном сохранялся фон, выплёскиваем его обратно на экран (стираем окно)
							if (win_bg_saved == 1) {
								for (int wy = 0; wy < win_h; wy++) {
									for (int wx = 0; wx < win_w; wx++) {
										video_memory32[(old_win_y + wy) * pitch_dw + (old_win_x + wx)] = window_bg_buffer[wy * win_w + wx];
									}
								}
							} else {
								// Если это самый первый шаг захвата, просто перекрываем синим
								draw_rect((unsigned char*)video_memory32, old_win_x, old_win_y, win_w, win_h, 69, 178, 253);
							}

							// Сдвигаем окно
							win[dragged_window_idx].lt.x += move_x;
							win[dragged_window_idx].lt.y -= move_y;
						}
					} else {
						dragged_window_idx = -1;
						win_bg_saved = 0;
					}

					// Сдвигаем мышь
					mouse_x += move_x;
					mouse_y -= move_y;
					if (mouse_x < 0)    mouse_x = 0;
					if (mouse_x > 1264) mouse_x = 1264;
					if (mouse_y < 0)    mouse_y = 0;
					if (mouse_y > 1008) mouse_y = 1008;

					// Обработка кнопки Пуск
					if (mouse_x >= 630 && mouse_y >= 995 && mouse_x <= 675 && mouse_y <= 1020 && left_button == 1 && prev_left_button == 0) {
						start_menu_open = !start_menu_open;
						if (start_menu_open) {
							draw_rect((unsigned char*)video_memory32, 631, 992, 36, 36, 255, 255, 255);
							draw_rect((unsigned char*)video_memory32, 633, 990, 36, 36, 255, 255, 255);
							draw_circle((unsigned char*)video_memory32, 635, 992, 2, 255, 255, 255);  
							draw_circle((unsigned char*)video_memory32, 667, 992, 2, 255, 255, 255); 
							draw_circle((unsigned char*)video_memory32, 635, 1024, 2, 255, 255, 255);  
							draw_circle((unsigned char*)video_memory32, 667, 1024, 2, 255, 255, 255);  
							draw_start((unsigned char*)video_memory32, 650, 992, 10, 135, 215);
							draw_rect((unsigned char*)video_memory32, 400, 300, 500, 680, 255, 255, 255);
						} else {
							render((unsigned char*)video_memory32, fs);
							for (int i = 0; i < 3; i++) {
								if (win[i].is_visible) draw_window((unsigned char*)video_memory32, &win[i]);
							}
						}
					} else if ((mouse_x <= 630 || mouse_y <= 995 || mouse_x >= 675 || mouse_y >= 1020) && left_button == 1 && prev_left_button == 0 && start_menu_open == 1) {
						start_menu_open = 0;
						render((unsigned char*)video_memory32, fs);
						for (int i = 0; i < 3; i++) {
							if (win[i].is_visible) draw_window((unsigned char*)video_memory32, &win[i]);
						}
					}

					// 2. ПОСТФЛИП ОКНА: Если окно двигалось, сохраняем подложку на новом месте и рендерим его
					if (dragged_window_idx != -1 && window_moved) {
						int new_x = win[dragged_window_idx].lt.x;
						int new_y = win[dragged_window_idx].lt.y;

						// Запоминаем чистый фон из-под новой позиции окна в теневой буфер ОЗУ
						for (int wy = 0; wy < win_h; wy++) {
							for (int wx = 0; wx < win_w; wx++) {
								window_bg_buffer[wy * win_w + wx] = video_memory32[(new_y + wy) * pitch_dw + (new_x + wx)];
							}
						}
						win_bg_saved = 1;

						// Рисуем само окно поверх сохраненного кадра
						draw_window((unsigned char*)video_memory32, &win[dragged_window_idx]);
					}

					prev_left_button = left_button; 

					// 3. ПОСТФЛИП МЫШИ: Сохраняем фон под новой позицией мыши (там уже может быть кусок нового окна)
					for (int y = 0; y < 16; y++) {
						for (int x = 0; x < 16; x++) {
							mouse_bg_buffer_local[y * 16 + x] = video_memory32[(mouse_y + y) * pitch_dw + (mouse_x + x)];
						}
					}

					// Накладываем курсор мыши самым последним слоем на экран
					draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
				}
			}
		} 
		
		if (current_scancode != 0) {
			data = current_scancode;
			current_scancode = 0;
			if (data == 59) {
				draw_os((unsigned char*)video_memory32, 640, 512, 0, 0, 255);
				install_to_disk();
				draw_os((unsigned char*)video_memory32, 640, 512, 255, 0, 0);
			} else if (data == 56) {
				alt_pressed = 1;
			} else if (data == 184) {
				alt_pressed = 0;
			} 
			if (data == 62 && alt_pressed == 1) {
				menu_open = 1;
				selected_option = 0;
				win[0].is_visible = 1; // Исправлено для работы с массивом окон
				draw_window((unsigned char*)video_memory32, &win[0]);
				draw_off_menu((unsigned char*)video_memory32);
				draw_rect((unsigned char*)video_memory32, 500, 565, 100, 4, 0, 120, 212);
			} 
			if (menu_open == 1) {
				if (data == 28) {
					if (selected_option == 1) {
						sys_shutdown();
					} else {
						while (inb(0x64) & 2) { io_wait(); }
						outb(0x64, 0xFE);
					}
				} else if (data == 1) {
					menu_open = 0;
					render((unsigned char*)video_memory32, fs);
					for (int i = 0; i < 3; i++) {
						if (win[i].is_visible) draw_window((unsigned char*)video_memory32, &win[i]);
					}
					// Пересохраняем фон мыши после перерисовки сцены
					for (int y = 0; y < 16; y++) {
						for (int x = 0; x < 16; x++) {
							mouse_bg_buffer_local[y * 16 + x] = video_memory32[(mouse_y + y) * pitch_dw + (mouse_x + x)];
						}
					}
					draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
				} else if (data == 77) {
					selected_option = 1;
					draw_rect((unsigned char*)video_memory32, 500, 565, 100, 4, 255, 255, 255);
					draw_rect((unsigned char*)video_memory32, 750, 565, 100, 4, 0, 120, 212);
				} else if (data == 75) {
					selected_option = 0;
					draw_rect((unsigned char*)video_memory32, 750, 565, 100, 4, 255, 255, 255);
					draw_rect((unsigned char*)video_memory32, 500, 565, 100, 4, 0, 120, 212);
				}
			}
		}
		
		// Обновляем часы на экране
		int old_m = last_m;
		draw_time((unsigned char*)video_memory32);
		if (last_m != old_m) {
			// Если часы тикнули и затерли мышь, пересохраняем её буфер
			for (int y = 0; y < 16; y++) {
				for (int x = 0; x < 16; x++) {
					mouse_bg_buffer_local[y * 16 + x] = video_memory32[(mouse_y + y) * pitch_dw + (mouse_x + x)];
				}
			}
			draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
		}
		io_wait(); 
	}
}
