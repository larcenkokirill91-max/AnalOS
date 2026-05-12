ENTRY(_start) /* Теперь точка входа — _start из ассемблера */
SECTIONS
{
    . = 0x100000;
    .text : {
        *(.multiboot) /* Сначала заголовок */
        *(.text)
    }
    .data : { *(.data) }
    .bss  : { *(.bss)  }
}
