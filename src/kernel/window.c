#include "window.h"
#include "../../drivers/screen/screen.h"

#define WIN_HH 25
#define WIN_WW 5

void draw_window(unsigned char* video_memory, struct window* win) {
	draw_rect(video_memory, win->lt.x, win->lt.y, win->width, WIN_HH, 0, 0, 0);
	draw_rect(video_memory, win->lt.x, win->lt.y + WIN_HH, WIN_WW, win->height, 255, 255, 255);
	draw_rect(video_memory, win->rt.x - WIN_WW, win->rt.y + WIN_HH, WIN_WW, win->height, 255, 255, 255);
	draw_rect(video_memory, win->lb.x, win->lb.y + 20, win->width, WIN_WW, 255, 255, 255);
	draw_rect(video_memory, win->lt.x + WIN_WW, win->lt.y + WIN_HH, win->width - WIN_WW, win->height - WIN_WW, 255, 255, 255);
}
