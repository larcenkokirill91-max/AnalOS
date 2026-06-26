#include <kernel.h>

extern unsigned int* global_video_memory;
extern unsigned int* back_buffer32;
extern int pitch_dw;

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
    (void)str; (void)color; (void)x; (void)y;
}

void clear_screen() {
    if (!back_buffer32) return;
    for (int i = 0; i < 1280 * 1024; i++) {
        back_buffer32[i] = 0x00000000;
    }
}

void draw_pixel(int index, unsigned int color) {
    if (global_video_memory != 0 && index >= 0 && index < 1280 * 1024) {
        global_video_memory[index] = color;
    }
}

void memcpy(void *dest, const void *src, int n) {
    unsigned char* d = (unsigned char *)dest;
    unsigned char* s = (unsigned char *)src;
    for (int i = 0; i < n; i++) {
        d[i] = s[i];
    }
}