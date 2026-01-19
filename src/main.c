/* djgpp_interrupt_fxed.c – timer ISR with proper argument */
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
#include <time.h>
#include <stdlib.h>

#include <mikmod.h>

#include "data.h"

#include "timer.h"
#include "gfx.h"
#include "math.h"
#include "audio.h"

#define APPROX_FRAMES_PER_SECOND 60
#define TICKS_PER_FRAME (TIMER_TICK_FREQ / APPROX_FRAMES_PER_SECOND)


const fx32 screenWFx = fx(320);
const fx32 screenHFx = fx(200);

#define BALL_COUNT 1024
typedef struct Ball
{
    fxv2 p;
    fxv2 dp;
} Ball;

static uint8_t color_bg = 1;

static Sprite sprBall;

typedef struct Game
{
    Ball balls[BALL_COUNT];

    // SOUND
    MODULE *bgMusic;
} Game;

static Game *GAME;

int Init(void)
{
    srand(time(0));

    if (AudioInit()) return -1;
    if (TimerInit()) return -1;
    if (GfxInit()) return -1;

    // Game init
    GAME = calloc(1, sizeof(*GAME));
    Ball *balls = GAME->balls;

    GAME->bgMusic = Player_LoadMem((char *)&music, DATA_SIZE(music), 4, 0);
    if (!GAME->bgMusic)
    {
        printf("NOOOOOOO!!!\n");
        return -1;
    }

    Player_Start(GAME->bgMusic);


    for (int i = 0; i < BALL_COUNT; ++i)
    {
        balls[i].p = FxV2(fx(rand()%(320-16+1)), fx(rand()%(200-16+1)));
        balls[i].dp = FxV2(fx(20) + fxr(25, 100), fx(-3));
    }

    return 0;
}

int Shutdown(void)
{
    TimerDestroy();
    GfxDestroy();
    AudioShutdown();

    Player_Stop();
    Player_Free(GAME->bgMusic);
    free(GAME);
    return 0;
}

void Update(void)
{
    AudioUpdate();
    const fx32 dimin = fxr(999, 1000);

    Ball *balls = GAME->balls;

    for (int i = 0; i < BALL_COUNT; ++i)
    {
        balls[i].dp.y += fxr(1, 10);
        balls[i].dp = FxV2Scale(balls[i].dp, dimin);
        balls[i].p = FxV2Add(balls[i].p, balls[i].dp);

        if (balls[i].p.x >= fx(320-16)) { balls[i].p.x = fx(320-16) ; balls[i].dp.x = -balls[i].dp.x ; }
        if (balls[i].p.x < 0)            { balls[i].p.x = fx(0)      ; balls[i].dp.x = -balls[i].dp.x ; }
        if (balls[i].p.y >= fx(200-16)) { balls[i].p.y = fx(200-16) ; balls[i].dp.y = -balls[i].dp.y ; }
        if (balls[i].p.y < 0)            { balls[i].p.y = fx(0)      ; balls[i].dp.y = -balls[i].dp.y ; }
    }
}

void Draw(void)
{
    Ball *balls = GAME->balls;

    GfxRenderTarget(GFX_BUFFER);
    GfxClear(color_bg);

    for (int i = 0; i < BALL_COUNT; ++i)
    {
        GfxDrawSpriteFx(&sprBall, balls[i].p.x, balls[i].p.y);
    }

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
