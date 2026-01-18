#include <dpmi.h>
#include <go32.h>
#include <stdint.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#include <sys/nearptr.h>
#include <crt0.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>

#include "math.h"
#include "gfx.h"

// Thanks, Juan J. Martinez, for the inspiration for this way of switching between backbuffer and actual VGA memory.
static uint8_t *vram_real; // This will get mapped to the real A000h:0000h VGA screen
static uint8_t vram_buffer[320*200]; // This is just a back buffer that gets copied to the real A000h:0000h
static uint8_t *vram_target;

static struct { uint8_t r, g, b; } palette[] = {
    #include "gfx_palette.inc"
};

void GfxRenderTarget(uint8_t useRealVRam)
{
    vram_target = (useRealVRam) ? vram_real : vram_buffer;
}

void GfxWaitVSync(void)
{
    while (inportb(0x3da) & 8);
    while (!(inportb(0x3da) & 8));
}

int GfxSetVgaMode(uint8_t mode)
{
    __dpmi_regs regs = { 0 };

    /* detect VGA card */
    regs.x.ax = 0x1a00;
    __dpmi_int(0x10, &regs);
    if (regs.h.al != 0x1a)
        return -1;

    memset(&regs, 0, sizeof(regs));
    regs.h.al = mode;
    __dpmi_int(0x10, &regs);

    if (mode == 0x13)
    {
        /* setup the VGA: 320x200 @ 60Hz */
        /* from: https://gist.github.com/juj/34306e6da02a8a7043e393f01e013f24 */
        disable();

        /* XXX: probably not neeed */
        GfxWaitVSync();

        outportw(0x3d4, 0x0011); /* Turn off write protect to CRTC registers */
        outportw(0x3d4, 0x0b06); /* New vertical total=525 lines, bits 0-7 */
        outportw(0x3d4, 0x3e07); /* New vertical total=525 lines, bits 8-9 */

        outportw(0x3d4, 0xb910); /* Vsync start=scanline 185 */
        outportw(0x3d4, 0x8f12); /* Vertical display end=scanline 399, bits 0-7 */
        outportw(0x3d4, 0xb815); /* Vertical blanking start=scanline 440, bits 0-7 */
        outportw(0x3d4, 0xe216); /* Adjust vblank end position */
        outportw(0x3d4, 0x8511); /* Vsync length=2 lines + turn write protect back on */

        /* XXX: probably not neeed */
        GfxWaitVSync();

        outportw(0x3d4, 0x2813); /* 8 pixel chars */
        outportw(0x3d4, 0x8e11); /* restore retrace */
        enable();
    }

    return 0;
}

void GfxFlip(void)
{
    memcpy(vram_real, vram_buffer, sizeof(vram_buffer));
    // movedata(_my_ds(), (uintptr_t)vram, vramDescriptor, 0, sizeof(vram));
}

void GfxClear(uint8_t color)
{
    memset(vram_target, color, 320*200);
}

void GfxDrawSprite(Sprite *spr, uint16_t x, uint16_t y)
{
    for (uint16_t i = 0; i < spr->h; ++i)
    {
        uint16_t dstOffsetY = (y + i) * 320;
        uint16_t srcOffsetY = i * spr->stride;

        if (dstOffsetY >= 320*200) break;

        for (uint16_t j = 0; j < spr->w; ++j)
        {
            uint16_t dstOffsetX = x + j;
            if (dstOffsetX >= 320) break;

            uint8_t c = spr->pixels[srcOffsetY + j];
            if (c == 0) continue;

            vram_target[dstOffsetY + dstOffsetX] = c;
        }
    }
}

void GfxDrawSpriteFx(Sprite *spr, fix16 fx, fix16 fy)
{
    uint16_t x = FixToInt(fx);
    uint16_t y = FixToInt(fy);
    GfxDrawSprite(spr, x, y);
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
    if (!__djgpp_nearptr_enable()) return -1;

    vram_target = vram_real = (uint8_t *)(0xa0000 + __djgpp_conventional_base);

    // Init palette
    outportb(0x3c8, 0);

    // Transparent
    outportb(0x3c9, 0);
    outportb(0x3c9, 0);
    outportb(0x3c9, 0);

    for (uint8_t i = 0; i < sizeof(palette)/sizeof(palette[0]); ++i)
    {
        outportb(0x3c9, (palette[i].r) >> 2);
        outportb(0x3c9, (palette[i].g) >> 2);
        outportb(0x3c9, (palette[i].b) >> 2);
    }

    return 0;
}

int GfxDestroy(void)
{
    __djgpp_nearptr_disable();

    if (GfxSetVgaMode(0x3)) return -1;

    return 0;
}