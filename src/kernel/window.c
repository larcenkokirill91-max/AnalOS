#include "window.h"
#include "../../drivers/screen/screen.h"

#define WIN_HH 25
#define WIN_WW 5

void draw_window(unsigned char* video_memory, struct window* win) {
	draw_rect(video_memory, win->lt.x + 1, win->lt.y + 1, win->width - 1, WIN_HH - 2, 0, 0, 0);
	draw_rect(video_memory, win->lt.x + 2, win->lt.y , win->width - 3, WIN_HH - 1, 0, 0, 0);
	draw_rect(video_memory, win->lt.x, win->lt.y + 2, win->width, WIN_HH - 3, 0, 0, 0);
	draw_rect(video_memory, win->lt.x, win->lt.y + WIN_HH, win->width, win->height - WIN_WW, 255, 255, 255);
}
