#include "gui.hpp"
#include "window.hpp"

static Window win[3] = {
    Window(0, 0, 0, 0, false),
    Window(0, 0, 0, 0, false),
    Window(0, 0, 0, 0, false)
};
static StartButton start_button(0, 0, 0, 0);
extern "C" {
    #include <kernel.h>

    extern unsigned int* back_buffer32;
    void swap_buffers();
    void cpp_init_windows() {
        win[0] = Window(450, 400, 450, 200, 1);
        win[1] = Window(450, 300, 400, 250, 0);
        win[2] = Window(900, 600, 300, 150, 0);

        start_button = StartButton(20, 979, 35, 35);
    }

    void cpp_draw_windows(unsigned char* video_memory) {
        for (int i = 0; i < 3; i++) {
            win[i].draw(video_memory);
        }
    }

    void cpp_draw_taskbar_widgets(unsigned char* back_buffer) {
        start_button.draw(back_buffer);
    }

    extern "C" int cpp_handle_mouse_hover(int mouse_x, int mouse_y) {
        static int last_hover_state = -1; 
        int current_hover = start_button.check_hover(mouse_x, mouse_y);
        
        if (current_hover != last_hover_state) {
            last_hover_state = current_hover;
            
            start_button.draw((unsigned char*)back_buffer32); 
            
            return 1;
        }
        return 0;
    }


    void cpp_draw_single_window(unsigned char* video_memory, int idx) {
        if (idx >= 0 && idx < 3) {
            win[idx].draw(video_memory);
        }
    }

    void cpp_set_window_visible(int idx, int visible) {
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