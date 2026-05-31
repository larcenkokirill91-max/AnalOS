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
    mov cx, 0x411B
    mov di, 0x7000      ; Буфер для ModeInfoBlock
    int 0x10
    cmp ax, 0x004F
    jne video_error

    ; 2. Включаем графический режим VESA
    mov ax, 0x4F02
    mov bx, 0x411B      
    int 0x10
    cmp ax, 0x004F
    jne video_error

    ; 3. Включаем линию А20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; === 4. НАДЕЖНОЕ РАСШИРЕННОЕ LBA ЧТЕНИЕ ДЛЯ ФЛЕШЕК ===
    mov ax, 0x1000
    mov es, ax
    
    ; Заполняем структуру DAP (Disk Address Packet) в стеке
    push dword 0        ; Старшие 4 байта LBA-адреса (нули)
    push dword 1        ; Начинаем с 1-го сектора (сразу за MBR)
    push word 0x1000    ; Сегмент памяти (0x1000)
    push word 0x0000    ; Смещение памяти (0x0000) -> Итого адрес 0x10000
    push word 30        ; Читаем 30 секторов ядра
    push word 0x0010    ; Размер структуры DAP (16 байт)

    mov ah, 0x42        ; Расширенное чтение LBA
    mov dl, [boot_drive]
    mov si, sp          ; Указатель на DAP в стеке
    int 0x13
    jc disk_error       ; Если сбой контроллера флешки — в красный экран
    
    add sp, 16          ; Чистим стек от DAP
    ; === КОНЕЦ ЧТЕНИЯ ===

    ; ВЫТАСКИВАЕМ ДАННЫЕ ИЗ СТРУКТУРЫ VBE:
    movzx edx, word [0x7000 + 16]  ; Жестко читаем 2 байта Pitch (шаг строки)
    mov ecx, dword [0x7000 + 40]   ; Жестко читаем 4 байта PhysBasePtr (адрес экрана)


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

    ; ЖЕЛЕЗНО ИСПРАВЛЕНО ДЛЯ КОРРЕКТНОГО МАСШТАБА:
    ; Записываем адрес экрана и pitch в фиксированные глобальные переменные в памяти ядра
    mov [0x9000], ecx    ; Физический адрес экрана запишется сразу за точкой входа
    mov [0x9010], edx    ; Шаг строки (Pitch) запишется следом

    push dword [0x9010]  ; Второй аргумент: pitch
    push dword [0x9000]  ; Первый аргумент: fb_address
    call KERNEL_OFFSET    ; Просто прыгаем в Си, стек теперь пуст и чист!
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

