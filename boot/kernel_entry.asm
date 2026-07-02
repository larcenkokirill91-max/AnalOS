[bits 32]
[extern kernel_main]
[extern __init_array_start]
[extern __init_array_end]

[extern timer_handler_c]
[extern keyboard_handler_c]
[extern mouse_handler_c]
[extern default_handler_c]

global _start
global timer_asm_handler
global keyboard_asm_handler
global mouse_asm_handler
global default_asm_handler

; Селектор сегмента данных ядра из твоей GDT (DATA_SEG equ 0x10)
%define KERNEL_DATA_SEG 0x10

section .text.entry

_start:
    ; Жесткая настройка сегментов, чтобы C++ код GUI видел глобальные переменные
    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, esp
    and esp, 0xFFFFFFF0 ; Выравнивание стека по стандарту System V ABI для GCC

    ; Вызываем глобальные конструкторы С++ оконного интерфейса
    mov ebx, __init_array_start
.init_loop:
    cmp ebx, __init_array_end
    je .done_init
    
    mov eax, [ebx]
    test eax, eax
    jz .skip_ctor
    call eax            
.skip_ctor:
    add ebx, 4
    jmp .init_loop
    
.done_init:
    call kernel_main
    jmp $

; Безопасные обработчики для чипсетов Intel
timer_asm_handler:
    pushad
    cld
    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    call timer_handler_c
    popad
    sti
    iretd

keyboard_asm_handler:
    pushad
    cld
    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    call keyboard_handler_c
    popad
    iretd

mouse_asm_handler:
    pushad
    cld
    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    call mouse_handler_c
    popad
    iretd

default_asm_handler:
    pushad
    cld
    mov ax, KERNEL_DATA_SEG
    mov ds, ax
    mov es, ax
    call default_handler_c
    popad
    iretd