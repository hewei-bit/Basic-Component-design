/*
 * @Author: your name
 * @Date: 2019-12-09 17:31:53
 * @LastEditTime : 2021-12-11 17:23:47
 * @LastEditors  : Please set LastEditors
 * @Description: redis连接池测试
 * @FilePath: test_CachePool.cpp
 */
#include <unistd.h> 
#include <sstream>
#include "CachePool.h"
using namespace std;

#define DB_HOST_IP          "127.0.0.1"             // 数据库服务器ip
#define DB_HOST_PORT        6379
#define DB_INDEX            0                       // redis默认支持16个db
#define DB_PASSWORD         ""                      // 数据库密码，不设置AUTH时该参数为空
#define DB_POOL_NAME        "redis_pool"            // 连接池的名字，便于将多个连接池集中管理
#define DB_POOL_MAX_CON     4                       // 连接池支持的最大连接数量


#define REDIS_TEST_COUNT 1000

static string int2string(uint32_t user_id)
{
    stringstream ss;
    ss << user_id;
    return ss.str();
}

static uint64_t getTickCount()
{
    struct timeval tval;
    uint64_t ret_tick;

    gettimeofday(&tval, NULL);

    ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
    return ret_tick;
}

void testCacheUsePool(const char *host, const uint16_t port, const char *password)
{
    printf("\n%s %s into\n", host, __FUNCTION__);
    const char *pool_name = "test_pool";

    CachePool *p_cache_pool = new CachePool(pool_name, host, port, 0, password, 10);
    if (p_cache_pool->Init())
    {
        printf("Init cache pool failed");
        return;
    }

    CacheConn *p_cache_conn = p_cache_pool->GetCacheConn();
    p_cache_conn->flushdb();
    // user -> uint32_t id; string name; string email; string phone
    // 测试插入速度
    uint64_t start_time = getTickCount();
    for (int i = 0; i < REDIS_TEST_COUNT; i++)
    {
        string key = "user:" + int2string(i);
        string name = "liaoqingfu-" + int2string(i);
        string str = p_cache_conn->set(key, name);
        if(str.empty())
            printf("p_cache_conn->set failed\n");
    }
    uint64_t end_time = getTickCount();
    printf("%s %s redis write time:%lums\n", host, __FUNCTION__, end_time - start_time);
    delete p_cache_pool;
}

void testCacheOneCmdPerConn(const char *host, const uint16_t port, const char *password)
{
    printf("\n%s %s into\n", host, __FUNCTION__);
    uint64_t start_time = getTickCount();
    CacheConn *p_cache_conn = new CacheConn(host, port, 0, password);
    p_cache_conn->flushdb();
    for (int i = 0; i < REDIS_TEST_COUNT; i++)
    {
        string key = "user:" + int2string(i);
        string name = "liaoqingfu-" + int2string(i);
        if(p_cache_conn->Init() != 0)
        {
            printf("p_cache_conn->Init() failed\n");
        }
        string str = p_cache_conn->set(key, name);
        if(str.empty()) {
            printf("p_cache_conn->set failed\n");
            usleep(5000);
        }

        delete p_cache_conn;
        p_cache_conn = new CacheConn(host, port, 0, password);
        // if(i % 20 == 0)
        // printf("testCacheOneCmdPerConn i = %d\n", i);
    }
    delete p_cache_conn;
    uint64_t end_time = getTickCount();
    printf("%s %s redis write time:%lums\n", host, __FUNCTION__, end_time - start_time);
}

int main(void)
{
    // CacheManager* pCacheManager = TestCacheManager::getInstance();
    // if (!pCacheManager) {
    // 	log_fatal("CacheManager init failed");
    // 	return -1;
    // }
    // testCache("129.204.197.215", 6379, "123456");
    // testCacheOneCmdPerConn2("129.204.197.215", 6379, "123456");
    // testCacheUsePool("129.204.197.215", 6379, "123456");
    // 每个连接执行一个redis command
    testCacheOneCmdPerConn(DB_HOST_IP, DB_HOST_PORT, DB_PASSWORD);
    // 使用连接池执行多个个redis command
    testCacheUsePool(DB_HOST_IP, DB_HOST_PORT,  DB_PASSWORD);
    return 0;
}