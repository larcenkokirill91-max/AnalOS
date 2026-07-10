[bits 64]

global keyboard_handler_asm
extern keyboard_handler_c

keyboard_handler_asm:
    ; Сохраняем все регистры общего назначения
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; Выравнивание стека по границе 16 байт и выделение Shadow Space (32 байта) под MS ABI
    mov rbp, rsp
    sub rsp, 40         ; 32 байта под shadow space + 8 байт для гарантии выравнивания
    and rsp, ~0xF       ; Жестко выравниваем указатель стека RSP

    call keyboard_handler_c

    ; Восстанавливаем оригинальное состояние стека перед извлечением регистров
    mov rsp, rbp

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    iretq

global mouse_handler_asm
extern mouse_handler_c

mouse_handler_asm:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rbp, rsp
    sub rsp, 40
    and rsp, ~0xF

    call mouse_handler_c

    mov rsp, rbp

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    iretq

global dummy_handler_asm
dummy_handler_asm:
    push rax
    mov rax, 0xFEE000B0
    mov dword [rax], 0
    pop rax
    iretq
