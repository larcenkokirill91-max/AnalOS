[org 0x7c00]
[bits 16]

KERNEL_OFFSET equ 0x10000
VBE_INFO_ADDR equ 0x9000      ; Новое безопасное место для структуры VBE (не конфликтует со стеком)

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    mov [boot_drive], dl

    ; 1. Настройка графики VBE 1280x1024, 32 bit
    mov ax, 0x4F01
    mov cx, 0x411B            ; 0x4000 (LFB) + 0x11B (режим)
    mov di, VBE_INFO_ADDR     ; Заполняем структуру по адресу 0x9000
    int 0x10
    cmp ax, 0x004F
    jne video_error

    mov ax, 0x4F02
    mov bx, 0x411B      
    int 0x10
    cmp ax, 0x004F
    jne video_error

    ; 2. Включаем линию А20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Сбрасываем дисковую подсистему перед чтением
    xor ax, ax
    mov dl, [boot_drive]
    int 0x13

    ; === ЧТЕНИЕ ЯДРА ===
    mov cx, 10          
    mov bx, 0x1000      
    
    xor bp, bp          
    mov si, 1           

load_loop:
    push ecx            

    ; Строим структуру DAP в стеке
    push word 0         
    push word 0         
    push bp             
    push si             
    push bx             
    push word 0x0000    
    push word 100       
    push word 0x0010    

    mov ah, 0x42
    mov dl, [boot_drive] 
    mov di, sp          
    push si             
    mov si, di          
    int 0x13
    pop si              
    jc disk_error       

    add sp, 16          
    
    add si, 100         
    adc bp, 0           
    add bx, 0x0C80      

    pop ecx             
    loop load_loop      

    ; Переход в 32-битный защищенный режим
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

    ; === ДОСТАЕМ АДРЕС ВИДЕОПАМЯТИ ИЗ СТРУКТУРЫ VBE ===
    ; В структуре VBE Mode Info по смещению 40 (0x28) лежит 32-битный указатель на Linear Framebuffer
    mov ebx, [VBE_INFO_ADDR + 0x28] 

    ; Теперь в EBX находится динамический адрес видеопамяти текущего ПК!
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