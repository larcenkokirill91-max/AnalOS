import os
import re

def pack_c_source_to_bin(input_file, output_file):
    current_dir = os.path.dirname(os.path.abspath(__file__))
    input_path = os.path.join(current_dir, input_file)
    output_path = os.path.join(current_dir, output_file)

    if not os.path.exists(input_path):
        print(f"Ошибка: исходный текстовый файл {input_path} не найден!")
        return

    with open(input_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Регулярное выражение ищет внутренности строк вида {0, 2, 4, 0, 0, 0}
    # Игнорирует комментарии благодаря очистке или строгому соответствию
    row_pattern = re.compile(r'\{\s*([0-9\s,\s]+)\s*\}')
    
    output_bytes = bytearray()
    
    # Удаляем однострочные комментарии перед поиском, чтобы они случайно не повлияли
    clean_content = re.sub(r'//.*', '', content)
    
    rows = row_pattern.findall(clean_content)
    
    for row_str in rows:
        # Разделяем по запятой и превращаем в целые числа
        bytes_list = [int(x.strip()) for x in row_str.split(',') if x.strip().isdigit()]
        if len(bytes_list) == 6:
            output_bytes.extend(bytes_list)

    if len(output_bytes) % 72 != 0 or len(output_bytes) == 0:
        print(f"Предупреждение: Считано байт: {len(output_bytes)}. Это не кратно размеру символа (72 байта)!")
        print("Проверьте правильность расстановки фигурных скобок в измененном файле.")
        return

    with open(output_path, 'wb') as f:
        f.write(output_bytes)
        
    print(f"Успешно! Изменения упакованы в бинарник: {output_path}")
    print(f"Итоговый размер: {len(output_bytes)} байт ({len(output_bytes) // 72} символов).")

if __name__ == "__main__":
    pack_c_source_to_bin("font_editable.h", "font.bin")