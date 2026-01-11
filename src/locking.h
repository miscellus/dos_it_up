#ifndef LOCKING_H
#define LOCKING_H

#include <dpmi.h>

//
// CREDIT: https://delorie.com/djgpp/doc/ug/interrupts/hwirqs.html
//

#define END_OF_FUNCTION(x) static void x##_End(void) {}

#define LOCK_VARIABLE(x)  LockData((uintptr_t)&(x), sizeof(x))
#define LOCK_FUNCTION(x)  LockCode((uintptr_t)(x), (uintptr_t)(x##_End) - (uintptr_t)(x))

static int LockData(uintptr_t a, long size)
{
    unsigned long baseaddr;
    __dpmi_meminfo region;

    if (__dpmi_get_segment_base_address(_my_ds(), &baseaddr) == -1)
        return (-1);

    region.handle = 0;
    region.size = size;
    region.address = baseaddr + a;

    if (__dpmi_lock_linear_region(&region) == -1)
        return (-1);

    return (0);
}

int LockCode(uintptr_t a, long size)
{
    unsigned long baseaddr;
    __dpmi_meminfo region;

    if (__dpmi_get_segment_base_address(_my_cs(), &baseaddr) == -1)
        return (-1);

    region.handle = 0;
    region.size = size;
    region.address = baseaddr + (unsigned long)a;

    if (__dpmi_lock_linear_region(&region) == -1)
        return (-1);

    return (0);
}

#endif