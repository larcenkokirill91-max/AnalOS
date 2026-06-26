import re
import os

# Укажите точный путь к вашему исходному файлу шрифта (где были font_char_XXXX)
INPUT_FILE = "kernel/include/drivers/font.h" 
OUTPUT_BIN = "kernel/include/drivers/font.bin"

GLYPH_ROWS = 12
GLYPH_COLS = 6
GLYPH_SIZE = GLYPH_ROWS * GLYPH_COLS  # 72 байта на один символ

print("[*] Чтение файла шрифта...")
if not os.path.exists(INPUT_FILE):
    print(f"[-] Ошибка: Файл {INPUT_FILE} не найден!")
    exit(1)

with open(INPUT_FILE, "r", encoding="utf-8", errors="ignore") as f:
    content = f.read()

print("[*] Поиск массивов символов...")
# Регулярное выражение ищет 'font_char_ЧИСЛО' и всё, что внутри фигурных скобок после него
pattern = re.compile(r'font_char_(\d+)\s*\[\s*\d*\s*\]\s*\[\s*\d*\s*\]\s*=\s*\{(.*?)\};', re.DOTALL)
matches = pattern.findall(content)

if not matches:
    print("[-] Символы не найдены. Убедитесь, что в файле остались конструкции вида const unsigned char font_char_XXX...")
    exit(1)

print(f"[+] Найдено символов: {len(matches)}")

# Находим максимальный ID, чтобы зарезервировать массив нужного размера
max_index = max(int(m[0]) for m in matches)
print(f"[+] Максимальный ID символа: {max_index}")

# Выделяем память под бинарник (заполняем нулями для пропущенных ID)
font_bytes = bytearray((max_index + 1) * GLYPH_SIZE)

for char_id_str, body in matches:
    char_id = int(char_id_str)
    
    # Удаляем комментарии // из тела массива
    body_clean = re.sub(r'//.*', '', body)
    
    # Извлекаем все числа (байты пикселей)
    digits = [int(x) for x in re.findall(r'\d+', body_clean)]
    
    if len(digits) == GLYPH_SIZE:
        offset = char_id * GLYPH_SIZE
        font_bytes[offset:offset+GLYPH_SIZE] = bytes(digits)
    else:
        print(f"[!] Предупреждение: Символ {char_id} имеет неверный размер ({len(digits)} байт вместо {GLYPH_SIZE})")

print(f"[*] Запись бинарного файла: {OUTPUT_BIN}")
with open(OUTPUT_BIN, "wb") as f:
    f.write(font_bytes)

print("[+] Скрипт успешно завершил работу!")