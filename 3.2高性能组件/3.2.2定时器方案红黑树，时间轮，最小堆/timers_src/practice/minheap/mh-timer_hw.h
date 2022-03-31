#ifndef MARK_MINHEAP_TIMER_H
#define MARK_MINHEAP_TIMER_H

#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#include <sys/time.h>
#include <mach/task.h>
#include <mach/mach.h>
#else
#include <time.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "minheap.h"

static min_heap_t min_heap;

static uint32_t
current_time()
{
    uint32_t t;
    struct timespec ti;
    clock_gettime(CLOCK_MONOTONIC, &ti);
    t = (uint32_t)ti.tv_sec * 1000;
    t += ti.tv_nsec / 1000000;
    return t;
}

void init_timer()
{
    min_heap_ctor_(&min_heap);
}

timer_entry_t *add_timer(uint32_t msec, timer_handler_pt callback)
{
}

bool del_timer(timer_entry_t *e)
{
}

int find_nearest_expire_timer()
{
}

void expire_timer()
{
}

#endif // MARK_MINHEAP_TIMER_H
