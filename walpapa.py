import os
from PIL import Image

def image_to_c_array(input_file, output_h_file, width=1280, height=1024):
    current_dir = os.path.dirname(os.path.abspath(__file__))
    input_path = os.path.join(current_dir, input_file)
    output_path = os.path.join(current_dir, output_h_file)

    if not os.path.exists(input_path):
        print(f"Ошибка: {input_path} не найден!")
        return

    img = Image.open(input_path).resize((width, height)).convert("RGBA")
    pixels = list(img.getdata())

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write("#ifndef WALLPAPER_H\n#define WALLPAPER_H\n\n")
        f.write(f"#define WALLPAPER_WIDTH {width}\n")
        f.write(f"#define WALLPAPER_HEIGHT {height}\n\n")
        f.write("static const unsigned char wallpaper_data[] = {\n    ")
        
        for i, (r, g, b, a) in enumerate(pixels):
            f.write(f"0x{r:02X},0x{g:02X},0x{b:02X},0x{a:02X},")
            if (i + 1) % 6 == 0:
                f.write("\n    ")
                
        f.write("\n};\n\n#endif // WALLPAPER_H\n")
    print(f"Готово! Си-массив сохранен в: {output_path}")

if __name__ == "__main__":
    image_to_c_array("wallpaper.png", "wallpaper.h")
