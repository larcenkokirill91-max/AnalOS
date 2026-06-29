#pragma once

class Window {
private:
    int x, y;
    int width, height;
    int is_visible; // Заменили bool на int

public:
    Window(int start_x, int start_y, int w, int h, int visible); // Заменили bool на int
    void set_position(int new_x, int new_y);
    void draw(unsigned char* video_memory);
    void set_visible(int visible); // Заменили bool на int

    int get_x();
    int get_y();
    int get_width();
    int get_height();
    int get_visible(); // Заменили bool на int
};
