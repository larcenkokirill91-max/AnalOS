#pragma once
#include <stdint.h>

typedef struct {
    void* FrameBufferBase;
    uint64_t FrameBufferSize;
    uint32_t HorizontalResolution;
    uint32_t VerticalResolution;
    uint32_t PixelsPerScanLine;
} BootInfo;

#include "idt.h"
#include "keyboard.h"
#include "mouse.h"
