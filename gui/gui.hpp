#pragma once

class StartButton{
private:
    int x, y;
    int width, height;
    int is_hovered;
public:
    StartButton(int start_x, int start_y, int width, int height);
    
    void draw(unsigned char* back_buffer);
    int check_hover(int mouse_x, int mouse_y);
};
