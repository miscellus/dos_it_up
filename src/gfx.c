#include <dpmi.h>
#include <go32.h>
#include <stdint.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#include <sys/nearptr.h>
#include <crt0.h>
#include <stdlib.h>
#include <stdio.h>

#include "math.h"

// uint8_t vram[320*200]; // This is just a back buffer that gets copied to the real A000h:0000h
// uint8_t *vram; // This is just a back buffer that gets copied to the real A000h:0000h

// static uint16_t vramDescriptor;

static void MapFrameBuffer(void)
{
    // vramDescriptor = __dpmi_segment_to_descriptor(0xa000);
    // vram = (uint8_t *) (__djgpp_conventional_base + 0xa0000UL);
    // vram = (uint8_t *)MK_FP(0xA000, 0);
    // vram = (uint8_t *)0xd0000000;
    // printf("vram %p\n", vram);
}

static int GfxSetVgaMode(uint8_t mode)
{
    __dpmi_regs regs;
    regs.h.ah = 0x00;
    regs.h.al = mode;
    __dpmi_int(0x10, &regs);
    return regs.h.ah ? -1 : 0;
}

void GfxFlip(void)
{
    // movedata(_my_ds(), (uintptr_t)vram, vramDescriptor, 0, sizeof(vram));
}

void GfxClear(uint8_t color)
{
    uint16_t color16 = (uint16_t)color << 8 | color;
    _farsetsel(_dos_ds);
    for (uintptr_t a = 0xA0000UL; a < 0xA0000 + 320*200; a += 2)
        _farnspokew(a, color16);
}

void GfxDrawRect(uint16_t box_x, uint16_t box_y, uint16_t box_w, uint16_t box_h, uint8_t color)
{
    // http://www.neuraldk.org/document.php?djgppGraphics
    _farsetsel(_dos_ds);
    for (uint16_t yo = box_y * 320, yo_end = MinU16(yo + box_h * 320, 200*320); yo < yo_end; yo += 320)
    {
        for (uint16_t x = box_x, x_end = MinU16(x + box_w, 320); x < x_end; ++x)
        {
            _farnspokeb(0xA0000UL + x + yo, color);
            // vram[x + yo] = color;
        }
    }
}

int GfxInit(void)
{
    if (GfxSetVgaMode(0x13)) return -1;
    __djgpp_nearptr_enable();
    MapFrameBuffer();

    return 0;
}

int GfxDestroy(void)
{
    __djgpp_nearptr_disable();

    if (GfxSetVgaMode(0x3)) return -1;


    return 0;
}