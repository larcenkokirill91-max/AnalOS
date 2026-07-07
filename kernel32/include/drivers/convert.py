import os

def unpack_bin_to_c_source(input_file, output_file):
    current_dir = os.path.dirname(os.path.abspath(__file__))
    input_path = os.path.join(current_dir, input_file)
    output_path = os.path.join(current_dir, output_file)

    if not os.path.exists(input_path):
        print(f"Ошибка: файл {input_path} не найден!")
        return

    with open(input_path, 'rb') as f:
        data = f.read()

    glyph_size = 12 * 6
    total_glyphs = len(data) // glyph_size

    out_lines = [
        "#include <stdint.h>",
        "",
        "// Автоматически сгенерированный шрифт. Отредактируйте матрицы",
        "// и запустите pack_font.py для сборки обратно в .bin файл.",
        ""
    ]

    for glyph_idx in range(total_glyphs):
        start = glyph_idx * glyph_size
        glyph_bytes = data[start:start + glyph_size]
        
        out_lines.append(f"const unsigned char font_char_{glyph_idx}[12][6] = {{")
        
        for row in range(12):
            row_bytes = glyph_bytes[row*6 : (row+1)*6]
            bytes_str = ", ".join(str(b) for b in row_bytes)
            
            comment = "".join('.' if b == 0 else 'X' for b in row_bytes)
            
            comma = "," if row < 11 else ""
            out_lines.append(f"  {{{bytes_str}}}{comma} // {comment}")
            
        out_lines.append("};")
        out_lines.append("")

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write("\n".join(out_lines))
        
    print(f"Успешно! Текстовый исходник сохранен в: {output_path}")
    print(f"Распаковано символов: {total_glyphs}")

if __name__ == "__main__":
    unpack_bin_to_c_source("font.bin", "font_editable.h")