typedef struct {
    unsigned int* framebuffer;
    unsigned int width;
    unsigned int height;
} BootInfo;

void init_screen_driver(BootInfo* info);
void fill_screen(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void draw_taskbar(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int rad, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void swap_buffers(void* gop);

// Меняем атрибут на ms_abi, чтобы загрузчик смог передать аргумент 'info'
void __attribute__((ms_abi)) kernel_main(BootInfo* info) {
    if (!info) {
        while(1) { __asm__ __volatile__("hlt"); }
    }

    // Инициализируем физический адрес экрана
    init_screen_driver(info);

    // Рисуем в Back Buffer
    fill_screen(20, 30, 50, 255);
    draw_taskbar(50, 700, 923, 40, 8, 255, 255, 255, 200);

    // Копируем Back Buffer на реальный экран
    swap_buffers(0);

    volatile int keep_running = 1;
    while (keep_running) {
        __asm__ __volatile__("hlt");
    }
}
