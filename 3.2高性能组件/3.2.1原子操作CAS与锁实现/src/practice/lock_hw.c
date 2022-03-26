/**
 * @File Name: lock_hw.c
 * @Brief :
 * @Author : hewei (hewei_1996@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-03-22
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define __USE_GNU
#include <sched.h>

#include <unistd.h>
#include <pthread.h>
#include <setjmp.h>

#include <sys/syscall.h>

#define TRHEAD_COUNT 3

#define thread_single 1
#define with_nothing 1
#define with_mutex_lock 0
#define with_spin_lock 0
#define with_atomic 0

pthread_mutex_t mutex;
pthread_spinlock_t spinlock;

// Ô­×Ó²Ù×÷
int inc(int *value, int add)
{
    int old;
    __asm__ volatile(
        "lock; xaddl %2, %1;"
        : "=a"(old)
        : "m"(*value), "a"(add)
        : "cc", "memory");
    return old;
}

void *func(void *arg)
{
    int *pcount = (int *)arg;

    int i = 0;

    while (i++ < 100000)
    {
#if with_nothing
        (*pcount)++;
#elif with_mutex_lock

#elif with_spin_lock

#elif with_atomic

#endif
        usleep(1);
    }
}

int main()
{
    pthread_t thid[TRHEAD_COUNT] = {0};

    int count = 0;

    pthread_mutex_init(&mutex, NULL);
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_SHARED);

#if thread_single
    int i = 0;
    for (i = 0; i < TRHEAD_COUNT; i++)
    {
        pthread_create(&thid[i], NULL, func, &count);
    }

#endif
}