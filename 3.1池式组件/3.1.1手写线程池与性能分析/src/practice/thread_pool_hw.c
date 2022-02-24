#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
//使用宏定义将某个东西添加到队列中
#define LL_ADD(item, list) \
    do                     \
    {                      \
        item->prev = NULL; \
        item->next = list; \
        list = item;       \
    } while (0)

//使用宏定义将某个东西从队列中去除
#define LL_REMOVE(item, list)              \
    do                                     \
    {                                      \
        if (item->prev != NULL)            \
            item->prev->next = item->next; \
        if (item->next != NULL)            \
            item->next->prev = item->prev; \
        if (list == item)                  \
            list = item->next;             \
        item->prev = item->next = NULL;    \
    } while (0)

// Nworker线程节点
typedef struct NWORKER
{
    pthread_t thread;
    int terminate;

} nworker;

// Njob任务节点
typedef struct NJOB
{
} nJob;

// Nworkqueue 线程队列
typedef struct NWORKQUEUE
{
    struct NWORKER *workers;
} nWorkQueue;

//给队列起别名为线程池
typedef nWorkQueue nThreadPool;

//给线程添加一个任务
static void *ntyWorkerThread(void *ptr)
{
}

//创建线程池
int ntyThreadPoolCreate(nThreadPool *workqueue, int numWorkers)
{
}

//关闭线程池
void ntyThreadPoolShutdown(nThreadPool *workqueue)
{
}

//将任务添加进等待队列
void ntyThreadPoolQueue(nThreadPool *workqueue, nJob *job)
{

    // 发送一个信号给另外一个正在处于阻塞等待状态的线程,使其脱离阻塞状态,继续执行
}

/************************** debug thread pool **************************/
// sdk  --> software develop kit
//  提供SDK给其他开发者使用
#if 1

#define KING_MAX_THREAD 80
#define KING_COUNTER_SIZE 1000
void king_counter(nJob *job)
{
}

int main(int argc, char *argv[])
{
}
#endif