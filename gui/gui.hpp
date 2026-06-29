#pragma once

class StartButton{
private:
    int x, y;
    int width, height; // Перенесли сюда (все int идут подряд)
    int is_hovered;    // Заменили bool на int (теперь все поля по 4 байта!)
public:
    StartButton(int start_x, int start_y, int width, int height);
    
    void draw(unsigned char* back_buffer);
    int check_hover(int mouse_x, int mouse_y); // Заменили bool на int
};
