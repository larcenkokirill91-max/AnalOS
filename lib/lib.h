#ifndef LIB_H
#define LIB_H

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, int n);
void strcpy(char *dest, const char *src);
void print_at(const char *str, unsigned char color, int x, int y);
void clear_screen();
void draw_pixel(int index, unsigned int color);
static inline void outl(unsigned short port, unsigned int val);
static inline unsigned int inl(unsigned short port);
#endif

