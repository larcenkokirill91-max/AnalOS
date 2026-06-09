#include "lib.h"
int strcmp(const char *s1, const char *s2) {
    int i = 0;
    while (s1[i] == s2[i]) {
        if (s1[i] == '\0') {
            return 0;
        }
        i++;
    }
    return s1[i] - s2[i];
}
int strncmp(const char *s1, const char *s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return s1[i] - s2[i]; 
        }
        if (s1[i] == '\0') {
            return 0; 
        }
    }
    return 0;
}
void strcpy(char *dest, const char *src) {
    int i = 0;
    while ((dest[i] = src[i]) != '\0') {
        i++;
    }
}
void print_at(const char *str, unsigned char color, int x, int y) {
    unsigned short *screen = (unsigned short *)0xB8000;
    int pos = (y * 80) + x;
    for (int i = 0; str[i] != '\0'; i++) {
        screen[pos + i] = (color << 8) | str[i];
    }
}
void clear_screen() {
    unsigned short *screen = (unsigned short *)0xB8000;
    unsigned short blank = (0x0F << 8) | ' '; 
    for (int i = 0; i < 80 * 25; i++) {
        screen[i] = blank;
    }
}
void draw_pixel(int index, unsigned int color) {
	unsigned int *fb = (unsigned int *)0xFD000000;
	fb[index] = color;
}
void (*draw_digit[10])(unsigned char*, int, int, unsigned char, unsigned char, unsigned char) = {
        draw_zero, draw_one, draw_two, draw_three, draw_four,
        draw_five, draw_six, draw_seven, draw_eight, draw_nine
};
