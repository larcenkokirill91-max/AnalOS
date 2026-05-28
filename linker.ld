ENTRY(kernel_main)
OUTPUT_FORMAT("binary")
SECTIONS
{
    . = 0x10000;
    
    .text : ALIGN(4K) { 
        *(.text.entry) /* ЖЁСТКО КЛАДЕМ ЧИСТЫЙ СТАРТ KERNEL_MAIN НА АДРЕС 0x10000 */
        *(.text) 
    }
    .rodata : ALIGN(4K) { *(.rodata) }
    .data   : ALIGN(4K) { *(.data) }
    .bss    : ALIGN(4K) { *(COMMON) *(.bss) }
}
