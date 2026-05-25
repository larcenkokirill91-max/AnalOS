OUTPUT_FORMAT("binary")
SECTIONS
{
    . = 0x7C00;
    .text : {
        *(.text)
    }
    . = 0x7C00 + 510;
    .sign : {
        SHORT(0xAA55);
    }
}

