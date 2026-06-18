[bits 32]
[extern kernel_main]
global _start
_start:
    mov ebp, esp
    and esp, 0xFFFFFFF0
    push ebx
    call kernel_main
    jmp $