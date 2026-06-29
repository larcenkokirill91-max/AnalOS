#pragma once
#ifndef CPP
#define CPP
#include "../../../gui/gui.hpp"
void cpp_init_windows();
void cpp_draw_windows(unsigned char* video_memory);
void cpp_draw_single_window(unsigned char* video_memory, int idx);
void cpp_set_window_visible(int idx, int visible);
int cpp_get_window_is_visible(int idx);
void cpp_set_window_position(int idx, int new_x, int new_y);
int cpp_get_window_x(int idx);
int cpp_get_window_y(int idx);
int cpp_get_window_width(int idx);
int cpp_get_window_height(int idx);
#endif