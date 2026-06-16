#include <kernel.h>

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
				draw_rect(video_memory, start_x + (col * 2), start_y + (row * 2), 2, 2, r, g, b, 255);
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
				draw_rect(video_memory, start_x + (col * 2), start_y + (row * 2), 2, 2, r, g, b, 255);
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
				draw_rect(video_memory, start_x + (col * 2), start_y + (row * 2), 2, 2, r, g, b, 255);
			}
		}
	}
}

void draw_zero(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned short font_zero[16] = {
    0b0000000000000000,
    0b0000000000000000,
	0b0000001110000000,
	0b0000010001000000,
	0b0000010001000000,
	0b0000010001000000,
	0b0000010001000000,
	0b0000010001000000,
	0b0000010001000000,
	0b0000010001000000,
	0b0000010001000000,
	0b0000010001000000,
	0b0000010001000000,
	0b0000001110000000,
	0b0000000000000000,
	0b0000000000000000,
    };
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            if ((font_zero[row] >> (15 - col)) & 1) {
                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b, 255);
            }
        }
    }
}

void draw_one(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned short font_one[16] = {
        0b0000000000000000,
        0b0000000000000000,
        0b0000000001100000,
        0b0000000011100000,
        0b0000000000100000,
        0b0000000000100000,
        0b0000000000100000,
        0b0000000000100000,
        0b0000000000100000,
        0b0000000000100000,
        0b0000000000100000,
        0b0000000000100000,
        0b0000000000100000,
        0b0000000000100000,
        0b0000000000000000,
        0b0000000000000000
    };
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            if ((font_one[row] >> (15 - col)) & 1) {
                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b, 255);
            }
        }
    }
}

void draw_two(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned short font_two[16] = {
        0b0000000000000000,
        0b0000111110000000,
        0b0001000001000000,
        0b0010000000100000,
        0b0010000000100000,
        0b0000000000100000,
        0b0000000001000000,
        0b0000000010000000,
        0b0000000100000000,
        0b0000001000000000,
        0b0000010000000000,
        0b0000100000000000,
        0b0001000000000000,
        0b0010000000000000,
        0b0011111111110000,
        0b0000000000000000
    };
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            if ((font_two[row] >> (15 - col)) & 1) {
                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b, 255);
            }
        }
    }
}

void draw_three(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned short font_three[16] = {
	0b0000000000000000,
	0b0000011111100000,
	0b0000100000010000,
	0b0001000000001000,
	0b0001000000001000,
	0b0000000000001000,
	0b0000000000001000,
	0b0000011111110000,
	0b0000000000001000,
	0b0000000000000100,
	0b0000000000000100,
	0b0000000000000100,
	0b0001000000000100,
	0b0001000000001100,
	0b0000110000011000,
	0b0000001111100000
    };
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            if ((font_three[row] >> (15 - col)) & 1) {
                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b, 255);
            }
        }
    }
}

void draw_four(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned short font_four[16] = {
        0b0000000000000000,
        0b0000000000010000,
        0b0000000000110000,
        0b0000000001010000,
        0b0000000010010000,
        0b0000000100010000,
        0b0000001000010000,
        0b0000010000010000,
        0b0000100000010000,
        0b0001000000010000,
        0b0010000000010000,
        0b0111111111111110,
        0b0000000000010000,
        0b0000000000010000,
        0b0000000000010000,
        0b0000000000010000
    };
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            if ((font_four[row] >> (15 - col)) & 1) {
                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b, 255);
            }
        }
    }
}

void draw_five(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned short font_five[16] = {
	0b0000000000000000,
	0b0001111111100000,
	0b0001000000000000,
	0b0001000000000000,
	0b0001000000000000,
	0b0001000000000000,
	0b0001111111000000,
	0b0000000001100000,
	0b0000000000100000,
	0b0000000000100000,
	0b0000000000100000,
	0b0010000000100000,
	0b0011000001100000,
	0b0001100011000000,
	0b0000111110000000,
	0b0000000000000000
    };
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            if ((font_five[row] >> (15 - col)) & 1) {
                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b, 255);
            }
        }
    }
}

void draw_six(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned short font_six[16] = {
        0b0000000000000000,
        0b0000000000000000,
        0b0000011111100000,
        0b0000110000010000,
        0b0001100000000000,
        0b0010000000000000,
        0b0010111111000000,
        0b0011000000110000,
        0b0010000000010000,
        0b0010000000001000,
        0b0010000000001000,
        0b0010000000001000,
        0b0011000000011000,
        0b0001100000110000,
        0b0000111111100000,
        0b0000000000000000
    };
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            if ((font_six[row] >> (15 - col)) & 1) {
                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b, 255);
            }
        }
    }
}

void draw_seven(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned short font_seven[16] = {
        0b0000000000000000,
        0b1111111111111111,
        0b0000000000011000,
        0b0000000000010000,
        0b0000000000100000,
        0b0000000001100000,
        0b0000000001000000,
        0b0000000010000000,
        0b0000000110000000,
        0b0000000100000000,
        0b0000000100000000,
        0b0000000100000000,
        0b0000001100000000,
        0b0000001000000000,
        0b0000001000000000,
        0b0000001000000000
    };
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            if ((font_seven[row] >> (15 - col)) & 1) {
                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b, 255);
            }
        }
    }
}

void draw_eight(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned short font_eight[16] = {
        0b0000000000000000,
        0b0000001111000000,
        0b0000010000100000,
        0b0000100000010000,
        0b0000100000010000,
        0b0000100000010000,
        0b0000010000100000,
        0b0000001111000000,
        0b0000010000100000,
        0b0000100000010000,
        0b0000100000010000,
        0b0000100000010000,
        0b0000100000010000,
        0b0000010000100000,
        0b0000001111000000,
        0b0000000000000000
    };
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            if ((font_eight[row] >> (15 - col)) & 1) {
                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b, 255);
            }
        }
    }
}

void draw_nine(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
    unsigned short font_nine[16] = {
    0b0000000000000000,
    0b0000000000000000,
    0b0000000111000000,
    0b0000001000100000,
    0b0000001000100000,
    0b0000001000100000,
    0b0000001001100000,
    0b0000000111100000,
    0b0000000000100000,
    0b0000000000100000,
    0b0000000000100000,
    0b0000000000100000,
    0b0000001001000000,
    0b0000000110000000,
    0b0000000000000000,
    0b0000000000000000,
    };
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 16; col++) {
            if ((font_nine[row] >> (15 - col)) & 1) {
                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b, 255);
            }
        }
    }
}
