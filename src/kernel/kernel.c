#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

void kernel_main(unsigned int fb_address) {
    unsigned char* video_memory = (unsigned char*)fb_address;

    // 1. Очистка экрана темно-серым фоном (попиксельно)
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int offset = (y * SCREEN_WIDTH + x) * 3;
            video_memory[offset]     = 45; // Blue
            video_memory[offset + 1] = 40; // Green
            video_memory[offset + 2] = 40; // Red
        }
    }

    // 2. Отрисовка КРАСНОГО квадрата 100x100 (координаты X: 150-250, Y: 250-350)
    for (int y = 250; y < 350; y++) {
        for (int x = 150; x < 250; x++) {
            int offset = (y * SCREEN_WIDTH + x) * 3;
            video_memory[offset]     = 0;   // Blue
            video_memory[offset + 1] = 0;   // Green
            video_memory[offset + 2] = 255; // Red
        }
    }

    // 3. Отрисовка ЗЕЛЕНОГО квадрата 100x100 (координаты X: 350-450, Y: 250-350)
    for (int y = 250; y < 350; y++) {
        for (int x = 350; x < 450; x++) {
            int offset = (y * SCREEN_WIDTH + x) * 3;
            video_memory[offset]     = 0;   // Blue
            video_memory[offset + 1] = 255; // Green
            video_memory[offset + 2] = 0;   // Red
        }
    }

    // 4. Отрисовка СИНЕГО квадрата 100x100 (координаты X: 550-650, Y: 250-350)
    for (int y = 250; y < 350; y++) {
        for (int x = 550; x < 650; x++) {
            int offset = (y * SCREEN_WIDTH + x) * 3;
            video_memory[offset]     = 255; // Blue
            video_memory[offset + 1] = 0;   // Green
            video_memory[offset + 2] = 0;   // Red
        }
    }

    // Вечный останов процессора
    while (1) {
        __asm__ __volatile__("hlt");
    }
}

