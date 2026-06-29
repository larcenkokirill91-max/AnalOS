[bits 32]
[extern kernel_main]
[extern __init_array_start]
[extern __init_array_end]

; Импортируем Си-функции обработчиков
[extern timer_handler_c]
[extern keyboard_handler_c]
[extern mouse_handler_c]
[extern default_handler_c]

; Экспортируем ассемблерные обертки для idt.c
global _start
global timer_asm_handler
global keyboard_asm_handler
global mouse_asm_handler
global default_asm_handler

section .text.entry

_start:
    mov ebp, esp
    and esp, 0xFFFFFFF0

    ; Вызываем глобальные конструкторы С++ вручную
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

; --- НАДЕЖНЫЕ ОБРАБОТЧИКИ ПРЕРЫВАНИЙ НА АСЕМБЛЕРЕ ---
timer_asm_handler:
    pushad
    cld
    call timer_handler_c
    popad
    iretd

keyboard_asm_handler:
    pushad
    cld
    call keyboard_handler_c
    popad
    iretd

mouse_asm_handler:
    pushad
    cld
    call mouse_handler_c
    popad
    iretd

default_asm_handler:
    pushad
    cld
    call default_handler_c
    popad
    iretd
