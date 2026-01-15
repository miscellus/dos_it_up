#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// 1.193182 Mhz base frequency of PIT
// 1193182 hz / (65535>>6) corresponds to 1166.356 hertz
// 65535 is default divisor
// 65535>>6 = 1023 is our divisor
#define TIMER_DIVISOR_SHIFT 8UL
#define TIMER_DIVISOR_MASK ((1UL << TIMER_DIVISOR_SHIFT) - 1UL)
#define TIMER_DIVISOR_DEFAULT (65535UL)
#define TIMER_DIVISOR (TIMER_DIVISOR_DEFAULT >> TIMER_DIVISOR_SHIFT)
#define TIMER_APPROX_PERIOD (TIMER_DIVISOR / 1193182.0)
#define TIMER_TICK_FREQ (1193182 / TIMER_DIVISOR)

void TimerInit(void);
void TimerDestroy(void);
uint32_t TimerGetTicks(void);
double TimerTicksToSeconds(int32_t ticks);

#endif