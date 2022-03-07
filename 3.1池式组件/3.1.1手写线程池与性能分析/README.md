@[TOC](C++后端开发（9）——C/C++内存池原理与实现)
# 1.线程池简介

 - 多线程技术主要解决处理器单元内多个线程执行的问题，它可以显著减少处理器单元的闲置时间，增加处理器单元的吞吐能力。       
 - 假设一个服务器完成一项任务所需时间为：T1 创建线程时间，T2 在线程中执行任务的时间，T3 销毁线程时间。    如果：T1 + T3 远大于 T2，则可以采用线程池，以提高服务器性能。
 - 一个线程池包括以下四个基本组成部分：
	 1. 线程池管理器（ntyWorkerThread）：用于创建并管理线程池，包括 创建线程池，销毁线程池，添加新任务；
	 2. 工作线程（NWORKER）：线程池中线程，在没有任务时处于等待状态，可以循环的执行任务；
	 3. 任务接口（NJOB）：每个任务必须实现的接口，以供工作线程调度任务的执行，它主要规定了任务的入口，任务执行完后的收尾工作，任务的执行状态等；
	 4. 任务队列（NWORKQUEUE）：用于存放没有处理的任务。提供一种缓冲机制。
 - 线程池不仅调整T1,T3产生的时间段，而且它还显著减少了创建线程的数目，看一个例子：
 - 假设一个服务器一天要处理50000个请求，并且每个请求需要一个单独的线程完成。在线程池中，线程数一般是固定的，所以产生线程总数不会超过线程池中线程的数目，而如果服务器不利用线程池来处理这些请求则线程总数为50000。一般线程池大小是远小于50000。所以利用线程池的服务器程序不会为了创建50000而在处理请求时浪费时间，从而提高效率。
 - 代码实现中并没有实现任务接口，而是把Runnable对象加入到线程池管理器（ThreadPool），然后剩下的事情就由线程池管理器（ThreadPool）来完成了![内存池框架](https://img-blog.csdnimg.cn/7bf98a96764b4b19b2907f7baf7c194a.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBA5L2V6JSa,size_20,color_FFFFFF,t_70,g_se,x_16#pic_center)

               
# 2.内存池优点

 - 降低资源消耗。通过重复利用已创建的线程降低线程创建、销毁线程造成的消耗。
 - 提高响应速度。当任务到达时，任务可以不需要等到线程创建就能立即执行。
 - 提高线程的可管理性。线程是稀缺资源，如果无限制的创建，不仅会消耗系统资源，还会降低系统的稳定性，使用线程池可以进行统一的分配、调优和监控
 - 线程池技术正是关注如何缩短或调整T1,T3时间的技术，从而提高服务器程序性能的。它把T1，T3分别安排在服务器程序的启动和结束的时间段或者一些空闲的时间段，这样在服务器程序处理客户请求时，不会有T1，T3的开销了。

# 3.实现
## 3.1 链表操作宏定义
```cpp
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
```
## 3.2 线程池相关结构体
```cpp
// Nworker线程节点
typedef struct NWORKER
{
    pthread_t thread;             //一个线程
    int terminate;                //终止状态
    struct NWORKQUEUE *workqueue; //任务队列
    struct NWORKER *prev;         //前一个线程
    struct NWORKER *next;         //后一个线程

} nWorker;
```

```cpp
// Njob任务节点
typedef struct NJOB
{
    void (*job_function)(struct NJOB *job); //任务指针
    void *user_data;                        //用户数据
    struct NJOB *prev;                      //前一个工作
    struct NJOB *next;                      //后一个工作
} nJob;
```

```c
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
```

## 3.3 主线程：从等待队列中给线程池添加任务
```c
//主线程：从等待队列中给线程池添加任务
static void *ntyWorkerThread(void *ptr)
{
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

```

## 3.4 创建线程池
```c
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

        // 把worker添加进workqueue的workers队列里
        LL_ADD(worker, worker->workqueue->workers);
    }
    return 0;
}

```
## 3.5 关闭线程池
```c
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

```

## 3.6 将任务添加进等待队列

```c
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

```

## 3.7 测试
```c
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
```
