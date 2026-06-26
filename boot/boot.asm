[org 0x7c00]
[bits 16]

KERNEL_OFFSET equ 0x10000

start:
    cli                 
    cld                 

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00      

    mov [boot_drive], dl

    ; Настройка графики 1280x1024
    mov ax, 0x4F01
    mov cx, 0x411B      
    mov di, 0x9000      
    int 0x10
    cmp ax, 0x004F
    jne video_error

    mov ax, 0x4F02
    mov bx, 0x411B      
    int 0x10
    cmp ax, 0x004F
    jne video_error

    in al, 0x92
    or al, 2
    out 0x92, al

  mov cx, [0x9000 + 0x10]   ; Запоминаем Pitch в байтах, пока мы в 16-битном режиме!
  lgdt [gdt_descriptor]
  mov eax, cr0
  or eax, 0x1
  mov cr0, eax

  jmp CODE_SEG:init_pm

    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm

video_error:
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

    ; Чтение ядра напрямую через порты диска ATA PIO
    mov edi, KERNEL_OFFSET
    mov ebx, 1          ; Стартовый сектор LBA (0 - загрузчик, 1 - начало ядра)
    mov ecx, 1000       ; Сколько секторов читать (согласуется с размером в Makefile)

.read_loop:
    push ecx
    
    ; 1. Выдаем количество секторов (1 сектор за итерацию)
    mov dx, 0x1F2
    mov al, 1
    out dx, al

    ; 2. Передаем LBA адрес (28-bit) в порты
    mov eax, ebx
    mov dx, 0x1F3
    out dx, al          

    mov eax, ebx
    shr eax, 8
    mov dx, 0x1F4
    out dx, al          

    mov eax, ebx
    shr eax, 16
    mov dx, 0x1F5
    out dx, al          

    mov eax, ebx
    shr eax, 24
    or al, 0xE0         ; Режим LBA + Master Drive
    mov dx, 0x1F6
    out dx, al

    ; 3. Команда "Чтение секторов с повтором" (Read Sectors With Retry)
    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

; 4. Ожидаем готовности контроллера диска
.wait_disk:
    in al, dx
    test al, 0x80       ; Статус BSY (Busy) должен быть 0
    jnz .wait_disk
    test al, 0x08       ; Статус DRQ (Data Request) должен быть 1
    jz .wait_disk

    ; 5. Читаем сектор (256 слов по 2 байта = 512 байт)
    mov ecx, 256
    mov dx, 0x1F0
    rep insw            ; Читает из порта DX в память [EDI], И САМА ДВИГАЕТ EDI ВПЕРЁД!

    inc ebx             ; Переходим к следующему LBA сектору
    pop ecx
    loop .read_loop     

    ; Достаем адрес видеопамяти для ядра из структуры VBE (как в основном загрузчике)
    ; Достаем адрес видеопамяти для ядра из структуры VBE
    mov ebx, [0x9000 + 0x28] 
    ; НОВОЕ: Достаем Pitch (BytesPerScanLine) — это 16-битное число по смещению 0x10
    movzx ecx, word [0x9000 + 0x10]
    ; Прыгаем на физический адрес ядра
    jmp CODE_SEG:KERNEL_OFFSET    
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