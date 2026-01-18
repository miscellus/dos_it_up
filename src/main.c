/* djgpp_interrupt_fixed.c – timer ISR with proper argument */
#include <dos.h>
#include <go32.h>
#include <dpmi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#include <sys/nearptr.h>

#include <mikmod.h>

#include "data.h"

#include "timer.h"
#include "gfx.h"
#include "math.h"
#include "audio.h"

#define APPROX_FRAMES_PER_SECOND 60
#define TICKS_PER_FRAME (TIMER_TICK_FREQ / APPROX_FRAMES_PER_SECOND)

static MODULE *musicModule;

int Init(void)
{
    AudioInit();
    TimerInit();
    GfxInit();

    return 0;
}

int Shutdown(void)
{
    TimerDestroy();
    GfxDestroy();

    Player_Stop();
    Player_Free(musicModule);
    AudioShutdown();
    return 0;
}

static fix16 box_x = FIX(50);
static fix16 box_y = FIX(50);
static uint8_t color_bg = 1;

static Sprite sprBall;

void Update(void)
{
    static fix16 dx = FRAC(8, 1);
    static fix16 dy = FRAC(1, 1);
    box_x += dx;
    box_y += dy;

    dy += FRAC(1, 10);

    const fix16 dimin = FRAC(92, 100);

    if (box_x + FIX(16) >= FIX(320)) box_x = FIX(320-16), dx = -FixMul(dimin, dx), color_bg = 1 + (color_bg + 1) % 63;
    if (box_x < 0) box_x = 0, dx = -FixMul(dimin, dx), color_bg = 1 + (color_bg + 1) % 63;
    if (box_y + FIX(16) >= FIX(200)) box_y = FIX(200-16), dy = -FixMul(dimin, dy), color_bg = 1 + (color_bg + 1) % 63;
    if (box_y < 0) box_y = 0, dy = -FixMul(dimin, dy), color_bg = 1 + (color_bg + 1) % 63;

    MikMod_Update();
}

void Draw(void)
{
    GfxRenderTarget(GFX_BUFFER);
    GfxClear(color_bg);
    GfxDrawSpriteFx(&sprBall, box_x, box_y);
    GfxRenderTarget(GFX_SCREEN);
    GfxWaitVSync();
    GfxFlip();
}

int main(void)
{
    Init();

    memset(&sprBall, 0, sizeof(sprBall));

    sprBall.pixels = sprite_sheet_1;
    sprBall.w = 16;
    sprBall.h = 16;
    sprBall.stride = 256;

    musicModule = Player_LoadMem((char *)&music, DATA_SIZE(music), 4, 0);
    if (!musicModule)
    {
        printf("NOOOOOOO!!!\n");
        return -1;
    }

    Player_Start(musicModule);

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

    Shutdown();
    return 0;
}
