#include <crt0.h>
#include <dpmi.h>
#include <go32.h>
#include <stdint.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#include <sys/nearptr.h>

uint8_t vram[320*200]; // This is just a back buffer that gets copied to the real A000h:0000h

static uint16_t vramDescriptor;

static void MapFrameBuffer(void)
{
    vramDescriptor = __dpmi_segment_to_descriptor(0xa000);
}

static int GfxSetVgaMode(uint16_t mode)
{
    __dpmi_regs regs;
    regs.h.ah = 0x00;
    regs.h.al = mode;
    __dpmi_int(0x10, &regs);
    return regs.h.ah ? -1 : 0;
}

void GfxFlip(void)
{
    movedata(_my_ds(), (uintptr_t)vram, vramDescriptor, 0, sizeof(vram));
}

int GfxInit(void)
{
    if (GfxSetVgaMode(0x13)) return -1;
    MapFrameBuffer();
    return 0;
}

int GfxDestroy(void)
{
    if (GfxSetVgaMode(0x3)) return -1;
    return 0;
}