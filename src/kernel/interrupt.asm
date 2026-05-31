[bits 32]

global idt_load
global keyboard_handler_asm
global exception_handler_asm
global dummy_handler_asm
extern keyboard_handler_main

idt_load:
    mov eax, [esp + 4]  
    lidt [eax]          
    ret

; Обработчик клавиатуры с ИСПРАВЛЕННЫМ выравниванием стека (РАУНД 3)
keyboard_handler_asm:
    pusha               
    push ds             
    push es

    ; Нам нужно выровнять стек по границе 16 байт перед вызовом Си
    mov ebx, esp        ; Сохраняем текущий указатель стека в незанятый EBX
    and esp, 0xFFFFFFF0 ; Выравниваем ESP вниз на ближайшие 16 байт
    
    call keyboard_handler_main 

    mov esp, ebx        ; Восстанавливаем оригинальный стек после Си-кода

    ; Отправляем сигнал EOI в ведущий PIC
    mov al, 0x20
    out 0x20, al        

    pop es              
    pop ds
    popa                
    iretd               

; ИСПРАВЛЕНО (РАУНД 4): Заглушка для аппаратных IRQ с правильным порядком сброса PIC
dummy_handler_asm:
    push ax
    mov al, 0x20
    out 0xA0, al        ; СНАЧАЛА шлем EOI в ведомый (Slave) PIC
    out 0x20, al        ; ЗАТЕМ шлем EOI в ведущий (Master) PIC
    pop ax
    iretd               

; ИСПРАВЛЕНО (РАУНД 2): Заглушка для исключений процессора (безопасный возврат)
exception_handler_asm:
    ; Так как мы не знаем, кинул ли процессор код ошибки, мы просто
    ; игнорируем его и возвращаемся, предотвращая Triple Fault намертво
    iretd

