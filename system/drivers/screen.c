#include "efi.h"

static EFI_GRAPHICS_OUTPUT_BLT_PIXEL virtual_framebuffer[1024 * 768];

EFIAPI void draw_pixel(UINT32 x, UINT32 y, UINT8 r, UINT8 g, UINT8 b) {
    if (x >= 1024 || y >= 768) return;
    UINT32 index = y * 1024 + x;
    virtual_framebuffer[index].Red = r;
    virtual_framebuffer[index].Green = g;
    virtual_framebuffer[index].Blue = b;
    virtual_framebuffer[index].Reserved = 0;
}

EFIAPI void draw_pixel_alpha(UINT32 x, UINT32 y, UINT8 r, UINT8 g, UINT8 b, UINT8 alpha) {
    if (x >= 1024 || y >= 768) return;

    UINT32 index = y * 1024 + x;
    if (alpha == 0) return;
    if (alpha == 255) {
        virtual_framebuffer[index].Red = r;
        virtual_framebuffer[index].Green = g;
        virtual_framebuffer[index].Blue = b;
        return;
    }

    UINT8 bg_r = virtual_framebuffer[index].Red;
    UINT8 bg_g = virtual_framebuffer[index].Green;
    UINT8 bg_b = virtual_framebuffer[index].Blue;

    virtual_framebuffer[index].Red   = ((r * alpha) + (bg_r * (255 - alpha))) >> 8;
    virtual_framebuffer[index].Green = ((g * alpha) + (bg_g * (255 - alpha))) >> 8;
    virtual_framebuffer[index].Blue  = ((b * alpha) + (bg_b * (255 - alpha))) >> 8;
}

EFIAPI void draw_rect(UINT32 x, UINT32 y, UINT32 w, UINT32 h, UINT8 r, UINT8 g, UINT8 b) {
    if (x >= 1024 || y >= 768 || w == 0 || h == 0) return;
    if (x + w > 1024) w = 1024 - x;
    if (y + h > 768) h = 768 - y;

    for (UINT32 i = 0; i < h; i++) {
        for (UINT32 j = 0; j < w; j++) {
            UINT32 index = (y + i) * 1024 + (x + j);
            virtual_framebuffer[index].Red = r;
            virtual_framebuffer[index].Green = g;
            virtual_framebuffer[index].Blue = b;
            virtual_framebuffer[index].Reserved = 0;
        }
    }
}

EFIAPI void fill_screen(UINT8 r, UINT8 g, UINT8 b) {
    draw_rect(0, 0, 1024, 768, r, g, b);
}

EFIAPI void draw_ui_element(UINT32 w, UINT32 h, int anchor, UINT8 r, UINT8 g, UINT8 b) {
    UINT32 final_x = 0;
    UINT32 final_y = 0;

    if (anchor == 0) {
        final_x = (1024 - w) / 2;
        final_y = (768 - h) / 2;
    }
    else if (anchor == 2) {
        final_x = 0;
        final_y = 768 - h;
        w = 1024;
    }

    draw_rect(final_x, final_y, w, h, r, g, b);
}

EFIAPI void draw_square(UINT32 size, UINT8 r, UINT8 g, UINT8 b) {
    draw_ui_element(size, size, 0, r, g, b);
}

EFIAPI void draw_alpha_test_bar(UINT32 start_x, UINT32 start_y, UINT32 height) {
    for (UINT32 a = 0; a < 256; a++) {
        UINT32 current_x = start_x + a;
        for (UINT32 current_y = start_y; current_y < (start_y + height); current_y++) {
            draw_pixel_alpha(current_x, current_y, 0, 255, 0, (UINT8)a);
        }
    }
}

EFIAPI void swap_buffers(EFI_GRAPHICS_OUTPUT_PROTOCOL *gop) {
    if (!gop || !gop->Blt) return;
    gop->Blt(gop, virtual_framebuffer, EfiBltBufferToVideo, 0, 0, 0, 0, 1024, 768, 0);
}
