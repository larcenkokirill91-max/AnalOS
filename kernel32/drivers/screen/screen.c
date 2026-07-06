#include <kernel.h>
#include <stdint.h>
#include "wallpaper.h"

extern unsigned int screen_pitch;
extern int pitch_dw;
extern unsigned int* back_buffer32;
extern unsigned int* global_video_memory;

void draw_xor_frame(unsigned int* video_memory, int x, int y, int w, int h, int pitch) {
    if (!video_memory) return;

    for (int i = 0; i < w; i++) {
        int idx_top = y * pitch + (x + i);
        int idx_bottom = (y + h - 1) * pitch + (x + i);
        
        if (idx_top >= 0 && idx_top < 1280 * 1024) {
            video_memory[idx_top] ^= 0x00FFFFFF;
        }
        if (idx_bottom >= 0 && idx_bottom < 1280 * 1024) {
            video_memory[idx_bottom] ^= 0x00FFFFFF;
        }
    }

    for (int j = 0; j < h; j++) {
        int idx_left = (y + j) * pitch + x;
        int idx_right = (y + j) * pitch + (x + w - 1);
        
        if (idx_left >= 0 && idx_left < 1280 * 1024) {
            video_memory[idx_left] ^= 0x00FFFFFF;
        }
        if (idx_right >= 0 && idx_right < 1280 * 1024) {
            video_memory[idx_right] ^= 0x00FFFFFF;
        }
    }
}

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
    {2, 2, 0, 0, 2, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0}
};

void draw_rect(unsigned char* buffer, int x, int y, int width, int height, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    if (!buffer) return;
    
    unsigned int* buf32 = (unsigned int*)buffer;
    
    unsigned int color = (r << 16) | (g << 8) | b;

    int start_y = (y < 0) ? 0 : y;
    int end_y = (y + height > 1024) ? 1024 : (y + height);

    int start_x = (x < 0) ? 0 : x;
    int end_x = (x + width > 1280) ? 1280 : (x + width);

    for (int current_y = start_y; current_y < end_y; current_y++) {
        int row_offset = current_y * 1280;
        
        for (int current_x = start_x; current_x < end_x; current_x++) {
            buf32[row_offset + current_x] = color;
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
    unsigned int* dst32 = (unsigned int*)video_memory;

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

extern unsigned int* global_video_memory;
extern int pitch_dw;
extern unsigned int* back_buffer32;
static unsigned int static_back_buffer[1280 * 1024];
void swap_buffers() {
    unsigned int* dest = global_video_memory;
    unsigned int* src = back_buffer32;
    if (!dest || !src) return;

    for (int i = 0; i < 1310720; i++) {
        dest[i] = src[i];
    }

    asm volatile ("outw %0, %1" : : "a"((uint16_t)4), "Nd"((uint16_t)0x01CE));
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

extern const unsigned char _binary_kernel32_include_drivers_font_bin_start[];
extern const unsigned char _binary_kernel32_include_drivers_font_bin_end[];

void draw_char(unsigned char* video_memory, int char_code, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    static const unsigned char alpha_table[] = { 0, 255, 200, 125, 75 };
    
    const unsigned char* font_data = _binary_kernel32_include_drivers_font_bin_start;
    const unsigned char* char_glyph = font_data + (char_code * 12 * 6);

    for (int row = 0; row < 12; row++) {
        for (int col = 0; col < 6; col++) {
            unsigned char pixel = char_glyph[row * 6 + col];
            
            if (pixel >= 1 && pixel <= 4) {
                unsigned char alpha = alpha_table[pixel];
                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b, alpha);
            }
        }
    }
}

void draw_wallpaper(unsigned char* video_memory) {
    int img_idx = 0; 

    for (int y = 0; y < WALLPAPER_HEIGHT; y++) {
        for (int x = 0; x < WALLPAPER_WIDTH; x++) {
            
            unsigned char r = wallpaper_data[img_idx];
            unsigned char g = wallpaper_data[img_idx + 1];
            unsigned char b = wallpaper_data[img_idx + 2];
            unsigned char a = wallpaper_data[img_idx + 3];
            img_idx += 4;

            if (a == 0) continue;

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

