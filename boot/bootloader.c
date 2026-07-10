#include "efi.h"

void __attribute__((ms_abi)) kernel_main(BootInfo* info);
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
    EFI_BOOT_SERVICES *bs = SystemTable->BootServices;

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = init_gop(SystemTable);
    if (!gop) {
        while(1) { __asm__ __volatile__("hlt"); }
    }

    // 1. Выделяем память под скрытый буфер экрана (1024 * 768 * 4 байта = 3145728 байт = 768 страниц по 4КБ)
    unsigned long long v_buffer_addr = 0;
    // 2 = EfiAllocateAnyPages, 4 = EfiRuntimeServicesData (чтобы ядро гарантированно видело эту память)
    long long status = bs->AllocatePages(2, 4, 768, &v_buffer_addr);
    if (status != 0) {
        while(1) { __asm__ __volatile__("hlt"); }
    }

    // Заполняем структуру данных для передачи в ядро
    BootInfo info;
    info.FrameBufferBase = (void*)gop->Mode->FrameBufferBase;
    info.FrameBufferSize = gop->Mode->FrameBufferSize;
    info.HorizontalResolution = 1024;
    info.VerticalResolution = 768;
    info.PixelsPerScanLine = (gop->Mode->Info) ? gop->Mode->Info->PixelsPerScanLine : 1024;
    info.VirtualFrameBuffer = (void*)v_buffer_addr;

    // 2. Получаем актуальную карту памяти (это жесткое требование UEFI перед выходом)
    UINTN map_size = 0;
    UINTN map_key = 0;
    UINTN desc_size = 0;
    UINT32 desc_ver = 0;

    // Сначала вызываем с нулевым буфером, чтобы узнать реальный размер карты памяти
    bs->GetMemoryMap(&map_size, 0, &map_key, &desc_size, &desc_ver);
    map_size += 2048; // Добавим запас под дескрипторы

    unsigned long long map_buffer = 0;
    // Выделяем временную память под саму карту памяти (округлим map_size до страниц)
    bs->AllocatePages(2, 4, (map_size / 4096) + 1, &map_buffer);

    // Получаем финальную карту памяти и MapKey
    status = bs->GetMemoryMap(&map_size, (void*)map_buffer, &map_key, &desc_size, &desc_ver);
    if (status != 0) {
        while(1) { __asm__ __volatile__("hlt"); }
    }

    // 3. Выходим из UEFI Boot Services. Теперь фоновые прерывания прошивки отключатся навсегда
    status = bs->ExitBootServices(ImageHandle, map_key);
    if (status != 0) {
        // Если выход не удался (например, карта памяти изменилась), пробуем еще раз
        bs->GetMemoryMap(&map_size, (void*)map_buffer, &map_key, &desc_size, &desc_ver);
        bs->ExitBootServices(ImageHandle, map_key);
    }

    // 4. Прыгаем в ядро!
    kernel_entry_t start_kernel = (kernel_entry_t)kernel_main;
    start_kernel(&info);

    while (1) {
        __asm__ __volatile__("hlt");
    }
    return 0;
}
