void print_at(const char *str, unsigned char color, int x, int y) {
        unsigned short *screen = (unsigned short *)0xB8000;
        int pos = (y * 80) + x;
        for (int i = 0; str[i] != '\0'; i++) {
                screen[pos + i] = (color << 8) | '_';
        }
}
void test_pixel() {
        unsigned short *screen = (unsigned short *)0xB8000;
        screen[80] = (0x02 << 8) | 'a';
}
void fill_line(unsigned char color, int start_pos) {
        unsigned short *screen = (unsigned short *)0xB8000;
        for (int i = 0; i < 10; i++) {
                screen[start_pos + i] = (color << 8) | '_';
        }
}
void k_main() {
        print_at("CENTER", 0x0A, 35, 12);
        while(1);
}
