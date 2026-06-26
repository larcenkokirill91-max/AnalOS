[bits 32]
[extern kernel_main]
global _start
_start:
    mov ebp, esp
    and esp, 0xFFFFFFF0
    call kernel_main
    jmp $