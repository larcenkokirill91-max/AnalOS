ENTRY(_start)
OUTPUT_FORMAT("binary")
SECTIONS
{
    . = 0x10000;
    .text : { 
        build/kernel_entry.o(.text)
        *(.text.entry) 
        *(.text) 
    }
    .rodata : { *(.rodata) }
    .data   : { *(.data) }
    .bss    : { *(COMMON) *(.bss) }
}