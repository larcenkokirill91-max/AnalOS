section .multiboot
    align 4
    dd 0x1BADB002              ; MAGIC: магическое число
    dd 0x00                    ; FLAGS: флаги
    dd -(0x1BADB002 + 0x00)    ; CHECKSUM: контрольная сумма
[bits 32]
section .text
    extern k_main  ; Вот эта строка обязательна!
    global _start

_start:
    call k_main
    hlt            ; Команда остановки процессора (чтобы не грелся)
