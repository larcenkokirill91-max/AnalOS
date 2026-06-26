#ifndef KEYBOARD_H
#define KEYBOARD_H

extern volatile unsigned int timer_ticks;

int keyboard_has_data(void);
int mouse_has_data(void);
void clear_keyboard_flag(void);
void clear_mouse_flag(void);
void idt_init(void);

#endif
