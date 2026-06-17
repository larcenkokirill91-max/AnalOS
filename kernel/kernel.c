#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 1024
#define WIN_HH 25
#define WIN_WW 5

#include <kernel.h>

unsigned int* back_buffer32 = (unsigned int*)0x04000000; 
unsigned int mouse_bg_buffer[256];
int alt_pressed = 0;
int menu_open = 0;
int mouse_x = 640;
int mouse_y = 512;
int mouse_cycle = 0;
signed char mouse_packet[3];
unsigned char data = 0;
int h = 0, m = 0, s = 0;
int last_s = -1;
int last_m = -1;
int last_h = -1;
int start_menu_open = 0;
int dragged_window_idx = -1;
int is_mouse_pressed = 0;
unsigned char current_scancode;
unsigned int screen_pitch = 5120;
int pitch_dw = 1280; 
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
void draw_time(unsigned char* back_buffer, unsigned char* video_memory) {
    get_rtc_time(&h, &m, &s);
    if (m != last_m || h != last_h) {
        last_m = m;
        last_h = h;
        int fh = floor_div(h, 10);
        int lh = tenth_digit(h, 10);
        int fm = floor_div(m, 10);
        int lm = tenth_digit(m, 10);
        draw_rect(back_buffer, 1180, 1000, 20, 20, 229, 236, 253, 255);
        draw_digit[fh](back_buffer, 1180, 1000, 0, 0, 0);
        draw_rect(back_buffer, 1196, 1000, 20, 20, 229, 236, 253, 255);
        draw_digit[lh](back_buffer, 1196, 1000, 0, 0, 0);
        draw_rect(back_buffer, 1212, 1002, 4, 4, 0, 0, 0, 255);
        draw_rect(back_buffer, 1212, 1012, 4, 4, 0, 0, 0, 255);
        draw_rect(back_buffer, 1216, 1000, 20, 20, 229, 236, 253, 255);
        draw_digit[fm](back_buffer, 1216, 1000, 0, 0, 0);
        draw_rect(back_buffer, 1232, 1000, 20, 20, 229, 236, 253, 255);
        draw_digit[lm](back_buffer, 1232, 1000, 0, 0, 0);
        unsigned int* src32 = (unsigned int*)back_buffer;
        unsigned int* dst32 = (unsigned int*)video_memory;
        for (int y = 1000; y < 1020; y++) {
            int row_offset = y * pitch_dw;
            for (int x = 1180; x < 1252; x++) {
                dst32[row_offset + x] = src32[row_offset + x];
            }
        }
    }
}
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
    draw_rect(video_memory, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 69, 178, 253, 255);
    draw_rect(video_memory, 0, 990, SCREEN_WIDTH, 40, 229, 236, 253, 255);
    draw_start(video_memory, 650, 992, 0, 120, 212);
    if (fs->magic == 0x15800) {
        draw_os(video_memory, 640, 512, 0, 0, 0);
    } else {
        draw_os(video_memory, 640, 512, 255, 0, 0);
    }
}
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
    unsigned int* window_bg_buffer = (unsigned int*)0x01000000;
    unsigned int* mouse_bg_buffer_local = (unsigned int*)0x01100000;
    int win_bg_saved = 0;
    int pitch_dw = screen_pitch / 4;
    unsigned int time_counter = 0;
    int old_win_x = 0;
    int old_win_y = 0;
    int is_dragging = 0;
    int frame_x = 0;
    int frame_y = 0;
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
    unsigned char selected_option = 0;
    static unsigned char prev_left_button = 0;
    mouse_init();
    idt_init();
    dragged_window_idx = -1;
    prev_left_button = 0;
    menu_open = 0;
    start_menu_open = 0;
    render((unsigned char*)back_buffer32, fs); 
    for (int i = 0; i < 3; i++) {
        if (win[i].is_visible) draw_window((unsigned char*)back_buffer32, &win[i]);
    }
    swap_buffers();
    draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
    while(1) {
        unsigned char status = inb(0x64);
        if (status & 0x01) {
            data = inb(0x60);
            if (status & 0x20) {
                if (mouse_cycle == 0 && (data & 0x08) == 0) { continue; }
                mouse_packet[mouse_cycle] = data;
                mouse_cycle++;
                if (mouse_cycle == 3) {
                    mouse_cycle = 0;
                    if ((mouse_packet[0] & 0x08) == 0) { continue; }
                    for (int y = 0; y < 16; y++) {
                        for (int x = 0; x < 16; x++) {
                            int screen_idx = (mouse_y + y) * pitch_dw + (mouse_x + x);
                            video_memory32[screen_idx] = back_buffer32[screen_idx]; 
                        }
                    }
                    signed char move_x = mouse_packet[1];
                    signed char move_y = mouse_packet[2];
                    mouse_x += move_x;
                    mouse_y -= move_y;
                    if (mouse_x < 0) mouse_x = 0;
                    if (mouse_y < 0) mouse_y = 0;
                    if (mouse_x > SCREEN_WIDTH - 16) mouse_x = SCREEN_WIDTH - 16;
                    if (mouse_y > SCREEN_HEIGHT - 16) mouse_y = SCREEN_HEIGHT - 16;
                    unsigned char left_button = (mouse_packet[0] & 0x01);
                    if (left_button == 1) {
                        if (prev_left_button == 0 && dragged_window_idx == -1 && start_menu_open == 0) {
                            for (int i = 0; i < 3; i++) {
                                if (win[i].is_visible && mouse_x >= win[i].lt.x && mouse_x <= (win[i].lt.x + win[i].width) && mouse_y >= win[i].lt.y && mouse_y <= (win[i].lt.y + WIN_HH)) {
                                    dragged_window_idx = i;
                                    old_win_x = win[i].lt.x;
                                    old_win_y = win[i].lt.y;
                                    is_dragging = 1; 
                                    frame_x = win[i].lt.x;
                                    frame_y = win[i].lt.y;
                                    draw_xor_frame(video_memory32, frame_x, frame_y, win[i].width, win[i].height, pitch_dw);
                                    break;
                                }
                            }
                        }
                        if (dragged_window_idx != -1 && (move_x != 0 || move_y != 0)) {
                            draw_xor_frame(video_memory32, frame_x, frame_y, win[dragged_window_idx].width, win[dragged_window_idx].height, pitch_dw);
                            win[dragged_window_idx].lt.x += move_x;
                            win[dragged_window_idx].lt.y -= move_y;
                            frame_x = win[dragged_window_idx].lt.x;
                            frame_y = win[dragged_window_idx].lt.y;
                            draw_xor_frame(video_memory32, frame_x, frame_y, win[dragged_window_idx].width, win[dragged_window_idx].height, pitch_dw);
                        }
                    } else {
                        if (dragged_window_idx != -1 && is_dragging == 1) {
                            draw_xor_frame(video_memory32, frame_x, frame_y, win[dragged_window_idx].width, win[dragged_window_idx].height, pitch_dw);
                            render((unsigned char*)back_buffer32, fs);
                            for (int i = 0; i < 3; i++) {
                                if (win[i].is_visible) {
                                    draw_window((unsigned char*)back_buffer32, &win[i]);
                                }
                            }
                            swap_buffers();
                            is_dragging = 0;
                        }
                        dragged_window_idx = -1;
                    }
                    prev_left_button = left_button;
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
            if (data == 62 && alt_pressed == 1 && menu_open == 0) {
                menu_open = 1;
                selected_option = 0;
                win[0].is_visible = 1;
                draw_window((unsigned char*)back_buffer32, &win[0]);
                draw_off_menu((unsigned char*)back_buffer32);
                draw_rect((unsigned char*)back_buffer32, 500, 565, 100, 4, 0, 120, 212, 255);
                swap_buffers();
                draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
            }
            if (menu_open == 1) {
                int line_y = win[0].lt.y + 165;
                int left_line_x = win[0].lt.x + 50;
                int right_line_x = win[0].lt.x + 300;

                if (data == 28) { // Клавиша Enter
                    if (selected_option == 1) {
                        sys_shutdown();
                    } else {
                        while (inb(0x64) & 2) { io_wait(); }
                        outb(0x64, 0xFE);
                    }
                } 
                else if (data == 1) {
                    menu_open = 0;
                    render((unsigned char*)back_buffer32, fs);
                    for (int i = 0; i < 3; i++) {
                        if (win[i].is_visible) {
                            draw_window((unsigned char*)back_buffer32, &win[i]);
                        }
                    }
                    swap_buffers();
                    draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
                } 
                else if (data == 77) {
                    selected_option = 1;
                    draw_rect((unsigned char*)back_buffer32, left_line_x, line_y, 100, 4, 255, 255, 255, 255);
                    draw_rect((unsigned char*)back_buffer32, right_line_x, line_y, 100, 4, 0, 120, 212, 255);
                    swap_buffers();
                    draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
                } 
                else if (data == 75) {
                    selected_option = 0;
                    draw_rect((unsigned char*)back_buffer32, right_line_x, line_y, 100, 4, 255, 255, 255, 255);
                    draw_rect((unsigned char*)back_buffer32, left_line_x, line_y, 100, 4, 0, 120, 212, 255);
                    swap_buffers();
                    draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
                }
            }
        }
        time_counter++;
        if (time_counter >= 4000) {
            time_counter = 0;
            draw_time((unsigned char*)back_buffer32, (unsigned char*)video_memory32);
            if (mouse_x >= 1170 && mouse_y >= 990) {
                draw_cursor((unsigned char*)video_memory32, mouse_x, mouse_y);
            }
        }
        io_wait();
    }
}