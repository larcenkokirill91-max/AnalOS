#pragma once
extern unsigned char current_scancode;
extern unsigned int mouse_bg_buffer[256];
extern int alt_pressed;
extern int menu_open;
extern int mouse_x;
extern int mouse_y;
extern int mouse_cycle;
extern signed char mouse_packet[3];
extern unsigned char data;
extern int h, m, s;
extern int last_s;
extern int last_m;
extern int last_h;
extern int start_menu_open;
extern int dragged_window_idx;
extern int is_mouse_pressed;
