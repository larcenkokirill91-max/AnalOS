ENTRY(_start)
OUTPUT_FORMAT("binary")

SECTIONS
{
    . = 0x10000;

    .text : ALIGN(4) {
        *(.text.entry)
        *(.text)
        *(.text.*)
    }
    
    .rodata : ALIGN(4) {
        *(.rodata)
        *(.rodata.*)
    }

    .data : ALIGN(4) {
        /* ИСПРАВЛЕНО: Сохраняем адреса конструкторов С++ объектов */
        __init_array_start = .;
        *(.init_array)
        *(.init_array.*)
        __init_array_end = .;

        *(.data)
        *(.data.*)
    }

    .bss : ALIGN(4) {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
        *(.note*)
        *(.fini_array*)
    }
}
