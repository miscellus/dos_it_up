#ifndef GFX_H
#define GFX_H

extern uint8_t vram[320*200];

int GfxInit(void);
void GfxFlip(void);
int GfxDestroy(void);

#endif