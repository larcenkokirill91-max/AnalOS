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
    mov cx, 0x4118
    mov di, 0x7000      ; Буфер для ModeInfoBlock
    int 0x10
    cmp ax, 0x004F
    jne video_error

    ; 2. Включаем графический режим VESA
    mov ax, 0x4F02
    mov bx, 0x4118      
    int 0x10
    cmp ax, 0x004F
    jne video_error

    ; 3. Включаем линию А20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; 4. Сброс и чтение диска (30 секторов ядра)
    xor ax, ax
    mov dl, [boot_drive]
    int 0x13

    mov ax, 0x1000
    mov es, ax
    xor bx, bx

    mov ah, 0x02
    mov al, 30          
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; ВЫТАСКИВАЕМ ДАННЫЕ ИЗ СТРУКТУРЫ VBE:
    mov edx, [0x7000 + 16]  ; edx = реальный Pitch (шаг строки в байтах). Он находится на 16-м байте.
    and edx, 0xFFFF         ; Сбрасываем верхние 16 бит, так как Pitch — это word (2 байта)
    
    mov ecx, [0x7000 + 40]  ; ecx = физический адрес экрана (PhysBasePtr)

    ; 5. Переход в 32-битный защищенный режим
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm

video_error:
    mov ax, 0xB800
    mov es, ax
    mov word [es:0], 0x0C56
    jmp $

disk_error:
    mov ax, 0xB800
    mov es, ax
    mov word [es:0], 0x0944
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

    ; ПЕРЕДАЕМ АРГУМЕНТЫ В СИ (в x86 они заталкиваются справа налево)
    push edx    ; Второй аргумент: шаг строки (pitch)
    push ecx    ; Первый аргумент: адрес экрана (fb_address)
    
    call KERNEL_OFFSET
    jmp $

align 4
gdt_start:
gdt_null:
    dd 0x0, 0x0
gdt_code:
    dw 0xFFFF, 0x0000
    db 0x00, 0x9A, 0xCF, 0x00
gdt_data:
    dw 0xFFFF, 0x0000
    db 0x00, 0x92, 0xCF, 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
boot_drive db 0

times 510-($-$$) db 0
dw 0xaa55
