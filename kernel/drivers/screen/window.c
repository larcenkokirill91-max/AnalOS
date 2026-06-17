#include <kernel.h>
#include <kernel.h>
#define WIN_HH 25
#define WIN_WW 5
void draw_window(unsigned char* video_memory, struct window* win) {
	draw_rect(video_memory, win->lt.x + 1, win->lt.y + 1, win->width - 1, WIN_HH - 2, 0, 0, 0, 255);
	draw_rect(video_memory, win->lt.x + 2, win->lt.y , win->width - 3, WIN_HH - 1, 0, 0, 0, 255);
	draw_rect(video_memory, win->lt.x, win->lt.y + 2, win->width, WIN_HH - 3, 0, 0, 0, 255);
	draw_rect(video_memory, win->lt.x, win->lt.y + WIN_HH, win->width, win->height - WIN_WW, 255, 255, 255, 255);}
void draw_xor_frame(unsigned int* vram, int x, int y, int width, int height, int pitch_dw) {
    for (int i = 0; i < width; i++) {
        int idx_top = y * pitch_dw + (x + i);
        int idx_bottom = (y + height - 1) * pitch_dw + (x + i);
        vram[idx_top] ^= 0x00FFFFFF;
        vram[idx_bottom] ^= 0x00FFFFFF;
    }
    for (int j = 1; j < height - 1; j++) {
        int idx_left = (y + j) * pitch_dw + x;
        int idx_right = (y + j) * pitch_dw + (x + width - 1);
        vram[idx_left] ^= 0x00FFFFFF;
        vram[idx_right] ^= 0x00FFFFFF;
    }
}