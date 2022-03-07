/**
 * @File Name: mmpool_hw.c
 * @Brief : 手写内存池加注释
 * @Author : hewei (hewei_1996@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-03-05
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>

// 规定以4096字节为界线
// 超过4096是mp_large_s
// 小于4096是mp_node_s

// 对齐
#define MP_ALIGNMENT 32
// 页大小
#define MP_PAGE_SIZE 4096
// 一次最大能申请的内存
#define MP_MAX_ALLOC_FROM_POOL (MP_PAGE_SIZE - 1)

// 内存对齐
#define mp_align(n, alignment) (((n) + (alignment - 1)) & ~(alignment - 1))
#define mp_align_ptr(p, alignment) (void *)((((size_t)p) + (alignment - 1)) & ~(alignment - 1))

// 大块结构体
struct mp_large_s
{
    struct mp_large_s *next;
    void *alloc;
};

// 小块结构体
struct mp_node_s
{
    unsigned char *last;
    unsigned char *end;

    struct mp_node_s *next;
    size_t failed;
};

// 内存池结构体
struct mp_pool_s
{
    size_t max;

    struct mp_node_s *current;
    struct mp_large_s *large;

    struct mp_node_s head[0];
};

struct mp_pool_s *mp_create_pool(size_t size);
void mp_destory_pool(struct mp_pool_s *pool);
void *mp_alloc(struct mp_pool_s *pool, size_t size);
void *mp_nalloc(struct mp_pool_s *pool, size_t size);
void *mp_calloc(struct mp_pool_s *pool, size_t size);
void mp_free(struct mp_pool_s *pool, void *p);

/*
动态进行内存对齐

int posix_memalign (void **memptr,
                    size_t alignment,
                    size_t size);

调用posix_memalign( )成功时会返回size字节的动态内存，并且这块内存的地址是alignment的倍数。参数alignment必须是2的幂，
还是void指针的大小的倍数。返回的内存块的地址放在了memptr里面，函数返回值是0.

调用失败时，没有内存会被分配，memptr的值没有被定义，返回如下错误码之一：

EINVAL
参数不是2的幂，或者不是void指针的倍数。

ENOMEM
没有足够的内存去满足函数的请求。

要注意的是，对于这个函数，errno不会被设置，只能通过返回值得到。

由posix_memalign( )获得的内存通过free( )释放。
*/

struct mp_pool_s *mp_create_pool(size_t size)
{
    struct mp_pool_s *p;
    // 动态内存对齐
    int ret = posix_memalign((void **)&p, MP_ALIGNMENT, size + sizeof(struct mp_pool_s) + sizeof(struct mp_node_s));
    if (ret)
    {
        return NULL;
    }
    // 默认创建一个小块
    p->max = (size < MP_MAX_ALLOC_FROM_POOL) ? size : MP_MAX_ALLOC_FROM_POOL;
    p->current = p->head;
    p->large = NULL;
    // 创建时指向申请空间最开始的地方
    p->head->last = (unsigned char *)p + sizeof(struct mp_pool_s) + sizeof(struct mp_node_s);
    p->head->end = p->head->last + size;

    p->head->failed = 0;

    return p;
}
// 销毁内存池
void mp_destory_pool(struct mp_pool_s *pool)
{
    struct mp_node_s *h, *n;
    struct mp_large_s *l;
    // 先释放大块
    for (l = pool->large; l; l = l->next)
    {
        if (l->alloc)
        {
            free(l->alloc);
        }
    }
    // 再释放小块
    h = pool->head->next;

    while (h)
    {
        n = h->next;
        free(h);
        h = n;
    }
    // 释放内存池
    free(pool);
}
// 重置不释放内存池
void mp_reset_pool(struct mp_pool_s *pool)
{
    struct mp_node_s *h;
    struct mp_large_s *l;
    // 先释放大块
    for (l = pool->large; l; l = l->next)
    {
        if (l->alloc)
        {
            free(l->alloc);
        }
    }

    pool->large = NULL;
    // 再释放小块
    for (h = pool->head; h; h = h->next)
    {
        h->last = (unsigned char *)h + sizeof(struct mp_node_s);
    }
}
// 申请一个小块
static void *mp_alloc_block(struct mp_pool_s *pool, size_t size)
{
    unsigned char *m;
    struct mp_node_s *h = pool->head;
    struct mp_node_s *p, *new_node, *current;
    // 计算上一个节点的剩余空间
    size_t psize = (size_t)(h->end - (unsigned char *)h);
    // 做好内存对齐
    int ret = posix_memalign((void **)&m, MP_ALIGNMENT, psize);
    if (ret)
        return NULL;

    // 新节点
    new_node = (struct mp_node_s *)m;
    new_node->end = m + psize;
    new_node->next = NULL;
    new_node->failed = 0;

    m += sizeof(struct mp_node_s);
    m = mp_align_ptr(m, MP_ALIGNMENT);
    // 此次申请的块的结尾
    new_node->last = m + size;

    current = pool->current;
    for (p = current; p->next; p = p->next)
    {
        if (p->failed++ > 4)
        {
            current = p->next;
        }
    }

    p->next = new_node;

    pool->current = current ? current : new_node;

    return m;
}

// 申请一个大块
static void *mp_alloc_large(struct mp_pool_s *pool, size_t size)
{
    // 申请大块空间直接使用malloc
    void *p = malloc(size);
    if (p == NULL)
        return NULL;
    // 先找空闲的空间
    size_t n = 0;
    struct mp_large_s *large;
    for (large = pool->large; large; large = large->next)
    {
        // 如果找到空
        if (large->alloc == NULL)
        {
            large->alloc = p;
            return p;
        }
        // 找3个就不找了
        if (n++ > 3)
            break;
    }
    // 给找到的节点申请空间
    large = mp_alloc(pool, sizeof(struct mp_large_s));
    if (large == NULL)
    {
        free(p);
        return NULL;
    }
    // 头插法
    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}

// 申请大块，自定义内存对齐大小
void *mp_memalign(struct mp_pool_s *pool, size_t size, size_t alignment)
{
    void *p;

    int ret = posix_memalign(&p, alignment, size);
    if (ret)
    {
        return NULL;
    }
    // 给节点结构体申请空间
    struct mp_large_s *large = mp_alloc(pool, sizeof(struct mp_large_s));
    if (large == NULL)
    {
        free(p);
        return NULL;
    }
    // 头插法
    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}

// 内存池分配（开始前进行内存对齐）
void *mp_alloc(struct mp_pool_s *pool, size_t size)
{
    unsigned char *m;
    struct mp_node_s *p;

    // 根据申请大小区分分配大块还是小块
    if (size <= pool->max)
    {
        p = pool->current;
        do
        {
            m = mp_align_ptr(p->last, MP_ALIGNMENT);
            // 如果这部分剩余空间足够大，就不找下一个了
            if ((size_t)(p->end - m) >= size)
            {
                p->last = m + size;
                return m;
            }
            // 找下一个节点
            p = p->next;
        } while (p);

        return mp_alloc_block(pool, size);
    }

    return mp_alloc_large(pool, size);
}

// 内存池分配（开始前不进行内存对齐）
void *mp_nalloc(struct mp_pool_s *pool, size_t size)
{
    unsigned char *m;
    struct mp_node_s *p;

    // 根据申请大小区分分配大块还是小块
    if (size <= pool->max)
    {
        p = pool->current;
        do
        {
            m = p->last;
            // 如果这部分剩余空间足够大，就不找下一个了
            if ((size_t)(p->end - m) >= size)
            {
                p->last = m + size;
                return m;
            }
            // 找下一个节点
            p = p->next;
        } while (p);

        return mp_alloc_block(pool, size);
    }

    return mp_alloc_large(pool, size);
}

// 申请的空间清空
void *mp_calloc(struct mp_pool_s *pool, size_t size)
{
    void *p = mp_alloc(pool, size);
    if (p)
    {
        memset(p, 0, size);
    }

    return p;
}

// 释放大页内存
void mp_free(struct mp_pool_s *pool, void *p)
{
    struct mp_large_s *l;
    for (l = pool->large; l; l = l->next)
    {
        if (p == l->alloc)
        {
            free(l->alloc);
            l->alloc = NULL;

            return;
        }
    }
}

int main(int argc, char *argv[])
{
    int size = 1 << 12;

    struct mp_pool_s *p = mp_create_pool(size);

    int i = 0;
    for (i = 0; i < 10; i++)
    {
        void *mp = mp_alloc(p, 512);
    }

    // printf("mp_align(123, 32): %d, mp_align(17, 32): %d\n", mp_align(123, 32), mp_align(17, 32));

    int j = 0;
    for (i = 0; i < 5; i++)
    {
        char *pp = mp_calloc(p, 32);
        for (j = 0; j < 32; j++)
        {
            if (pp[j])
            {
                printf("calloc wrong\n");
            }
            printf("calloc success\n");
        }
    }

    for (i = 0; i < 5; i++)
    {
        void *l = mp_alloc(p, 8192);
        mp_free(p, l);
    }

    mp_reset_pool(p);

    for (i = 0; i < 58; i++)
    {
        mp_alloc(p, 256);
    }

    mp_destory_pool(p);

    return 0;
}