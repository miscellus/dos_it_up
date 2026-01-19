#ifndef GFX_H
#define GFX_H

#include "math.h"

typedef struct
{
	uint8_t *pixels; // 8 bit indicies for mode 0x13
	uint16_t w;
	uint16_t h;
	uint16_t stride;
} Sprite;

enum { GFX_BUFFER = 0, GFX_SCREEN = 1 };

int GfxSetVgaMode(uint8_t mode);
void GfxClear(uint8_t color);
void GfxDrawRect(uint16_t box_x, uint16_t box_y, uint16_t box_w, uint16_t box_h, uint8_t color);
void GfxDrawSprite(Sprite *spr, uint16_t x, uint16_t y);
void GfxDrawSpriteFx(Sprite *spr, fx32 fx, fx32 fy);
void GfxFlip(void);
void GfxRenderTarget(uint8_t useRealVRam);
void GfxWaitVSync(void);

int GfxInit(void);
int GfxDestroy(void);
#endif