#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void TimerInit(void);
void TimerDestroy(void);
uint32_t TimerGetTicks(void);
void TimerWaitVSync(void);

#endif