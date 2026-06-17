#pragma once
#ifndef SCREEN_H
#define SCREEN_H

void draw_rect(unsigned char* video_memory, int start_x, int start_y, int width, int height, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void draw_os(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b);
void draw_cursor(unsigned char* video_memory, int start_x, int start_y);
void undraw_cursor(unsigned char* video_memory, int start_x, int start_y);
void draw_circle(unsigned char* video_memory, int center_x, int center_y, int radius, unsigned char r, unsigned char g, unsigned char b);
void swap_buffers(void);
void draw_time(unsigned char* back_buffer, unsigned char* video_memory);
void draw_restart(unsigned char* video_memory, int start_x, int start_y);
void draw_off(unsigned char* video_memory, int start_x, int start_y);
void draw_off_menu(unsigned char* back_buffer, int win_x, int win_y);

#endif
