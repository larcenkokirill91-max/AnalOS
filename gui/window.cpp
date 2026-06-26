#include "window.hpp"

extern "C" {
    #include <kernel.h>
}

Window::Window(int start_x, int start_y, int w, int h, bool visible) {
    x = start_x;
    y = start_y;
    width = w;
    height = h;
    is_visible = visible;
}

void Window::set_position(int new_x, int new_y) {
    x = new_x;
    y = new_y;
}

void Window::draw(unsigned char* video_memory) {
    constexpr int  win_hh = 25;
    constexpr int  win_ww = 5;

    if (!video_memory || !is_visible) return;

    draw_rect(video_memory, x + 1, y + 1, width - 1,  win_hh - 2, 0, 0, 0, 255);
    draw_rect(video_memory, x + 2, y , width - 3,  win_hh - 1, 0, 0, 0, 255);
    draw_rect(video_memory, x, y + 2, width,  win_hh - 3, 0, 0, 0, 255);
    
    draw_rect(video_memory, x, y +  win_hh, width,height -  win_ww, 255, 255, 255, 255);
}
void Window::set_visible(bool visible) {
    is_visible = visible;
}

int Window::get_x() { return x; }
int Window::get_y() { return y; }
int Window::get_width() { return width; }
int Window::get_height() { return height; }
bool Window::get_visible() { return is_visible; }