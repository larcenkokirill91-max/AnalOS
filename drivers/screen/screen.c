#include "screen.h"

extern unsigned int screen_pitch;

void draw_rect(unsigned char* video_memory, int start_x, int start_y, int width, int height, unsigned char r, unsigned char g, unsigned char b) {
    for (int y = start_y; y < start_y + height; y++) {
        for (int x = start_x; x < start_x + width; x++) {
            int offset = (y * screen_pitch) + (x * 4);
            video_memory[offset]     = b;
            video_memory[offset + 1] = g;
            video_memory[offset + 2] = r;
            video_memory[offset + 3] = 0;
        }
    }
}

void draw_A(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned char font_A[8] = {
        0b00011000,
        0b00100100,
        0b01000010,
        0b01111110,
        0b01000010,
        0b01000010,
        0b01000010,
        0b00000000
    };
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if ((font_A[row] >> (7 - col)) & 1) {
                draw_rect(video_memory, start_x + (col * 2), start_y + (row * 2), 2, 2, r, g, b);
            }
        }
    }
}

void draw_B(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned char font_B[8] = {
        0b01111100,
        0b01000010,
        0b01000010,
        0b01111100,
        0b01000010,
        0b01000010,
        0b01111100,
        0b00000000
    };
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if ((font_B[row] >> (7 - col)) & 1) {
                draw_rect(video_memory, start_x + (col * 2), start_y + (row * 2), 2, 2, r, g, b);
            }
        }
    }
}

void draw_C(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned char font_C[8] = {
        0b00011100,
        0b00100010,
        0b01000000,
        0b01000000,
        0b01000000,
        0b00100010,
        0b00011100,
        0b00000000
    };
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if ((font_C[row] >> (7 - col)) & 1) {
                draw_rect(video_memory, start_x + (col * 2), start_y + (row * 2), 2, 2, r, g, b);
            }
        }
    }
}

void draw_os(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned char font_os[8] = {
        0b01110111,
        0b10001000,
        0b10001000,
        0b11111000,
        0b10001111,
        0b10001000,
        0b10001000,
        0b01110111
    };
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if ((font_os[row] >> (7 - col)) & 1) {
                draw_rect(video_memory, start_x + (col * 2), start_y + (row * 2), 2, 2, r, g, b);
            }
        }
    }
}
void draw_off(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned char font_off[8] = {
        0b11111111,
        0b10000001,
        0b10011001,
        0b10100101,
        0b10100101,
        0b10111101,
        0b10000001,
        0b11111111
    };
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if ((font_off[row] >> (7 - col)) & 1) {
                draw_rect(video_memory, start_x + (col * 2), start_y + (row * 2), 2, 2, r, g, b);
            }
        }
    }
}
void draw_cursor(unsigned char* video_memory, int start_x, int start_y) {
	unsigned char cursor[8][8] = {
		{2, 2, 0, 0, 0, 0, 0, 0},
		{2, 1, 2, 0, 0, 0, 0, 0},
		{2, 1, 1, 2, 0, 0, 0, 0},
		{2, 1, 1, 1, 2, 0, 0, 0},
		{2, 1, 1, 1, 1, 2, 0, 0},
		{2, 1, 1, 2, 2, 2, 2, 0},
		{2, 1, 2, 0, 0, 0, 0, 0},
		{2, 2, 0, 0, 0, 0, 0, 0}
	};
	for (int row = 0; row < 8; row++) {
		for (int col = 0; col < 8; col++) {
			unsigned char pixel = cursor[row][col];
			if (pixel == 1) {
				draw_rect(video_memory, start_x + col, start_y + row, 1, 1, 255, 255, 255);
			} 
			else if (pixel == 2) {
				draw_rect(video_memory, start_x + col, start_y + row, 1, 1, 0, 0, 0);
			}
		}
	}
}
void undraw_cursor(unsigned char* video_memory, int start_x, int start_y) {
	unsigned char cursor[8][8] = {
		{2, 2, 0, 0, 0, 0, 0, 0},
		{2, 1, 2, 0, 0, 0, 0, 0},
		{2, 1, 1, 2, 0, 0, 0, 0},
		{2, 1, 1, 1, 2, 0, 0, 0},
		{2, 1, 1, 1, 1, 2, 0, 0},
		{2, 1, 1, 2, 2, 2, 2, 0}, 
		{2, 1, 2, 0, 0, 0, 0, 0},
		{2, 2, 0, 0, 0, 0, 0, 0}
	};
	for (int row = 0; row < 8; row++) {
		for (int col = 0; col < 8; col++) {
			unsigned char pixel = cursor[row][col];
			if (pixel == 1 || pixel == 2) {
				draw_rect(video_memory, start_x + col, start_y + row, 1, 1, 69, 178, 253);
			}
		}
	}
}
