#include "start_menu.h"
#include "../drivers/screen/screen.h"
void draw_start(unsigned char* video_memory, int start_x, int start_y) {
	draw_rect(video_memory, start_x, start_y, 5, 5, 96, 205, 255);
	for (int i = 0; i < 4; i++) {
		start_x += 5;
		start_y += 5;
		draw_rect(video_memory, start_x, start_y, 5, 5, 96, 205, 255);}
	start_x -= 20;
	start_y -= 20;
	for (int i = 0; i < 4; i++) {
		start_x -= 5;
		start_y += 5;
		draw_rect(video_memory, start_x, start_y, 5, 5, 96, 205, 255);}
	start_x += 10;
        start_y -= 5;
	draw_rect(video_memory, start_x, start_y, 5, 5, 96, 205, 255);
	for (int i = 0; i < 4; i++) {
		start_x += 5;
		draw_rect(video_memory, start_x, start_y, 5, 5, 96, 205, 255);}
	start_x -= 10;
        start_y += 5;
	draw_rect(video_memory, start_x, start_y, 5, 5, 96, 205, 255);
	start_y -= 20;
	start_x += 1;
	draw_rect(video_memory, start_x, start_y, 5, 5, 0, 120, 212);
	for (int i = 0; i < 4; i++) {
		start_x += 5;
		start_y += 5;
		draw_rect(video_memory, start_x, start_y, 5, 5, 0, 120, 212);}
	start_x -= 20;
	start_y -= 20;
	for (int i = 0; i < 4; i++) {
		start_x -= 5;
		start_y += 5;
		draw_rect(video_memory, start_x, start_y, 5, 5, 0, 120, 212);}
	start_x += 10;
        start_y -= 5;
	draw_rect(video_memory, start_x, start_y, 5, 5, 0, 120, 212);
	for (int i = 0; i < 4; i++) {
		start_x += 5;
		draw_rect(video_memory, start_x, start_y, 5, 5, 0, 120, 212);}
	start_x -= 10;
        start_y += 5;
        draw_rect(video_memory, start_x, start_y, 5, 5, 96, 205, 255);}