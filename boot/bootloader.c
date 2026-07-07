#include "efi.h"

typedef struct {
    unsigned int* framebuffer;
    unsigned int width;
    unsigned int height;
} BootInfo;

typedef void (__attribute__((ms_abi)) *kernel_entry_t)(BootInfo*);
static const EFI_GUID gop_guid = {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};

EFIAPI EFI_GRAPHICS_OUTPUT_PROTOCOL* init_gop(EFI_SYSTEM_TABLE *SystemTable) {
    if (!SystemTable || !SystemTable->BootServices) return 0;
    EFI_BOOT_SERVICES *bs = SystemTable->BootServices;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = 0;

    long long status = bs->LocateProtocol((EFI_GUID*)&gop_guid, 0, (void**)&gop);
    if (status != 0 || !gop || !gop->Mode) return 0;

    UINT32 best_mode = 0xFFFFFFFF;
    for (UINT32 i = 0; i < gop->Mode->MaxMode; i++) {
        UINTN size_of_info = 0;
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = 0;
        status = gop->QueryMode(gop, i, &size_of_info, &info);
        if (status == 0 && info) {
            if (info->HorizontalResolution == 1024 && info->VerticalResolution == 768 &&
                (info->PixelFormat == 0 || info->PixelFormat == 1)) {
                if (gop->SetMode(gop, i) == 0) {
                    if (gop->Mode->FrameBufferBase != 0) {
                        best_mode = i;
                        break;
                    }
                }
                }
        }
    }

    if (best_mode == 0xFFFFFFFF) {
        for (UINT32 i = 0; i < gop->Mode->MaxMode; i++) {
            if (gop->SetMode(gop, i) == 0) {
                if (gop->Mode->FrameBufferBase != 0 && gop->Mode->Info &&
                    (gop->Mode->Info->PixelFormat == 0 || gop->Mode->Info->PixelFormat == 1)) {
                    best_mode = i;
                break;
                    }
            }
        }
    }

    if (best_mode == 0xFFFFFFFF) return 0;
    gop->SetMode(gop, best_mode);
    return gop;
}

EFIAPI long long efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = init_gop(SystemTable);
    if (!gop) {
        while(1) { __asm__ __volatile__("hlt"); }
    }

    BootInfo info;
    info.framebuffer = (unsigned int*)gop->Mode->FrameBufferBase;
    info.width = 1024;
    info.height = 768;

    void kernel_main(BootInfo* info);
    kernel_entry_t start_kernel = (kernel_entry_t)kernel_main;

    start_kernel(&info);

    while (1) {
        __asm__ __volatile__("hlt");
    }
    return 0;
}
