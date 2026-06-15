#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

// Включаем библиотеку чтения картинок
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct RlePixel {
    unsigned short count;
    unsigned int color;
};

int main() {
    int width, height, channels;
    
    // Загружаем картинку и принудительно заставляем её прочитаться в 3 канала (RGB)
    unsigned char* img_data = stbi_load("logo.png", &width, &height, &channels, 3);
    
    if (!img_data) {
        std::cerr << "[ОШИБКА] Не удалось открыть logo.png в папке!" << std::endl;
        return 1;
    }

    std::vector<RlePixel> rle_pairs;

    // Сжимаем картинку СТРОГО ПОСТРОЧНО
    for (int y = 0; y < height; y++) {
        int row_start_pixel = y * width;
        
        // Берем первый пиксель строки
        unsigned char r = img_data[(row_start_pixel * 3) + 0];
        unsigned char g = img_data[(row_start_pixel * 3) + 1];
        unsigned char b = img_data[(row_start_pixel * 3) + 2];
        unsigned int current_color = (r << 16) | (g << 8) | b;
        unsigned short current_count = 1;

        for (int x = 1; x < width; x++) {
            int pixel_idx = row_start_pixel + x;
            unsigned char pr = img_data[(pixel_idx * 3) + 0];
            unsigned char pg = img_data[(pixel_idx * 3) + 1];
            unsigned char pb = img_data[(pixel_idx * 3) + 2];
            unsigned int pixel_color = (pr << 16) | (pg << 8) | pb;


            if (pixel_color == current_color && current_count < 65535) {
                current_count++;
            } else {
                rle_pairs.push_back({current_count, current_color});
                current_color = pixel_color;
                current_count = 1;
            }
        }
        // Записываем последнюю серию в строке
        rle_pairs.push_back({current_count, current_color});
    }

    // Сохраняем результат в файл для твоей ОС
    std::ofstream out("src/kernel/wallpaper.h");
    out << "#ifndef WALLPAPER_H\n#define WALLPAPER_H\n\n";
    out << "#define WALLPAPER_COUNT " << (unsigned int)rle_pairs.size() << "\n\n";
    out << "struct rle_pixel {\n    unsigned short count;\n    unsigned int color;\n} __attribute__((packed));\n\n";
    out << "struct rle_pixel wallpaper_data[] = {\n";

    for (const auto& pair : rle_pairs) {
        out << "    {" << std::dec << pair.count << ", 0x" 
            << std::hex << std::setw(8) << std::setfill('0') << std::uppercase << pair.color << "},\n";
    }
    out << "};\n\n#endif\n";

    stbi_image_free(img_data);
    std::cout << "[C++] УСПЕХ! Картинка пожата построчно в " << rle_pairs.size() << " RLE-блоков." << std::endl;
    return 0;
}
