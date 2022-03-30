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

#define THREAD_NUM 10

typedef unsigned long int uint64;

typedef int (*pthread_mutex_lock_t)(pthread_mutex_t *mutex);
pthread_mutex_lock_t pthread_mutex_lock_f;

typedef int (*pthread_mutex_unlock_t)(pthread_mutex_t *mutex);
pthread_mutex_unlock_t pthread_mutex_unlock_f;

#define DEADLOCK_TEST_1 1

#define DEADLOCK_GRAPH_CHECK 1

#if DEADLOCK_GRAPH_CHECK
#define MAX 100

enum Type
{
    PROCESS,
    RESOURCE
};

struct source_type
{
    uint64 id;
    enum Type type;

    uint64 lock_id;
    int degress;
};

struct vertex
{
    struct source_type s;
    struct vertex *next;
};

struct task_graph
{
    struct vertex list[MAX];
    int num;

    struct source_type locklist[MAX];
    int lockidx;

    pthread_mutex_t mutex;
};

struct task_graph *tg = NULL;
int path[MAX + 1];
int visited[MAX];
int k = 0;
int deadlock = 0;

#endif

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    pthread_t selfid = pthread_self();

    pthread_mutex_lock_f(mutex);
}

static init_hook()
{
    pthread_mutex_lock_f = dlsym(RTLD_NEXT, "pthread_mutex_lock");

    pthread_mutex_lock_f = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
}

#if DEADLOCK_TEST_1

pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_4 = PTHREAD_MUTEX_INITIALIZER;

void *thread_rountine_1(void *args)
{
    pthread_t selfid = pthread_self(); //

    printf("thread_routine 1 : %ld \n", selfid);

    pthread_mutex_lock(&mutex_1);
    sleep(1);
    pthread_mutex_lock(&mutex_2);

    pthread_mutex_unlock(&mutex_2);
    pthread_mutex_unlock(&mutex_1);

    return (void *)(0);
}

void *thread_rountine_2(void *args)
{
    pthread_t selfid = pthread_self(); //

    printf("thread_routine 2 : %ld \n", selfid);

    pthread_mutex_lock(&mutex_2);
    sleep(1);
    pthread_mutex_lock(&mutex_3);

    pthread_mutex_unlock(&mutex_3);
    pthread_mutex_unlock(&mutex_2);

    return (void *)(0);
}

void *thread_rountine_3(void *args)
{
    pthread_t selfid = pthread_self(); //

    printf("thread_routine 3 : %ld \n", selfid);

    pthread_mutex_lock(&mutex_3);
    sleep(1);
    pthread_mutex_lock(&mutex_4);

    pthread_mutex_unlock(&mutex_4);
    pthread_mutex_unlock(&mutex_3);

    return (void *)(0);
}

void *thread_rountine_4(void *args)
{
    pthread_t selfid = pthread_self(); //

    printf("thread_routine 4 : %ld \n", selfid);

    pthread_mutex_lock(&mutex_4);
    sleep(1);
    pthread_mutex_lock(&mutex_1);

    pthread_mutex_unlock(&mutex_1);
    pthread_mutex_unlock(&mutex_4);

    return (void *)(0);
}
#endif

int main()
{

#if DEADLOCK_TEST_1
    init_hook();
    start_check();

    printf("start_check\n");
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

#if DEADLOCK_GRAPH_CHECK
    tg = (struct task_graph *)malloc(sizeof(struct task_graph));
    tg->num = 0;

    struct source_type v1;
    v1.id = 1;
    v1.type = PROCESS;
    add_vertex(v1);

    struct source_type v2;
    v2.id = 2;
    v2.type = PROCESS;
    add_vertex(v2);

    struct source_type v3;
    v3.id = 3;
    v3.type = PROCESS;
    add_vertex(v3);

    struct source_type v4;
    v4.id = 4;
    v4.type = PROCESS;
    add_vertex(v4);

    struct source_type v5;
    v5.id = 5;
    v5.type = PROCESS;
    add_vertex(v5);

    add_edge(v1, v2);
    add_edge(v2, v3);
    add_edge(v3, v4);
    add_edge(v4, v5);
    add_edge(v3, v1);

    search_for_cycle(search_vertex(v1));
#endif
}
