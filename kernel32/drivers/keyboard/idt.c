#include <kernel.h>
__attribute__((aligned(4096))) volatile unsigned int timer_ticks = 0; 
unsigned char current_scancode = 0;

signed char mouse_packet[3];

static volatile int kbd_ready = 0;
static volatile int mouse_ready = 0;
int mouse_cycle = 0;

struct idt_entry idt[256];

int keyboard_has_data(void) {
    __asm__ __volatile__("" : : : "memory");
    return kbd_ready;
}

int mouse_has_data(void) {
    __asm__ __volatile__("" : : : "memory");
    return mouse_ready;
}

void clear_keyboard_flag(void) {
    kbd_ready = 0;
}

void clear_mouse_flag(void) {
    mouse_ready = 0;
}

#ifdef __cplusplus
extern "C" {
#endif

void timer_handler_c(void) {
    timer_ticks++;
    outb(0x20, 0x20);
}

void keyboard_handler_c(void) {
    while (inb(0x64) & 1) {
        current_scancode = inb(0x60);
        kbd_ready = 1;
    }
    outb(0x20, 0x20);
}

void mouse_handler_c(void) {
    unsigned char status = inb(0x64);
    
    if ((status & 0x21) == 0x21) {
        unsigned char data = inb(0x60);
        
        if (mouse_cycle == 0 && (data & 0x08) == 0) {
        } else {
            mouse_packet[mouse_cycle] = (signed char)data;
            mouse_cycle++;
            
            if (mouse_cycle == 3) {
                mouse_cycle = 0;
                mouse_ready = 1;
            }
        }
    }
    
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

void default_handler_c(void) {
    outb(0x20, 0x20);
}

#ifdef __cplusplus
}
#endif

void idt_set_gate(int num, unsigned int handler, unsigned short selector, unsigned char flags) {
    idt[num].offset_lower = handler & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].flags = flags;
    idt[num].offset_higher = handler >> 16;
}

void pic_remap(void) {
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();
    outb(0x21, 0x20); io_wait();
    outb(0xA1, 0x28); io_wait();
    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();
    
    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);
}

void mouse_wait_command(void) {
    while ((inb(0x64) & 2) != 0);
}

void mouse_wait_data(void) {
    while ((inb(0x64) & 1) == 0);
}

void mouse_write(unsigned char data) {
    mouse_wait_command();
    outb(0x64, 0xD4);
    io_wait();
    mouse_wait_command();
    outb(0x60, data);
    io_wait();
}

void mouse_init(void) {
    unsigned char status;

    mouse_wait_command();
    outb(0x64, 0xA8); 

    mouse_wait_command();
    outb(0x64, 0x20);
    mouse_wait_data();
    status = inb(0x60);

    status |= 0x02;
    status &= ~0x20;

    mouse_wait_command();
    outb(0x64, 0x60);
    mouse_wait_command();
    outb(0x60, status);

    mouse_write(0xF6);
    mouse_wait_data();
    inb(0x60);

    mouse_write(0xF4);
    mouse_wait_data();
    inb(0x60);
}

extern void timer_asm_handler(void);
extern void keyboard_asm_handler(void);
extern void mouse_asm_handler(void);
extern void default_asm_handler(void);

__attribute__((aligned(16))) struct idt_ptr ptr;
void idt_init(void) {
    pic_remap();
    
    ptr.long_idt = (sizeof(struct idt_entry) * 256) - 1;
    ptr.addres = (unsigned int)&idt;
    
    for (int num = 0; num < 256; num++) {
        idt_set_gate(num, (unsigned int)default_asm_handler, 0x08, 0x8E);
    }
    
    idt_set_gate(32, (unsigned int)timer_asm_handler, 0x08, 0x8E);    // IRQ0
    idt_set_gate(33, (unsigned int)keyboard_asm_handler, 0x08, 0x8E); // IRQ1
    idt_set_gate(44, (unsigned int)mouse_asm_handler, 0x08, 0x8E);    // IRQ12
    
    __asm__ __volatile__("lidt (%0)" : : "r"(&ptr));
    __asm__ __volatile__("sti"); 
}
