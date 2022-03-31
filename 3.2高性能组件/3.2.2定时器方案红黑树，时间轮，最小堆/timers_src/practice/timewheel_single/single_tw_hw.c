/**
 * @File Name: single_tw_hw.c
 * @Brief : 单层级时间轮
 * @Author : hewei (hewei_1996@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-03-31
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <time.h>

#define MAX_TIMER ((1 << 17) - 1)
#define MAX_CONN ((1 << 16) - 1)

typedef struct conn_node
{
    uint8_t used; // 引用    就是这段时间发送的心跳包的个数
    int id;       // fd
} conn_node_t;

typedef struct timer_node
{
    struct timer_node *next; // 下一个
    struct conn_node *node;  // 本身
    uint32_t idx;
} timer_node_t;

static timer_node_t timer_nodes[MAX_TIMER] = {0};
static conn_node_t conn_nodes[MAX_CONN] = {0};
static uint32_t t_iter = 0;
static uint32_t c_iter = 0;

timer_node_t *get_timer_node()
{ // 注意：没有检测定时任务数超过 MAX_TIMER 的情况
}

conn_node_t *get_conn_node()
{ // 注意：没有检测连接数超过 MAX_CONN 的情况
}

#define TW_SIZE 16
#define EXPIRE 10
#define TW_MASK (TW_SIZE - 1)
static uint32_t tick = 0;

typedef struct link_list
{
    timer_node_t head;  // 方便遍历
    timer_node_t *tail; // 方便添加
} link_list_t;

void add_conn(link_list_t *tw, conn_node_t *cnode, int delay)
{
}

void link_clear(link_list_t *list)
{
}

void check_conn(link_list_t *tw)
{
}

static time_t
current_time()
{
}

int main()
{
}
