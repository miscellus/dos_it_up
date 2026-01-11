#ifndef GFX_H
#define GFX_H

extern uint8_t vram[320*200];

void GfxFlip(void);
void GfxClear(uint8_t color);
void GfxDrawRect(uint16_t box_x, uint16_t box_y, uint16_t box_w, uint16_t box_h, uint8_t color);

int GfxInit(void);
int GfxDestroy(void);

#endif