from PIL import Image

def convert_image_rle_final(image_path, output_h_path, width=1280, height=1024):
    # Открываем картинку и строго подгоняем под разрешение экрана
    img = Image.open(image_path).resize((width, height)).convert("RGB")
    
    rle_pairs = []
    
    # Идем строго построчно
    for y in range(height):
        row_pixels = []
        for x in range(width):
            # Извлекаем чистые цвета R, G, B для каждого пикселя
            r, g, b = img.getpixel((x, y))
            # Собираем в 32-битное число
            pixel_value = (r << 16) | (g << 8) | b
            row_pixels.append(pixel_value)
            
        # Сжимаем эту строку методом RLE
        current_color = row_pixels[0]
        current_count = 1
        
        for pixel in row_pixels[1:]:
            if pixel == current_color and current_count < 65535:
                current_count += 1
            else:
                rle_pairs.append((current_count, current_color))
                current_color = pixel
                current_count = 1
        rle_pairs.append((current_count, current_color))

    # Записываем готовый файл Си
    with open(output_h_path, "w") as f:
        f.write("#ifndef WALLPAPER_H\n#define WALLPAPER_H\n\n")
        f.write(f"#define WALLPAPER_COUNT {len(rle_pairs)}\n\n")
        f.write("struct rle_pixel {\n    unsigned short count;\n    unsigned int color;\n};\n\n")
        f.write("struct rle_pixel wallpaper_data[] = {\n")
        for count, color in rle_pairs:
            f.write(f"    {{{count}, 0x{color:08X}}},\n")
        f.write("};\n\n#endif\n")
        
    print(f"[УСПЕХ] Сжато построчно в {len(rle_pairs)} блоков.")

convert_image_rle_final("logo.png", "src/kernel/wallpaper.h")
