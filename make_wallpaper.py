import os
from PIL import Image

def convert_image_to_rgba_bin(input_file, output_file, width=1280, height=1024):
    current_dir = os.path.dirname(os.path.abspath(__file__))
    input_path = os.path.join(current_dir, input_file)
    output_path = os.path.join(current_dir, output_file)

    if not os.path.exists(input_path):
        print(f"Ошибка: исходный файл {input_path} не найден!")
        return

    try:
        # Открываем изображение и приводим к нужному разрешению и формату RGBA
        img = Image.open(input_path)
        img = img.resize((width, height))
        img_rgba = img.convert("RGBA")
        
        # Получаем плоский список всех пикселей [(r,g,b,a), (r,g,b,a), ...]
        pixels = list(img_rgba.getdata())
        
        output_bytes = bytearray()
        for r, g, b, a in pixels:
            # Записываем каждый канал (R, G, B, A) как 1 байт (0-255)
            output_bytes.extend([r, g, b, a])
            
        with open(output_path, 'wb') as f:
            f.write(output_bytes)
            
        print(f"Успешно! Изображение упаковано в бинарник: {output_path}")
        # Ожидаемый размер: 1280 * 1024 * 4 байта = 5 242 880 байт
        print(f"Итоговый размер: {len(output_bytes)} байт.")

    except Exception as e:
        print(f"Произошла ошибка при конвертации: {e}")

if __name__ == "__main__":
    # Замените 'wallpaper.jpg' на имя вашего исходного файла
    convert_image_to_rgba_bin("wallpaper.png", "wallpaper_rgba.bin")