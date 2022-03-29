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

#define pthread_key_test 1

#define setjmp_test 1
#define setjmp_test_with_trycatch 1

#define CPU_affinity_test 1

pthread_mutex_t mutex;
pthread_spinlock_t spinlock;

// 原子操作
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

// 测试
void *func(void *arg)
{
    int *pcount = (int *)arg;

    int i = 0;

    while (i++ < 100000)
    {
#if with_nothing
        (*pcount)++;
#elif with_mutex_lock
        pthread_mutex_lock(&mutex);
        (*pcount)++;
        pthread_mutex_unlock(&mutex);
#elif with_spin_lock
        pthread_spin_lock(&spinlock);
        (*pcount)++;
        pthread_spin_unlock(&spinlock);
#elif with_atomic
        inc(pcount, 1);
#endif
        usleep(1);
    }
}

// 线程私有数据测试
#if pthread_key_test

pthread_key_t key;

typedef void *(*thread_cb)(void *);

void print_thread1_key(void)
{

    int *p = (int *)pthread_getspecific(key);

    printf("thread 1 : %d\n", *p);
}

void *thread1_proc(void *arg)
{

    int i = 5;
    pthread_setspecific(key, &i);

    print_thread1_key();
}

void print_thread2_key(void)
{

    char *ptr = (char *)pthread_getspecific(key);

    printf("thread 2 : %s\n", ptr);
}

void *thread2_proc(void *arg)
{

    char *ptr = "thread2_proc";
    pthread_setspecific(key, ptr);
    print_thread2_key();
}

struct pair
{
    int x;
    int y;
};

void print_thread3_key(void)
{

    struct pair *p = (struct pair *)pthread_getspecific(key);

    printf("thread 3  x: %d, y: %d\n", p->x, p->y);
}

void *thread3_proc(void *arg)
{

    struct pair p = {1, 2};

    pthread_setspecific(key, &p);
    print_thread3_key();
}
#endif

#if setjmp_test
void sub_func(int idx)
{

    printf("sub_func : %d\n", idx);
#if !setjmp_test_with_trycatch
    longjmp(env, idx);
#endif
    Throw(idx);
}
#endif

#if setjmp_test_with_trycatch
// setjmp / longjmp
struct ExceptionFrame
{

    jmp_buf env;
    int count;

    struct ExceptionFrame *next;
};

// stack --> current node save

jmp_buf env;
int count = 0;

#define Try              \
    count = setjmp(env); \
    if (count == 0)

#define Catch(type) else if (count == type)

#define Throw(type) longjmp(env, type);

#define Finally

#endif

#if CPU_affinity_test

void process_affinity(int num)
{

    // gettid();
    pid_t selfid = syscall(__NR_gettid);

    cpu_set_t mask;
    CPU_ZERO(&mask);

    CPU_SET(1, &mask);

    // selfid
    sched_setaffinity(0, sizeof(mask), &mask);

    while (1)
        ;
}

#endif

int main()
{
    pthread_t thid[TRHEAD_COUNT] = {0};

    int count = 0;

    pthread_mutex_init(&mutex, NULL);
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_SHARED);
    pthread_key_create(&key, NULL);

#if thread_single

    int i = 0;
    for (i = 0; i < TRHEAD_COUNT; i++)
    {
        pthread_create(&thid[i], NULL, func, &count);
    }

    for (i = 0; i < 100; i++)
    {
        printf("count --> %d\n", count);
        sleep(1);
    }
#endif

#if pthread_key_test
    thread_cb callback[THREAD_COUNT] = {
        thread1_proc,
        thread2_proc,
        thread3_proc};

    int i = 0;
    for (i = 0; i < THREAD_COUNT; i++)
    {
        pthread_create(&thid[i], NULL, callback[i], &count);
    }

    for (i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(thid[i], NULL);
    }
#endif

#if setjmp_test
    count = setjmp(env);
    if (count == 0)
    {
        sub_func(++count);
    }
    else if (count == 1)
    {
        sub_func(++count);
    }
    else if (count == 2)
    {
        sub_func(++count);
    }
    else if (count == 3)
    {
        sub_func(++count);
    }
    {
        printf("other item\n");
    }
#endif

#if setjmp_test_with_trycatch
    Try
    {
        sub_func(++count);
    }
    Catch(1)
    {
        sub_func(++count);
    }
    Catch(2)
    {
        sub_func(++count);
    }
    Catch(3)
    {
        sub_func(++count);
    }
    Finally
    {
        printf("other item\n");
    }
#endif

#if CPU_affinity_test
    // 8
    int num = sysconf(_SC_NPROCESSORS_CONF);

    int i = 0;
    pid_t pid = 0;
    for (i = 0; i < num / 2; i++)
    {
        pid = fork();
        if (pid <= (pid_t)0)
        {
            break;
        }
    }

    if (pid == 0)
    {
        process_affinity(num);
    }

    while (1)
        usleep(1);
#endif
}