/**
 * @File Name: deadlock_hw.c
 * @Brief :
 * @Author : hewei (hewei_1996@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-03-30
 *
 */

#define _GNU_SOURCE
#include <dlfcn.h>

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <stdlib.h>
#include <stdint.h>

typedef int (*pthread_mutex_lock_t)(pthread_mutex_t *mutex);
pthread_mutex_lock_t pthread_mutex_lock_f;

typedef int (*pthread_mutex_unlock_t)(pthread_mutex_t *mutex);
pthread_mutex_unlock_t pthread_mutex_unlock_f;

pthread_mutex_t mtx1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx4 = PTHREAD_MUTEX_INITIALIZER;

int pthread_mutex_lock(pthread_mutex_t *mutex)
{

    printf("pthread_mutex_lock selfid %ld, mutex: %p\n", pthread_self(), mutex);
    // beforelock(pthread_self(), mutex);

    pthread_mutex_lock_f(mutex);

    // afterlock(pthread_self(), mutex);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{

    printf("pthread_mutex_unlock\n");

    pthread_mutex_unlock_f(mutex);
    // afterunlock(pthread_self(), mutex);
}

static int init_hook()
{
    //
    // dlopen();
    pthread_mutex_lock_f = dlsym(RTLD_NEXT, "pthread_mutex_lock");

    pthread_mutex_unlock_f = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
}

void *thread_routine_a(void *arg)
{

    printf("thread_routine a \n");
    pthread_mutex_lock(&mtx1);
    sleep(1);

    pthread_mutex_lock(&mtx2);

    pthread_mutex_unlock(&mtx2);

    pthread_mutex_unlock(&mtx1);

    printf("thread_routine a exit\n");
}

void *thread_routine_b(void *arg)
{

    printf("thread_routine b \n");
    pthread_mutex_lock(&mtx2);
    sleep(1);

    pthread_mutex_lock(&mtx3);

    pthread_mutex_unlock(&mtx3);

    pthread_mutex_unlock(&mtx2);

    printf("thread_routine b exit \n");
    // -----
    pthread_mutex_lock(&mtx1);
}

void *thread_routine_c(void *arg)
{

    printf("thread_routine c \n");
    pthread_mutex_lock(&mtx3);
    sleep(1);

    pthread_mutex_lock(&mtx4);

    pthread_mutex_unlock(&mtx4);

    pthread_mutex_unlock(&mtx3);

    printf("thread_routine c exit \n");
}

void *thread_routine_d(void *arg)
{

    printf("thread_routine d \n");
    pthread_mutex_lock(&mtx4);
    sleep(1);

    pthread_mutex_lock(&mtx1);

    pthread_mutex_unlock(&mtx1);

    pthread_mutex_unlock(&mtx4);

    printf("thread_routine d exit \n");
}

int main()
{
#if 1
    init_hook();

    pthread_t tid1, tid2, tid3, tid4;

    pthread_create(&tid1, NULL, thread_routine_a, NULL);
    pthread_create(&tid2, NULL, thread_routine_b, NULL);
    pthread_create(&tid3, NULL, thread_routine_c, NULL);
    pthread_create(&tid4, NULL, thread_routine_d, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    pthread_join(tid3, NULL);
    pthread_join(tid4, NULL);
#endif
}
