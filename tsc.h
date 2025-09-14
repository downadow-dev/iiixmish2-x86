// SPDX-License-Identifier: GPL-2.0
#ifndef TSC_H
#define TSC_H
/**
 * \file
 *
 * Provides access to the CPU timestamp counter.
 *
 *//*
 * Copyright (C) 2020-2022 Martin Whitaker.
 * Copyright (C) 2025 Menshikov Sviatoslav Sergeevich.
 *  - [03/15/25] changes for iiixmish2-x86
 */

/* This file also contains code from memtest86+ `system/timers.c':
     Copyright (C) 2004-2022 Sam Demeulemeester.
*/

#include "types.h"
#include "io.h"

#define rdtsc(low, high)            \
    __asm__ __volatile__("rdtsc"    \
        : "=a" (low),               \
          "=d" (high)               \
    )

#define rdtscl(low)                 \
    __asm__ __volatile__("rdtsc"    \
        : "=a" (low)                \
        : /* no inputs */           \
        : "edx"                     \
    )

static uint32 clks_per_msec = 0;

/**
 * Reads and returns the timestamp counter value.
 */
static inline uint64 get_tsc(void)
{
    uint32    tl;
    uint32    th;

    rdtsc(tl, th);
    return (uint64)th << 32 | (uint64)tl;
}

static void correct_tsc(void)
{
    uint64 start_time, end_time, run_time;
    int loops = 0;

    // Use PIT Timer to find TSC correction factor
    outb((inb(0x61) & ~0x02) | 0x01, 0x61);
    outb(0xb0, 0x43);
    outb(59659 /* 50 mS */ & 0xff, 0x42);
    outb(59659 /* 50 mS */ >> 8, 0x42);

    start_time = get_tsc();

    loops = 0;
    do {
        loops++;
    } while ((inb(0x61) & 0x20) == 0);

    end_time = get_tsc();

    run_time = end_time - start_time;

    // Make sure we have a credible result
    if (loops >= 4 && run_time >= 50000) {
       clks_per_msec = run_time / 50;
    }
}

#endif // TSC_H
