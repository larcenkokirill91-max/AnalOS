#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 1024
#define STATIC_VIDEO_MEMORY ((unsigned int*)0xD0000000)
#define STATIC_PITCH_DW 1280
#define WIN_HH 25

#include <kernel.h>
#include <stdint.h>

// 1. ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ЯДРА
unsigned int screen_pitch = 5120;
int global_pitch_dw = 1280;
unsigned int* global_video_memory = 0;

int pitch_dw = 1280; 

static unsigned int static_back_buffer[1280 * 1024];
unsigned int* back_buffer32 = static_back_buffer;

int mouse_x = 0;
int mouse_y = 0;
int dragged_window_idx = -1;
int menu_open = 0;
int start_menu_open = 0;
int alt_pressed = 0;
int y = 0, mth = 0, d = 0, h = 0, m = 0, s = 0;
int last_s = -1;
int last_m = -1;
int last_h = -1;
int last_d = -1;
int last_mth = -1;
int last_y = -1;

void idt_init();
void mouse_init();
struct window;
void draw_window(unsigned char* video_memory, struct window* win);
void swap_buffers();
void draw_cursor(unsigned char* video_memory, int start_x, int start_y);
void draw_xor_frame(unsigned int* video_memory, int x, int y, int w, int h, int pitch);
void draw_char(unsigned char* video_memory, int char_code, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b);
void cpp_init_windows();
void cpp_draw_windows(unsigned char* video_memory);
void cpp_draw_single_window(unsigned char* video_memory, int idx);
void cpp_set_window_visible(int idx, int visible);
int cpp_get_window_is_visible(int idx);
void cpp_set_window_position(int idx, int new_x, int new_y);
int cpp_get_window_x(int idx);
int cpp_get_window_y(int idx);
int cpp_get_window_width(int idx);
int cpp_get_window_height(int idx);
void cpp_draw_taskbar_widgets(unsigned char* back_buffer);
int cpp_handle_mouse_hover(int mouse_x, int mouse_y);

void render(unsigned char* video_memory, struct superblock* fs) {
    if (!video_memory) return;
    draw_rect(video_memory, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 255, 255);
    draw_rect((unsigned char*)back_buffer32, 0, 966, 1280, 58, 238, 238, 238, 255);
}
void init_pit_channel0(void) {
    uint32_t divisor = 1193182 / 100; 
    
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}
void sys_shutdown() {
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

void draw_time(unsigned char* back_buffer, unsigned char* video_memory) { 
    get_rtc_time(&y, &mth, &d, &h, &m, &s);
    
    if (m != last_m || h != last_h || d != last_d || mth != last_mth || y != last_y) { 
        last_m = m; last_h = h; last_d = d; last_mth = mth; last_y = y;
        
        int fh = floor_div(h, 10); 
        int lh = tenth_digit(h, 10); 
        int fm = floor_div(m, 10); 
        int lm = tenth_digit(m, 10); 

        int fd = floor_div(d, 10);
        int ld = tenth_digit(d, 10);
        int fmth = floor_div(mth, 10);
        int lmth = tenth_digit(mth, 10);
        
        int current_y = 2000 + y; 
        int y1 = current_y / 1000;
        int y2 = (current_y / 100) % 10;
        int y3 = (current_y / 10) % 10;
        int y4 = current_y % 10;

        draw_rect(back_buffer, 1100, 966, 152, 58, 238, 238, 238, 255); 

        // СТРОКА 1: ВРЕМЯ (Y 979)
        int time_x = 1150; 

        draw_char(back_buffer, fh + 16, time_x,      979, 0, 0, 0); 
        draw_char(back_buffer, lh + 16, time_x + 8,  979, 0, 0, 0); 
        draw_char(back_buffer, 26,      time_x + 14, 979, 0, 0, 0); // Двоеточие
        draw_char(back_buffer, fm + 16, time_x + 18, 979, 0, 0, 0); 
        draw_char(back_buffer, lm + 16, time_x + 26, 979, 0, 0, 0); 

        // СТРОКА 2: ДАТА (Y 1001)
        int date_x = 1130; 

        draw_char(back_buffer, fd + 16,   date_x,      1001, 0, 0, 0);
        draw_char(back_buffer, ld + 16,   date_x + 8,  1001, 0, 0, 0);
        draw_char(back_buffer, 14,        date_x + 14, 1004, 0, 0, 0); // Точка
        
        draw_char(back_buffer, fmth + 16, date_x + 22, 1001, 0, 0, 0);
        draw_char(back_buffer, lmth + 16, date_x + 30, 1001, 0, 0, 0);
        draw_char(back_buffer, 14,        date_x + 36, 1004, 0, 0, 0); // Точка
        
        draw_char(back_buffer, y1 + 16,   date_x + 44, 1001, 0, 0, 0);
        draw_char(back_buffer, y2 + 16,   date_x + 52, 1001, 0, 0, 0);
        draw_char(back_buffer, y3 + 16,   date_x + 60, 1001, 0, 0, 0);
        draw_char(back_buffer, y4 + 16,   date_x + 68, 1001, 0, 0, 0);

        unsigned int* src32 = (unsigned int*)back_buffer; 
        unsigned int* dst32 = (unsigned int*)video_memory; 
        for (int y_coord = 966; y_coord < 1024; y_coord++) { 
            int row_offset = y_coord * pitch_dw; 
            for (int x_coord = 1100; x_coord < 1252; x_coord++) { 
                dst32[row_offset + x_coord] = src32[row_offset + x_coord];
            } 
        }
    }
}

void init_pit_system_timer(void) {
    // 1193182 / 100 Гц = 11931 (0x2E9B)
    uint32_t divisor = 1193182 / 100; 
    
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

#define HEAP_START 0x04000000  
#define HEAP_SIZE  0x04000000  

struct memory_block {
    unsigned int size;       
    unsigned char is_free;   
    struct memory_block* next; 
};

static struct memory_block* free_list = (struct memory_block*)HEAP_START;

__attribute__((section(".text.entry")))
void heap_init() {
    free_list->size = HEAP_SIZE - sizeof(struct memory_block);
    free_list->is_free = 1;
    free_list->next = 0; 
}

__attribute__((section(".text.entry")))
void* malloc(unsigned int size) {
    if (size == 0) return 0;
    size = (size + 3) & ~3;

    struct memory_block* curr = free_list;
    while (curr) {
        if (curr->is_free && curr->size >= size) {
            if (curr->size >= size + sizeof(struct memory_block) + 4) {
                struct memory_block* next_block = (struct memory_block*)((unsigned char*)curr + sizeof(struct memory_block) + size);
                next_block->size = curr->size - size - sizeof(struct memory_block);
                next_block->is_free = 1;
                next_block->next = curr->next;
                curr->size = size;
                curr->next = next_block;
            }
            curr->is_free = 0;
            return (void*)((unsigned char*)curr + sizeof(struct memory_block));
        }
        curr = curr->next;
    }
    return 0; 
}

static void play_sound(uint32_t nFrequence) {
    if (nFrequence == 0) return;
    
    uint32_t Div = 1193180 / nFrequence;
    
    outb(0x43, 0xB6);
    
    outb(0x42, (uint8_t)(Div & 0xFF));
    outb(0x42, (uint8_t)((Div >> 8) & 0xFF));
    
    uint8_t tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

 static void nosound() {
 	uint8_t tmp = inb(0x61) & 0xFC;
     
 	outb(0x61, tmp);
 }
 
static void play_sound(uint32_t nFrequence);
void timer_wait(int ticks);

 void beep() {
 	 play_sound(500);
 	 timer_wait(10);
 	 nosound();
 }

__attribute__((section(".text.entry")))
void free(void* ptr) {
    if (!ptr) return;
    struct memory_block* block = (struct memory_block*)((unsigned char*)ptr - sizeof(struct memory_block));
    block->is_free = 1;

    struct memory_block* core_block = free_list;
    while (core_block) {
        if (core_block->is_free && core_block->next && core_block->next->is_free) {
            core_block->size += sizeof(struct memory_block) + core_block->next->size;
            core_block->next = core_block->next->next;
            continue; 
        }
        core_block = core_block->next;
    }
}

__attribute__((section(".text.entry")))
void kernel_main(void) {
    heap_init();
    back_buffer32 = (unsigned int*)malloc(1280 * 1024 * sizeof(unsigned int));
    init_pit_channel0();
    unsigned int* vbe_info_struct = (unsigned int*)0x7000;
    unsigned int* real_lfb = (unsigned int*)(vbe_info_struct[0x28 / 4]); 

    if (real_lfb != 0 && real_lfb != (unsigned int*)0xFFFFFFFF) {
        global_video_memory = real_lfb;
    } else {
        global_video_memory = (unsigned int*)0xFD000000;
    }

    unsigned int check_addr = (unsigned int)global_video_memory;
    if (check_addr == 0xFD000000 || check_addr == 0xE0000000) {
        pitch_dw = 1280;
    } else {
        pitch_dw = 1280; 
    }
    global_pitch_dw = pitch_dw;

    unsigned int* video_memory32 = global_video_memory;
    struct superblock* fs = (struct superblock*)0x15800;

    int old_win_x = 0;
    int old_win_y = 0;
    int is_dragging = 0;
    int frame_x = 0;
    int frame_y = 0;
    
    unsigned char selected_option = 0;
    static unsigned char prev_left_button = 0;
    static unsigned int last_printed_tick = 0;
    init_pit_channel0(); 

    idt_init();
    mouse_init();

    render((unsigned char*)back_buffer32, fs);
    
    if (!back_buffer32) {
        back_buffer32 = (unsigned int*)0x02000000;
    }

    cpp_init_windows();

    dragged_window_idx = -1;
    prev_left_button = 0;
    menu_open = 0;
    start_menu_open = 0;
    
    render((unsigned char*)back_buffer32, fs);
    for (int i = 0; i < 3; i++) {
        if (cpp_get_window_is_visible(i)) {
            cpp_draw_single_window((unsigned char*)back_buffer32, i);
        }
    }
    // cpp_draw_taskbar_widgets((unsigned char*)back_buffer32);
    
    swap_buffers();
    draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
    beep();

    while(1) {
                // ====================================================================
        // --- 1. ОБРАБОТКА МЫШИ ПО СОБЫТИЮ IRQ12 ---
        // ====================================================================
        int mouse_ready_flag = 0;
        signed char local_packet[3];

        __asm__ __volatile__("cli");
        if (mouse_has_data()) {
            mouse_ready_flag = 1;
            local_packet[0] = mouse_packet[0];
            local_packet[1] = mouse_packet[1];
            local_packet[2] = mouse_packet[2];
            clear_mouse_flag();
        }
        __asm__ __volatile__("sti");
        if (mouse_ready_flag) {
            int curr_mouse_x = mouse_x;
            int curr_mouse_y = mouse_y;

            for (int y_offset = 0; y_offset < 16; y_offset++) {
                int target_y = curr_mouse_y + y_offset;
                if (target_y < 0 || target_y >= 1024) continue;

                int row_offset = target_y * 1280;

                for (int x_offset = 0; x_offset < 16; x_offset++) {
                    int target_x = curr_mouse_x + x_offset;
                    if (target_x < 0 || target_x >= 1280) continue;

                    int screen_idx = row_offset + target_x;
                    
                    video_memory32[screen_idx] = back_buffer32[screen_idx]; 
                }
            }

            signed char move_x = local_packet[1]; 
            signed char move_y = local_packet[2]; 
            mouse_x += move_x;
            mouse_y -= move_y;

            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x > SCREEN_WIDTH - 16) mouse_x = SCREEN_WIDTH - 16;
            if (mouse_y > SCREEN_HEIGHT - 16) mouse_y = SCREEN_HEIGHT - 16;

            unsigned char left_button = (local_packet[0] & 0x01);
            if (left_button == 1) {
                if (prev_left_button == 0 && dragged_window_idx == -1 && start_menu_open == 0) {
                    for (int i = 0; i < 3; i++) {
                        if (cpp_get_window_is_visible(i) && 
                            mouse_x >= cpp_get_window_x(i) && 
                            mouse_x <= (cpp_get_window_x(i) + cpp_get_window_width(i)) && 
                            mouse_y >= cpp_get_window_y(i) && 
                            mouse_y <= (cpp_get_window_y(i) + WIN_HH)) {
                            
                            dragged_window_idx = i;
                            old_win_x = cpp_get_window_x(i);
                            old_win_y = cpp_get_window_y(i);
                            is_dragging = 1; 
                            frame_x = cpp_get_window_x(i);
                            frame_y = cpp_get_window_y(i);
                            
                            draw_xor_frame(video_memory32, frame_x, frame_y, cpp_get_window_width(i), cpp_get_window_height(i), pitch_dw);
                            break;
                        }
                    }
                }
                
                if (dragged_window_idx != -1 && (move_x != 0 || move_y != 0)) {
                    draw_xor_frame(video_memory32, frame_x, frame_y, cpp_get_window_width(dragged_window_idx), cpp_get_window_height(dragged_window_idx), pitch_dw);
                    frame_x += move_x;
                    frame_y -= move_y;
                    draw_xor_frame(video_memory32, frame_x, frame_y, cpp_get_window_width(dragged_window_idx), cpp_get_window_height(dragged_window_idx), pitch_dw);
                }
            } else {
                if (dragged_window_idx != -1 && is_dragging == 1) {
                    draw_xor_frame(video_memory32, frame_x, frame_y, cpp_get_window_width(dragged_window_idx), cpp_get_window_height(dragged_window_idx), pitch_dw);
                    cpp_set_window_position(dragged_window_idx, frame_x, frame_y);
                    
                }
            }
            prev_left_button = left_button;
            
            draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
        }

        if (is_dragging == 1 || menu_open == 1) {
            

            render((unsigned char*)back_buffer32, fs);

            cpp_draw_windows((unsigned char*)back_buffer32);

            if (menu_open == 1) {
                int win0_x = cpp_get_window_x(0);
                int win0_y = cpp_get_window_y(0);
                
                draw_off_menu((unsigned char*)back_buffer32, win0_x, win0_y);
                
                int current_line_x = (selected_option == 1) ? (win0_x + 300) : (win0_x + 50);
                int current_line_y = win0_y + 165;
                draw_rect((unsigned char*)back_buffer32, current_line_x, current_line_y, 100, 4, 0, 120, 212, 255);
            }


            swap_buffers();


            if (prev_left_button == 0 && is_dragging == 1) {
                is_dragging = 0;
                dragged_window_idx = -1;
            }

            draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
        }

        int kbd_ready_flag = 0;
        unsigned char kbd_data = 0;

        __asm__ __volatile__("cli");
        if (keyboard_has_data()) {
            kbd_ready_flag = 1;
            kbd_data = current_scancode;
            clear_keyboard_flag();
        }
        __asm__ __volatile__("sti");

        if (kbd_ready_flag) {
            if (kbd_data == 59) {
                beep();
            } else if (kbd_data == 56) {
                alt_pressed = 1;
            } else if (kbd_data == 184) {
                alt_pressed = 0;
            }

            if (kbd_data == 57 && menu_open == 0) {
                menu_open = 1;
                selected_option = 0;
                cpp_set_window_visible(0, 1);
                int start_line_y = cpp_get_window_y(0) + 165;
                int start_left_x = cpp_get_window_x(0) + 50;
                
                cpp_draw_single_window((unsigned char*)back_buffer32, 0);
                draw_off_menu((unsigned char*)back_buffer32, cpp_get_window_x(0), cpp_get_window_y(0));
                draw_rect((unsigned char*)back_buffer32, start_left_x, start_line_y, 100, 4, 0, 120, 212, 255);
                
                last_m = -1;
                // draw_time((unsigned char*)back_buffer32, (unsigned char*)video_memory32);
                
                swap_buffers();
                draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
            }

            if (menu_open == 1) {
                int line_y = cpp_get_window_y(0) + 165;
                int left_line_x = cpp_get_window_x(0) + 50;
                int right_line_x = cpp_get_window_x(0) + 300;

                if (kbd_data == 28) {
                    if (selected_option == 1) {
                        sys_shutdown();
                    } else {
                        while (inb(0x64) & 2) { io_wait(); }
                        outb(0x64, 0xFE);
                    }
                } 
                else if (kbd_data == 1) {
                    menu_open = 0;
                    cpp_set_window_visible(0, 0);
                    render((unsigned char*)back_buffer32, fs);
                    for (int i = 0; i < 3; i++) {
                        if (cpp_get_window_is_visible(i)) {
                            cpp_draw_single_window((unsigned char*)back_buffer32, i);
                        }
                    }
                    cpp_draw_taskbar_widgets((unsigned char*)back_buffer32);
                    
                    last_m = -1;
                    // draw_time((unsigned char*)back_buffer32, (unsigned char*)video_memory32);
                    
                    swap_buffers();
                    draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
                } 
                else if (kbd_data == 77) {
                    selected_option = 1;
                    draw_rect((unsigned char*)back_buffer32, left_line_x, line_y, 100, 4, 255, 255, 255, 255);
                    draw_rect((unsigned char*)back_buffer32, right_line_x, line_y, 100, 4, 0, 120, 212, 255);
                    swap_buffers();
                    draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
                } 
                else if (kbd_data == 75) {
                    selected_option = 0;
                    draw_rect((unsigned char*)back_buffer32, right_line_x, line_y, 100, 4, 255, 255, 255, 255);
                    draw_rect((unsigned char*)back_buffer32, left_line_x, line_y, 100, 4, 0, 120, 212, 255);
                    swap_buffers();
                    draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
                }
            }
        }

        // --- 3. ОБНОВЛЕНИЕ ВРЕМЕНИ ПО ТАЙМЕРУ PIT (IRQ0) ---
     //   __asm__ __volatile__("cli");
        //if (timer_ticks - last_printed_tick >= 18) {
      //      last_printed_tick += 18; 
      //      __asm__ __volatile__("sti"); 
//
     //       draw_time((unsigned char*)back_buffer32, (unsigned char*)video_memory32);
      //      
      //      if (mouse_x >= 1180 && mouse_y >= 990) {
      //          draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
       //     }
      //      __asm__ __volatile__("cli"); 
     //   }
//
     //   if (!mouse_has_data() && !keyboard_has_data()) {
     //       __asm__ __volatile__("sti\n\thlt"); 
      //  } else {
      //      __asm__ __volatile__("sti"); 
      //  }
    }
}
