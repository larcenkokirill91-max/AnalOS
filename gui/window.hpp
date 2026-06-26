#pragma once

class Window {
private:
    int x, y;
    int width, height;
    bool is_visible;

public:
    Window(int start_x, int start_y, int w, int h, bool visible);
    void set_position(int new_x, int new_y);
    void draw(unsigned char* video_memory);
    void set_visible(bool visible);

    int get_x();
    int get_y();
    int get_width();
    int get_height();
    bool get_visible();
};