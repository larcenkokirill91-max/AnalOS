#pragma once

#define FONT_H
#ifndef FONT_H

#include <kernel.h>

extern const unsigned char _binary_kernel_include_drivers_font_bin_start[];
extern const unsigned char _binary_kernel_include_drivers_font_bin_end[];

static void draw_char(unsigned char* video_memory, int char_code, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    static const unsigned char alpha_table[] = { 0, 255, 200, 125, 75 };
    
    const unsigned char* font_data = _binary_kernel_include_drivers_font_bin_start;
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

#endif