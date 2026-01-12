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

// Thanks, Juan J. Martinez, for the inspiration for this way of switching between backbuffer and actual VGA memory.
static uint8_t *vram_real; // This will get mapped to the real A000h:0000h VGA screen
static uint8_t vram_buffer[320*200]; // This is just a back buffer that gets copied to the real A000h:0000h
static uint8_t *vram_target;

// static uint8_t buffer[320 * 200];

// static uint16_t vramDescriptor;

void GfxSetRenderTarget(uint8_t useRealVRam)
{
    if (useRealVRam)
    {
        vram_target = vram_real;
    }
    else
    {
        vram_target = vram_buffer;
    }
}

static int MapFrameBuffer(void)
{
    if (!__djgpp_nearptr_enable()) return -1;

    vram_target = vram_real = (uint8_t *)(0xa0000 + __djgpp_conventional_base);
    return 0;
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
    uint32_t color32 = (uint32_t)color << 8UL | color;
    color32 |= color32 << 16UL;
    // _farsetsel(_dos_ds);
    // for (uintptr_t a = 0xA0000UL; a < 0xA0000 + 320*200; a += sizeof(color32))
    //     _farnspokel(a, color32);

    uint32_t *at = (uint32_t *)vram_target;
    for (uint32_t *end = (uint32_t*)(vram_target + 320*200); at < end; ++at)
    {
        *at = color32;
    }
}

void GfxDrawRect(uint16_t box_x, uint16_t box_y, uint16_t box_w, uint16_t box_h, uint8_t color)
{
    // http://www.neuraldk.org/document.php?djgppGraphics
#if 0
    _farsetsel(_dos_ds);
    for (uint16_t yo = box_y * 320, yo_end = MinU16(yo + box_h * 320, 200*320); yo < yo_end; yo += 320)
    {
        for (uint16_t x = box_x, x_end = MinU16(x + box_w, 320); x < x_end; ++x)
        {
            _farnspokeb(0xA0000UL + x + yo, color);
            // vram[x + yo] = color;
        }
    }
#else
    for (uint16_t yo = box_y * 320, yo_end = MinU16(yo + box_h * 320, 200*320); yo < yo_end; yo += 320)
    {
        for (uint16_t x = box_x, x_end = MinU16(x + box_w, 320); x < x_end; ++x)
        {
            vram_target[x + yo] = color;
        }
    }
#endif
}

int GfxInit(void)
{
    if (GfxSetVgaMode(0x13)) return -1;
    if (MapFrameBuffer()) return -1;

    return 0;
}

int GfxDestroy(void)
{
    __djgpp_nearptr_disable();

    if (GfxSetVgaMode(0x3)) return -1;


    return 0;
}