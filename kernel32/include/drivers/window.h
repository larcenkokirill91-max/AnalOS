#pragma once
#ifndef WINDOW_H
#define WINDOW_H
struct Point {
	int x;
	int y;};

void draw_xor_frame(unsigned int* vram, int x, int y, int width, int height, int pitch_dw);
#endif