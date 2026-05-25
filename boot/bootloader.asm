[org 0x7c00]
[bits 16]

KERNEL_OFFSET equ 0x10000

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    mov [boot_drive], dl

    ; 1. Получаем информацию о режиме VBE 0x4115 (800x600, 24-bit цвет)
    mov ax, 0x4F01
    mov cx, 0x4115
    mov di, 0x7000      ; Безопасный буфер для ModeInfoBlock
    int 0x10
    cmp ax, 0x004F
    jne error

    ; 2. Включаем графический режим
    mov ax, 0x4F02
    mov bx, 0x4115      ; Режим + флаг Linear Framebuffer
    int 0x10
    cmp ax, 0x004F
    jne error

    ; 3. Включаем линию А20 через порт 0x92
    in al, 0x92
    or al, 2
    out 0x92, al

    ; 4. Читаем ядро с дискеты (сектора 2-20) в память по адресу 0x10000
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    
    mov ah, 0x02        ; Функция чтения секторов BIOS
    mov al, 19          ; Читаем 19 секторов
    mov ch, 0           ; Цилиндр 0
    mov dh, 0           ; Головка 0
    mov cl, 2           ; Сектор 2
    mov dl, [boot_drive]
    int 0x13
    jc error

    ; Считываем 32-битный физический адрес экрана из структуры VBE (смещение 40)
    mov ecx, [0x7000 + 40]

    ; 5. Переход в 32-битный защищенный режим
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm

error:
    ; Если видеорежим или диск сбоит, выводим красную букву 'E' на экран в текстовом режиме
    mov ax, 0xB800
    mov es, ax
    mov word [es:0], 0x0C45
    jmp $

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp

    ; Передаем физический адрес экрана (LFB) в ядро
    push ecx
    call KERNEL_OFFSET
    jmp $

; --- Глобальная таблица дескрипторов (GDT) ---
gdt_start:
gdt_null:
    dd 0x0, 0x0
gdt_code:
    dw 0xffff, 0x0, 0x9a00, 0xcf
gdt_data:
    dw 0xffff, 0x0, 0x9200, 0xcf
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
boot_drive db 0

times 510-($-$$) db 0
dw 0xaa55

