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

int main(void)
{
    Init();

    uint16_t box_x = 50;
    uint16_t box_y = 50;

    for (int i = 0; !kbhit(); ++i)
    {
        // uint32_t ticks = TimerGetTicks();
        // if ((i & 15) == 0) printf("i: %d\t\t\tticks: %ld\n", i, ticks);

        //
        // UPDATE
        //

        box_x += 1;
        box_y += 1;

        if (box_x >= 320) box_x = 0;
        if (box_y >= 200) box_y = 0;

        //
        // DRAW
        //

        memset(vram, 0, sizeof(vram));

        for (uint16_t y = box_y; y < box_y + 20 && y < 200; ++y)
        {
            for (uint16_t x = box_x; x < box_x + 20 && x < 320; ++x)
            {
                vram[x + y*320] = 6;
            }
        }

        // for (uint16_t y = 0, oy = 0; y < 200; ++y, oy += 320)
        // {
        //     for (uint16_t x = 0; x < 320; ++x)
        //     {
        //         vram[oy + x] = (uint8_t)(x + y);
        //     }
        // }
        GfxFlip();

        // TimerWaitVSync();
    }

    printf("Press ENTER to quit ...\n");
    getchar();

    Destroy();
    return 0;
}
