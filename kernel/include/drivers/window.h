#pragma once
#ifndef WINDOW_H
#define WINDOW_H
struct Point {
	int x;
	int y;};
struct window {
	unsigned int width;
	unsigned int height;
	struct Point lt;
	unsigned char is_visible;
	unsigned char resizable;
	unsigned char is_dragging;};
void draw_window(unsigned char* video_memory, struct window* win);
void draw_xor_frame(unsigned int* vram, int x, int y, int width, int height, int pitch_dw);
#endif
