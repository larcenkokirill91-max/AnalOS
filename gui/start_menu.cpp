#include "gui.hpp"
#include "window.hpp"

extern "C" {
    #include <kernel.h>
}

StartButton::StartButton (int x, int y, int width, int height) {
    is_hovered = false;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
}

int StartButton::check_hover (int mouse_x, int mouse_y) {
    if (mouse_x >= x && mouse_x <= (x + width) && mouse_y >= y && mouse_y <= (y + height)) {
        is_hovered = 1;
    } else { 
        is_hovered = 0;
    }
    return is_hovered;
}

void StartButton::draw(unsigned char* back_buffer) {
    if (!back_buffer) return;                     
    
    // hover = (45, 165, 255), not hover = (0, 120, 212)
    int r = is_hovered ? 45  : 0;
    int g = is_hovered ? 165 : 120;
    int b = is_hovered ? 255 : 212;

    unsigned char ur = (unsigned char)r;
    unsigned char ug = (unsigned char)g;
    unsigned char ub = (unsigned char)b;
    unsigned int* buf32 = (unsigned int*)back_buffer;
    draw_rect(back_buffer, x - 15, y,      15, 15, ur, ug, ub, 255);
    draw_rect(back_buffer, x + 2,  y,      15, 15, ur, ug, ub, 255);
    draw_rect(back_buffer, x - 15, y + 17, 15, 15, ur, ug, ub, 255);
    draw_rect(back_buffer, x + 2,  y + 17, 15, 15, ur, ug, ub, 255);
}