/* djgpp_interrupt_fixed.c – timer ISR with proper argument */
#include <dos.h>
#include <go32.h>
#include <dpmi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "timer.h"
#include "gfx.h"

#define APPROX_FRAMES_PER_SECOND 60
#define TICKS_PER_FRAME (TIMER_TICK_FREQ / APPROX_FRAMES_PER_SECOND)

int Init(void)
{
    printf("Init Timer\n");
    TimerInit();

    printf("Init Gfx\n");
    GfxInit();
    return 0;
}

int Destroy(void)
{
    printf("Destroy timer ...\n");
    TimerDestroy();

    printf("Destroy Gfx\n");
    GfxDestroy();
    return 0;
}

static uint16_t box_x = 50;
static uint16_t box_y = 50;

void Update(void)
{
    box_x += 1;
    box_y += 1;

    while (box_x >= 320) box_x -= 320;
    while (box_y >= 200) box_y -= 200;
}

void Draw(void)
{
    // memset(vram, 1, sizeof(vram));
    TimerWaitVSync();
    GfxClear(1);

    // for (uint16_t y = 0, oy = 0; y < 200; ++y, oy += 320)
    // {
    //     for (uint16_t x = 0; x < 320; ++x)
    //     {
    //         vram[oy + x] = (uint8_t)(x ^ y);
    //     }
    // }

    GfxDrawRect(box_x-3, box_y-3, 26, 26, 0);
    GfxDrawRect(box_x, box_y, 20, 20, 6);

    // GfxFlip();
}

int main(void)
{
    Init();

    uint32_t tstart = TimerGetTicks();
    uint32_t tlast = tstart;
    uint32_t taccum = 0;

    for (uint32_t i = 0; !kbhit(); ++i)
    {
        int32_t ticks = TimerGetTicks() - tlast;
        tlast += ticks;
        taccum += ticks;

        for (; taccum > TICKS_PER_FRAME; taccum -= TICKS_PER_FRAME)
        {
            Update();
        }

        Draw();
    }

    printf("Press ENTER to quit ...\n");
    getchar();

    Destroy();
    return 0;
}
