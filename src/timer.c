#include <dos.h>
#include <dpmi.h>
#include <go32.h>

#include "timer.h"
#include "locking.h"

#define TIMER_INTERRUPT 0x8

static volatile uint32_t timerTicks = 0;

// 1.193182 Mhz base frequency of PIT
// 1193182 hz / (65535>>6) corresponds to 1166.356 hertz
// 65535 is default divisor
// 65535>>6 = 1023 is our divisor
#define TIMER_DIVISOR_SHIFT 6UL
#define TIMER_DIVISOR_MASK ((1UL << TIMER_DIVISOR_SHIFT) - 1UL)
#define TIMER_DIVISOR_DEFAULT (65535UL)
#define TIMER_DIVISOR (TIMER_DIVISOR_DEFAULT >> TIMER_DIVISOR_SHIFT)
#define TIMER_APPROX_TICK_PERIOD 1166

static _go32_dpmi_seginfo oldTimerHandlerSegInfo;

static int TimerHandler(void)
{
    if (++timerTicks & TIMER_DIVISOR_MASK)
    {
        outportb(0x20, inportb(0x20));  /* End-Of-Interrupt command */
        return 0;
    }

    return 1;
}
END_OF_FUNCTION(TimerHandler)

static void TimerSetDivisor(uint16_t divisor)
{
    /*
        https://wiki.osdev.org/Programmable_Interval_Timer

        Set the PIT channel‑0 divider.
        divisor = 0‑65535  (0 is interpreted as 65536 by the PIT)
        The function programs mode 2 (rate‑generator) on channel 0.

        0b00110110
        - bits 7-6 = 00  → select channel 0
        - bits 5-4 = 11  → access mode: lobyte then hibyte
        - bits 3-1 = 011 → operating mode 3 (square wave generator)
        - bit    0 = 0   → binary counting
    */
    disable();
    outportb(0x43, 0x36);
    outportb(0x40, (uint8_t)(divisor & 0xFF));
    outportb(0x40, (uint8_t)(divisor >> 8));
    enable();
}

void TimerInit(void)
{
    LOCK_VARIABLE(timerTicks);
    LOCK_FUNCTION(TimerHandler);

    _go32_dpmi_seginfo info;
    info.pm_selector = _go32_my_cs();
    info.pm_offset = (uint32_t)TimerHandler;
    // _go32_dpmi_allocate_iret_wrapper(&info);

    _go32_dpmi_get_protected_mode_interrupt_vector(TIMER_INTERRUPT, &oldTimerHandlerSegInfo);
    _go32_dpmi_chain_protected_mode_interrupt_vector(TIMER_INTERRUPT, &info);
    // _go32_dpmi_set_protected_mode_interrupt_vector(TIMER_INTERRUPT, &info);

    TimerSetDivisor(TIMER_DIVISOR);
}

void TimerDestroy(void)
{
    TimerSetDivisor(TIMER_DIVISOR_DEFAULT); // regular speed
    _go32_dpmi_set_protected_mode_interrupt_vector(TIMER_INTERRUPT, &oldTimerHandlerSegInfo);
}

void TimerWaitVSync(void)
{
    while (inportb(0x3da) & 8);
    while (!(inportb(0x3da) & 8));
}

uint32_t TimerGetTicks(void)
{
    return timerTicks;
}