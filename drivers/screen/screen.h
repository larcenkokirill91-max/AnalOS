#ifndef SCREEN_H
#define SCREEN_H

void draw_rect(unsigned char* video_memory, int start_x, int start_y, int width, int height, unsigned char r, unsigned char g, unsigned char b);
void draw_A(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b);
void draw_B(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b);
void draw_C(unsigned char* video_memory, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b);
void draw_char(unsigned char* video_memory, char c, int start_x, int start_y, unsigned char r, unsigned char g, unsigned char b);

#endif
