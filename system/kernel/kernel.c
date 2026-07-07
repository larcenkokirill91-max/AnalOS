#include "efi.h"

EFIAPI long long efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = init_gop(SystemTable);

    if (!gop) {
        while(1) { __asm__ __volatile__("hlt"); }
    }

    fill_screen(20, 30, 50, 255);
    draw_taskbar(50, 700, 923, 40, 8, 255, 255, 255, 200);
    swap_buffers(gop);

    volatile int keep_running = 1;
    while (keep_running) {
        __asm__ __volatile__("hlt");
    }

    return 0;
}