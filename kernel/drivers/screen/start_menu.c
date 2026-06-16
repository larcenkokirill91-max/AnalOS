#include <kernel.h>
#include <kernel.h>
void draw_start(unsigned char* video_memory, int start_x, int start_y, int r, int g, int b) {
	start_x -= 15;
	draw_rect(video_memory, start_x, start_y, 15, 15, r, g, b, 255);
	start_x += 17;
	draw_rect(video_memory, start_x, start_y, 15, 15, r, g, b, 255);
	start_x -= 17;
	start_y += 17;
	draw_rect(video_memory, start_x, start_y, 15, 15, r, g, b, 255);
	start_x += 17;
	draw_rect(video_memory, start_x, start_y, 15, 15, r, g, b, 255);
}