import os
from PIL import Image, ImageDraw, ImageFont

# --- НАСТРОЙКИ ---
FONT_PATH = 'font.ttf'      # Путь к вашему TTF файлу
FONT_SIZE = 12              # Высота шрифта
GLYPH_WIDTH = 6             # Строгая ширина матрицы
GLYPH_HEIGHT = 12           # Строгая высота матрицы

# Проверяем наличие файла шрифта
if not os.path.exists(FONT_PATH):
    print(f"Ошибка: Файл {FONT_PATH} не найден в текущей директории!")
    exit(1)

try:
    font = ImageFont.truetype(FONT_PATH, FONT_SIZE)
except Exception as e:
    print(f"Не удалось загрузить шрифт: {e}")
    exit(1)

print("// Сгенерированные массивы шрифта по вашему TTF файлу")
print("#include <stdint.h>\n")

# Строго ОТ НУЛЯ до абсолютного максимума Unicode BMP (65535)
for i in range(0, 65536):
    char = chr(i)
    name = f"char_{i}"
    
    # Создаем пустое черно-белое изображение (черный фон)
    img = Image.new('L', (GLYPH_WIDTH, GLYPH_HEIGHT), 0)
    draw = ImageDraw.Draw(img)
    
    try:
        # Получаем границы символа для центрирования
        bbox = draw.textbbox((0, 0), char, font=font)
        text_w = bbox[2] - bbox[0]
        text_h = bbox[3] - bbox[1]
        
        offset_x = (GLYPH_WIDTH - text_w) // 2 - bbox[0]
        offset_y = (GLYPH_HEIGHT - text_h) // 2 - bbox[1]
        
        # Рисуем символ
        draw.text((offset_x, offset_y), char, fill=255, font=font)
    except Exception:
        # Если символ управляющий (0-31) или не поддерживается библиотекой,
        # оставляем матрицу пустой (все нули), чтобы не ломать генерацию
        pass
    
    # Генерируем C-код
    print(f"const unsigned char font_{name}[{GLYPH_HEIGHT}][{GLYPH_WIDTH}] = {{")
    for y in range(GLYPH_HEIGHT):
        row_values = []
        for x in range(GLYPH_WIDTH):
            gray = img.getpixel((x, y))
            if gray > 210:
                val = 1
            elif gray > 140:
                val = 2
            elif gray > 80:
                val = 3
            elif gray > 20:
                val = 4
            else:
                val = 0
            row_values.append(str(val))
            
        comma = ',' if y < GLYPH_HEIGHT - 1 else ''
        visual_row = "".join([('X' if int(v) > 0 else '.') for v in row_values])
        print(f"  {{{', '.join(row_values)}}}{comma} // {visual_row}")
        
    print("};\n")