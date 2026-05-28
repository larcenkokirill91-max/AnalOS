extern unsigned int screen_pitch;
void draw_rect(unsigned char* video_memory, int start_x, int start_y, int width, int height, unsigned char r, unsigned char g, unsigned char b) {
	for (int y = start_y; y < start_y + height; y++) {
		for (int x = start_x; x < start_x + width; x++) {
			int offset = (y * screen_pitch) +(x * 4);
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
				draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b);
			}
		}
	}
}
void draw_B(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
        unsigned char font_A[8] = {
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
                        if ((font_A[row] >> (7 - col)) & 1) {
                                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b);
                        }
                }
        }
}
void draw_C(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b) {
        unsigned char font_A[8] = {
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
                        if ((font_A[row] >> (7 - col)) & 1) {
                                draw_rect(video_memory, start_x + col, start_y + row, 1, 1, r, g, b);
                        }
                }
        }
}
