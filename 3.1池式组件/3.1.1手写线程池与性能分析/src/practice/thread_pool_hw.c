/**
 * @File Name: thread_pool_hw.c
 * @Brief : 线程池实现（C语言）
 * @Author : hewei (hewei_1996@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-03-02
 *
 */

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
    pthread_t thread;             //一个线程
    int terminate;                //终止状态
    struct NWORKQUEUE *workqueue; //任务队列
    struct NWORKER *prev;         //前一个线程
    struct NWORKER *next;         //后一个线程

} nWorker;

// Njob任务节点
typedef struct NJOB
{
    void (*job_function)(struct NJOB *job); //任务指针
    void *user_data;                        //用户数据
    struct NJOB *prev;                      //前一个工作
    struct NJOB *next;                      //后一个工作
} nJob;

// Nworkqueue 工作队列
typedef struct NWORKQUEUE
{
    struct NWORKER *workers;   //工作人员
    struct NJOB *waiting_jobs; //任务
    pthread_mutex_t jobs_mtx;  //互斥锁
    pthread_cond_t jobs_cond;  //条件变量
} nWorkQueue;

//给队列起别名为线程池
typedef nWorkQueue nThreadPool;

//主线程：给线程池添加任务
static void *ntyWorkerThread(void *ptr)
{
    //
    nWorker *worker = (nWorker *)ptr;

    while (1)
    {
        pthread_mutex_lock(&worker->workqueue->jobs_mtx);

        //如果工作队列中等待任务为空
        while (worker->workqueue->waiting_jobs == NULL)
        {
            if (worker->terminate)
                break;
            //阻塞等待添加任务
            pthread_cond_wait(&worker->workqueue->jobs_cond, &worker->workqueue->jobs_mtx);
        }
        //工作队列进入终止状态
        if (worker->terminate)
        {
            pthread_mutex_unlock(&worker->workqueue->jobs_mtx);
            break;
        }
        //把任务从等待队列中取出来
        nJob *job = worker->workqueue->waiting_jobs;
        if (job != NULL)
        {
            LL_REMOVE(job, worker->workqueue->waiting_jobs);
        }

        pthread_mutex_unlock(&worker->workqueue->jobs_mtx);

        //如果取出来的任务为空
        if (job == NULL)
            continue;

        //将任务指针塞进任务的回调函数中
        job->job_function(job);
    }

    // 退出循环
    free(worker);
    pthread_exit(NULL);
}

//创建线程池
int ntyThreadPoolCreate(nThreadPool *workqueue, int numWorkers)
{
    if (numWorkers < 1)
        numWorkers = 1;
    memset(workqueue, 0, sizeof(nThreadPool));
    //初始化要使用的条件变量
    pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
    memcpy(&workqueue->jobs_cond, &blank_cond, sizeof(workqueue->jobs_cond));
    //初始化互斥锁
    pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
    memcpy(&workqueue->jobs_mtx, &blank_mutex, sizeof(workqueue->jobs_mtx));

    int i = 0;
    // 申请numWorkers个worker
    for (i = 0; i < numWorkers; i++)
    {
        // 申请一个worker
        nWorker *worker = (nWorker *)malloc(sizeof(nWorker));
        if (worker == NULL)
        {
            perror("malloc");
            return 1;
        }

        // 把队列塞进worker的结构体里
        memset(worker, 0, sizeof(nWorker));
        worker->workqueue = workqueue;

        // 创建线程
        int ret = pthread_create(&worker->thread, NULL, ntyWorkerThread, (void *)worker);
        if (ret)
        {
            perror("pthread_create");
            free(worker);
            return 1;
        }

        // 把work添加进workqueue的workers队列里
        LL_ADD(worker, worker->workqueue->workers);
    }
    return 0;
}

//关闭线程池
void ntyThreadPoolShutdown(nThreadPool *workqueue)
{
    nWorker *worker = NULL;

    for (worker = workqueue->workers; worker != NULL; worker = worker->next)
    {
        worker->terminate = 1;
    }

    pthread_mutex_lock(&workqueue->jobs_mtx);

    workqueue->workers = NULL;
    workqueue->waiting_jobs = NULL;
    // 广播条件变量
    pthread_cond_broadcast(&workqueue->jobs_cond);

    pthread_mutex_unlock(&workqueue->jobs_mtx);
}

//将任务添加进等待队列
void ntyThreadPoolQueue(nThreadPool *workqueue, nJob *job)
{
    pthread_mutex_lock(&workqueue->jobs_mtx);

    //将任务添加进等待队列
    LL_ADD(job, workqueue->waiting_jobs);
    // 发送一个信号给另外一个正在处于阻塞等待状态的线程,
    // 使其脱离阻塞状态,继续执行
    pthread_cond_signal(&workqueue->jobs_cond);

    pthread_mutex_unlock(&workqueue->jobs_mtx);
}

/************************** debug thread pool **************************/
// sdk  --> software develop kit
//  提供SDK给其他开发者使用
#if 1

#define KING_MAX_THREAD 80
#define KING_COUNTER_SIZE 1000

void king_counter(nJob *job)
{
    int index = *(int *)job->user_data;
    printf("index : %d, selfid : %lu\n", index, pthread_self());

    free(job->user_data);
    free(job);
}

int main(int argc, char *argv[])
{
    nThreadPool pool;
    // 创建线程池
    ntyThreadPoolCreate(&pool, KING_MAX_THREAD);

    int i = 0;
    for (i = 0; i < KING_COUNTER_SIZE; i++)
    {
        nJob *job = (nJob *)malloc(sizeof(nJob));
        if (job == NULL)
        {
            perror("malloc");
            exit(1);
        }

        job->job_function = king_counter;
        // nJob结构体里userdata是一个指针
        job->user_data = malloc(sizeof(int));
        *(int *)job->user_data = i;

        ntyThreadPoolQueue(&pool, job);
    }

    getchar();
    printf("\n");
}
#endif