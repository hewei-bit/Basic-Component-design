#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include <sstream>

#include "threadpool.h"
#include "CachePool.h"


using namespace std;

#define TASK_NUMBER 1000

#define DB_HOST_IP          "127.0.0.1"             // 数据库服务器ip
#define DB_HOST_PORT        6379
#define DB_INDEX            0                       // redis默认支持16个db
#define DB_PASSWORD         ""                      // 数据库密码，不设置AUTH时该参数为空
#define DB_POOL_NAME        "redis_pool"            // 连接池的名字，便于将多个连接池集中管理
#define DB_POOL_MAX_CON     4       

struct threadpool *threadpool_init(int thread_num, int queue_max_num)
{
    struct threadpool *pool = NULL;
    do
    {
        pool = (struct threadpool *)malloc(sizeof(struct threadpool));
        if (NULL == pool)
        {
            printf("failed to malloc threadpool!\n");
            break;
        }
        pool->thread_num = thread_num;
        pool->queue_max_num = queue_max_num;
        pool->queue_cur_num = 0;
        pool->head = NULL;
        pool->tail = NULL;
        if (pthread_mutex_init(&(pool->mutex), NULL))
        {
            printf("failed to init mutex!\n");
            break;
        }
        if (pthread_cond_init(&(pool->queue_empty), NULL))
        {
            printf("failed to init queue_empty!\n");
            break;
        }
        if (pthread_cond_init(&(pool->queue_not_empty), NULL))
        {
            printf("failed to init queue_not_empty!\n");
            break;
        }
        if (pthread_cond_init(&(pool->queue_not_full), NULL))
        {
            printf("failed to init queue_not_full!\n");
            break;
        }
        pool->pthreads = (pthread_t *)malloc(sizeof(pthread_t) * thread_num);
        if (NULL == pool->pthreads)
        {
            printf("failed to malloc pthreads!\n");
            break;
        }
        pool->queue_close = 0;
        pool->pool_close = 0;
        int i;
        for (i = 0; i < pool->thread_num; ++i)
        {
            pthread_create(&(pool->pthreads[i]), NULL, threadpool_function, (void *)pool);
        }

        return pool;
    } while (0);

    return NULL;
}

int threadpool_add_job(struct threadpool *pool, void *(*callback_function)(void *arg), void *arg)
{
    assert(pool != NULL);
    assert(callback_function != NULL);
    assert(arg != NULL);

    pthread_mutex_lock(&(pool->mutex));
    while ((pool->queue_cur_num == pool->queue_max_num) && !(pool->queue_close || pool->pool_close))
    {
        pthread_cond_wait(&(pool->queue_not_full), &(pool->mutex)); //队列满的时候就等待
    }
    if (pool->queue_close || pool->pool_close) //队列关闭或者线程池关闭就退出
    {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }
    struct job *pjob = (struct job *)malloc(sizeof(struct job));
    if (NULL == pjob)
    {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }
    pjob->callback_function = callback_function;
    pjob->arg = arg;
    pjob->next = NULL;
    if (pool->head == NULL)
    {
        pool->head = pool->tail = pjob;
        pthread_cond_broadcast(&(pool->queue_not_empty)); //队列空的时候，有任务来时就通知线程池中的线程：队列非空
    }
    else
    {
        pool->tail->next = pjob;
        pool->tail = pjob;
    }
    pool->queue_cur_num++;
    pthread_mutex_unlock(&(pool->mutex));
    return 0;
}

static uint64_t get_tick_count()
{
    struct timeval tval;
    uint64_t ret_tick;

    gettimeofday(&tval, NULL);

    ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
    return ret_tick;
}

void *threadpool_function(void *arg)
{
    struct threadpool *pool = (struct threadpool *)arg;
    struct job *pjob = NULL;
    uint64_t start_time = get_tick_count();
    uint64_t end_time = get_tick_count();
    while (1) //死循环
    {
        pthread_mutex_lock(&(pool->mutex));
        while ((pool->queue_cur_num == 0) && !pool->pool_close) //队列为空时，就等待队列非空
        {
            end_time = get_tick_count();
            printf("threadpool need time:%lums\n", end_time - start_time);
            pthread_cond_wait(&(pool->queue_not_empty), &(pool->mutex));
        }
        if (pool->pool_close) //线程池关闭，线程就退出
        {
            pthread_mutex_unlock(&(pool->mutex));
            pthread_exit(NULL);
        }
        pool->queue_cur_num--;
        pjob = pool->head;
        if (pool->queue_cur_num == 0)
        {
            pool->head = pool->tail = NULL;
        }
        else
        {
            pool->head = pjob->next;
        }
        if (pool->queue_cur_num == 0)
        {
            pthread_cond_signal(&(pool->queue_empty)); //队列为空，就可以通知threadpool_destroy函数，销毁线程函数
        }
        if (pool->queue_cur_num == pool->queue_max_num - 1)
        {
            pthread_cond_broadcast(&(pool->queue_not_full)); //队列非满，就可以通知threadpool_add_job函数，添加新任务
        }
        pthread_mutex_unlock(&(pool->mutex));

        (*(pjob->callback_function))(pjob->arg); //线程真正要做的工作，回调函数的调用
        free(pjob);
        pjob = NULL;
    }
}
int threadpool_destroy(struct threadpool *pool)
{
    assert(pool != NULL);
    pthread_mutex_lock(&(pool->mutex));
    if (pool->queue_close || pool->pool_close) //线程池已经退出了，就直接返回
    {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }

    pool->queue_close = 1; //置队列关闭标志
    while (pool->queue_cur_num != 0)
    {
        pthread_cond_wait(&(pool->queue_empty), &(pool->mutex)); //等待队列为空
    }

    pool->pool_close = 1; //置线程池关闭标志
    pthread_mutex_unlock(&(pool->mutex));
    pthread_cond_broadcast(&(pool->queue_not_empty)); //唤醒线程池中正在阻塞的线程
    pthread_cond_broadcast(&(pool->queue_not_full));  //唤醒添加任务的threadpool_add_job函数
    int i;
    for (i = 0; i < pool->thread_num; ++i)
    {
        pthread_join(pool->pthreads[i], NULL); //等待线程池的所有线程执行完毕
    }

    pthread_mutex_destroy(&(pool->mutex)); //清理资源
    pthread_cond_destroy(&(pool->queue_empty));
    pthread_cond_destroy(&(pool->queue_not_empty));
    pthread_cond_destroy(&(pool->queue_not_full));
    free(pool->pthreads);
    struct job *p;
    while (pool->head != NULL)
    {
        p = pool->head;
        pool->head = p->next;
        free(p);
    }
    free(pool);
    return 0;
}


// #define random(x) (rand()%x)

static string int2string(uint32_t user_id)
{
    stringstream ss;
    ss << user_id;
    return ss.str();
}

void *workUsePool(void *arg)
{
    CachePool *pCachePool = (CachePool *)arg;
    CacheConn *pCacheConn = pCachePool->GetCacheConn();
    int count = rand() % TASK_NUMBER;
    string key = "user:" + int2string(count);
    string name = "liaoqingfu-" + int2string(count);
    pCacheConn->set(key, name);
    pCachePool->RelCacheConn(pCacheConn);
    return NULL;
}

void *workNoPool(void *arg)
{
    CacheConn *pCacheConn = new CacheConn(DB_HOST_IP, DB_HOST_PORT, DB_INDEX, DB_PASSWORD);
    int count = rand() % TASK_NUMBER;
    string key = "user:" + int2string(count);
    string name = "liaoqingfu-" + int2string(count);
    pCacheConn->set(key, name);
    delete pCacheConn;
    return NULL;
}

int main(int argc, char *argv[])
{
    int thread_num = 1;    // 线程池线程数量
    int db_maxconncnt = 4; // 连接池最大连接数量(核数*2 + 磁盘数量)
    int use_pool = 1;      // 是否使用连接池
    int task_num = TASK_NUMBER;
    if (argc != 4)
    {
        printf("usage:  ./test_ThreadPool thread_num db_maxconncnt use_pool\n \
                example: ./test_ThreadPool 4  4 1 ");

        return 1;
    }
    thread_num = atoi(argv[1]);
    db_maxconncnt = atoi(argv[2]);
    use_pool = atoi(argv[3]);

    const char *pool_name = DB_POOL_NAME;
    const char *host    =  DB_HOST_IP;
    uint16_t port       =  DB_HOST_PORT;
    const char *password = DB_PASSWORD;
    int db_index = DB_INDEX;

    CachePool *pCachePool = new CachePool(pool_name, host, port, 0, password, db_maxconncnt);
    if (pCachePool->Init())
    {
        printf("Init cache pool failed\n");
        return -1;
    }

    CacheConn *pCacheConn = pCachePool->GetCacheConn();
    pCacheConn->flushdb();
    pCachePool->RelCacheConn(pCacheConn);

    printf("thread_num = %d, connection_num:%d, use_pool:%d\n", 
        thread_num, db_maxconncnt, use_pool);
    struct threadpool *pool = threadpool_init(thread_num, TASK_NUMBER);
    for (int i = 0; i < task_num; i++)
    {
        if (use_pool)
        {
            threadpool_add_job(pool, workUsePool, (void *)pCachePool);
        }
        else
        {
            threadpool_add_job(pool, workNoPool, (void *)pCachePool);
        }
    }

    while(pool->queue_cur_num !=0 )     // 判断队列是否还有任务
    {
         sleep(1);  // 还有任务主线程继续休眠
    }
    sleep(2);       // 没有任务要处理了休眠2秒退出，这里等待主要是确保所有线程都已经空闲
    
    threadpool_destroy(pool);
    delete pCachePool;

    return 0;
}