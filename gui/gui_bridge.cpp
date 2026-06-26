#include "window.hpp"

static Window win[3] = {
    Window(0, 0, 0, 0, false),
    Window(0, 0, 0, 0, false),
    Window(0, 0, 0, 0, false)
};

extern "C" {
    void cpp_init_windows() {
        win[0] = Window(450, 400, 450, 200, true);
        win[1] = Window(450, 300, 400, 250, false);
        win[2] = Window(900, 600, 300, 150, false);
    }

    void cpp_draw_windows(unsigned char* video_memory) {
        for (int i = 0; i < 3; i++) {
            win[i].draw(video_memory);
        }
    }

    void cpp_draw_single_window(unsigned char* video_memory, int idx) {
        if (idx >= 0 && idx < 3) {
            win[idx].draw(video_memory);
        }
    }

    void cpp_set_window_visible(int idx, bool visible) {
        if (idx >= 0 && idx < 3) {
            win[idx].set_visible(visible);
        }
    }

    int cpp_get_window_is_visible(int idx) {
        if (idx >= 0 && idx < 3) {
            return win[idx].get_visible();
        }
        return 0;
    }

    void cpp_set_window_position(int idx, int new_x, int new_y) {
        if (idx >= 0 && idx < 3) {
            win[idx].set_position(new_x, new_y);
        }
    }

    int cpp_get_window_x(int idx) { 
        return win[idx].get_x(); 
    }
    
    int cpp_get_window_y(int idx) { 
        return win[idx].get_y(); 
    }
    
    int cpp_get_window_width(int idx) { 
        return win[idx].get_width(); 
    }
    
    int cpp_get_window_height(int idx) { 
        return win[idx].get_height(); 
    }
}