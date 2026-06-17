#include <kernel.h>

extern unsigned int screen_pitch;
extern unsigned int* back_buffer32;

// Выносим курсор в секцию данных, чтобы полностью разгрузить стек ядра
static const unsigned char graphics_cursor[16][16] = {
    {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 0, 0, 0, 0, 0},
    {2, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0},
    {2, 1, 2, 2, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 2, 2, 0, 2, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 2, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

void draw_rect(unsigned char* video_memory, int start_x, int start_y, int width, int height, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    if (a == 0) return; 
    for (int y = start_y; y < start_y + height; y++) {
        for (int x = start_x; x < start_x + width; x++) {
            int offset = (y * screen_pitch) + (x * 4);
            if (a == 255) {
                video_memory[offset]     = b;
                video_memory[offset + 1] = g;
                video_memory[offset + 2] = r;
                video_memory[offset + 3] = 255;
            } else {
                unsigned char bg_b = video_memory[offset];
                unsigned char bg_g = video_memory[offset + 1];
                unsigned char bg_r = video_memory[offset + 2];
                video_memory[offset]     = (unsigned char)(((b - bg_b) * a) >> 8) + bg_b;
                video_memory[offset + 1] = (unsigned char)(((g - bg_g) * a) >> 8) + bg_g;
                video_memory[offset + 2] = (unsigned char)(((r - bg_r) * a) >> 8) + bg_r;
                video_memory[offset + 3] = 255;
            }
        }
    }
}

void draw_os(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    static const unsigned char font_os[8] = {
        0b01110111, 0b10001000, 0b10001000, 0b11111000,
        0b10001111, 0b10001000, 0b10001000, 0b01110111
    };
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if ((font_os[row] >> (7 - col)) & 1) {
                draw_rect(video_memory, start_x + (col * 2), start_y + (row * 2), 2, 2, r, g, b, 255);
            }
        }
    }
}

void draw_cursor(unsigned char* video_memory, int start_x, int start_y) {
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            unsigned char pixel = graphics_cursor[row][col];
            if (pixel == 1) {
                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, 255, 255, 255, 255);
            } 
            else if (pixel == 2) {
                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, 0, 0, 0, 255);
            }
        }
    }
}

void undraw_cursor(unsigned char* video_memory, int start_x, int start_y) {
    extern unsigned int* back_buffer32; 
    unsigned int* dst32 = (unsigned int*)video_memory;
    extern int pitch_dw;

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int screen_x = start_x + x;
            int screen_y = start_y + y;

            if (screen_x >= 0 && screen_x < 1280 && screen_y >= 0 && screen_y < 1024) {
                int screen_idx = screen_y * pitch_dw + screen_x;
                dst32[screen_idx] = back_buffer32[screen_idx];
            }
        }               
    }
}


void draw_circle(unsigned char* video_memory, int center_x, int center_y, int radius, unsigned char r, unsigned char g, unsigned char b) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                draw_rect(video_memory, center_x + x, center_y + y, 1, 1, r, g, b, 255);
            }
        }
    }
}

void swap_buffers() {
    unsigned int* dest = (unsigned int*)0xD0000000;
    for (int i = 0; i < 1310720; i++) {
        dest[i] = back_buffer32[i];
    }
}

void draw_restart(unsigned char* video_memory, int start_x, int start_y) {
    draw_circle(video_memory, start_x, start_y, 50, 0, 120, 212);
    draw_circle(video_memory, start_x, start_y, 35, 255, 255, 255);
    start_y += 25;
    draw_rect(video_memory, start_x, start_y, 30, 30, 0, 120, 212, 255);
    start_x += 10;
    draw_rect(video_memory, start_x, start_y, 20, 20, 255, 255, 255, 255);
    start_x -= 40;
    start_y -= 8;
    draw_rect(video_memory, start_x, start_y, 30, 65, 255, 255, 255, 255);
}

void draw_off(unsigned char* video_memory, int start_x, int start_y) {
    draw_circle(video_memory, start_x, start_y, 50, 0, 120, 212);
    draw_circle(video_memory, start_x, start_y, 35, 255, 255, 255);
    start_y -= 50;
    start_x -= 5;
    draw_rect(video_memory, start_x, start_y, 10, 40, 0, 120, 212, 255);
}

void draw_off_menu(unsigned char* back_buffer, int win_x, int win_y) {
    int restart_x = win_x + 100;
    int off_x = win_x + 350;
    int icons_y = win_y + 100;
    draw_restart(back_buffer, restart_x, icons_y);
    draw_off(back_buffer, off_x, icons_y);
}