#ifndef LOCKING_H
#define LOCKING_H

#include <dpmi.h>

//
// CREDIT: https://delorie.com/djgpp/doc/ug/interrupts/hwirqs.html
//

#define END_OF_FUNCTION(x) static void x##_End(void) {}

#define LOCK_VARIABLE(x)  LockData((void *)&x, sizeof(x))
#define LOCK_FUNCTION(x)  LockCode(x, (long)x##_End - (long)x)

static int LockData(void *a, long size)
{
    unsigned long baseaddr;
    __dpmi_meminfo region;

    if (__dpmi_get_segment_base_address(_my_ds(), &baseaddr) == -1)
        return (-1);

    region.handle = 0;
    region.size = size;
    region.address = baseaddr + (unsigned long)a;

    if (__dpmi_lock_linear_region(&region) == -1)
        return (-1);

    return (0);
}

int LockCode(void *a, long size)
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