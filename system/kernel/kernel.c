#include "efi.h"

EFIAPI long long efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = init_gop(SystemTable);

    if (!gop) {
        while(1) { __asm__ __volatile__("hlt"); }
    }

    // Отрисовка без передачи gop в каждый метод
    fill_screen(20, 30, 50);
    draw_ui_element(0, 48, 2, 100, 100, 100);
    draw_ui_element(300, 300, 0, 255, 255, 255);

    for(UINT32 i = 0; i < 200; i++) {
        draw_pixel(i + 400, i + 250, 255, 0, 0);
    }

    // Альфа-полоса шириной 256 пикселей, высотой 60 пикселей
    draw_alpha_test_bar(280, 200, 60);

    // Вывод готового изображения
    swap_buffers(gop);

    volatile int keep_running = 1;
    while (keep_running) {
        __asm__ __volatile__("hlt");
    }

    return 0;
}
