
#ifndef _MARK_ADAPTER_
#define _MARK_ADAPTER_

#include "reactor.h"
#include "hiredis/hiredis.h"
#include "hiredis/async.h"

typedef struct {
    event_t e;
    int mask;
    redisAsyncContext *ctx;
} redis_event_t;

static void redisReadHandler(int fd, int events, void *privdata) {
    ((void)fd);
    ((void)events);
    event_t *e = (event_t*)privdata;
    redis_event_t *re = (redis_event_t *)(char *)e;
    redisAsyncHandleRead(re->ctx);
}

static void redisWriteHandler(int fd, int events, void *privdata) {
    ((void)fd);
    ((void)events);
    event_t *e = (event_t*)privdata;
    redis_event_t *re = (redis_event_t *)(char *)e;
    redisAsyncHandleWrite(re->ctx);
}

static void redisEventUpdate(void *privdata, int flag, int remove) {
    redis_event_t *re = (redis_event_t *)privdata;
    reactor_t *r = re->e.r;
    int prevMask = re->mask;
    int enable = 0;
    if (remove) {
        if ((re->mask & flag) == 0)
            return ;
        re->mask &= ~flag;
        enable = 0;
    } else {
        if (re->mask & flag)
            return ;
        re->mask |= flag;
        enable = 1;
    }
    int fd = re->ctx->c.fd;
    if (re->mask == 0) {
        del_event(r->epfd, fd);
    } else if (prevMask == 0) {
        add_event(r->epfd, fd, re->mask, &re->e);
    } else {
        if (flag & EPOLLIN) {
            enable_event(r->epfd, fd, &re->e, enable, 0);
        } else if (flag & EPOLLOUT) {
            enable_event(r->epfd, fd, &re->e, 0, enable);
        }
    }
}

static void redisAddRead(void *privdata) {
    redis_event_t *re = (redis_event_t *)privdata;
    re->e.read_fn = redisReadHandler;
    redisEventUpdate(privdata, EPOLLIN, 0);
}

static void redisDelRead(void *privdata) {
    redis_event_t *re = (redis_event_t *)privdata;
    re->e.read_fn = 0;
    redisEventUpdate(privdata, EPOLLIN, 1);
}

static void redisAddWrite(void *privdata) {
    redis_event_t *re = (redis_event_t *)privdata;
    re->e.write_fn = redisWriteHandler;
    redisEventUpdate(privdata, EPOLLOUT, 0);
}

static void redisDelWrite(void *privdata) {
    redis_event_t *re = (redis_event_t *)privdata;
    re->e.write_fn = 0;
    redisEventUpdate(privdata, EPOLLOUT, 1);
}

static void redisCleanup(void *privdata) {
    redis_event_t *re = (redis_event_t *)privdata;
    reactor_t *r = re->e.r;
    del_event(r->epfd, re->ctx->c.fd);
    hi_free(re);
}

static int redisAttach(reactor_t *r, redisAsyncContext *ac) {
    redisContext *c = &(ac->c);
    redis_event_t *re;
    
    /* Nothing should be attached when something is already attached */
    if (ac->ev.data != NULL)
        return REDIS_ERR;

    /* Create container for ctx and r/w events */
    re = (redis_event_t*)hi_malloc(sizeof(*re));
    if (re == NULL)
        return REDIS_ERR;

    re->ctx = ac;
    re->e.fd = c->fd;
    re->e.r = r;
    re->mask = 0;

    ac->ev.addRead = redisAddRead;
    ac->ev.delRead = redisDelRead;
    ac->ev.addWrite = redisAddWrite;
    ac->ev.delWrite = redisDelWrite;
    ac->ev.cleanup = redisCleanup;
    ac->ev.data = re;
    return REDIS_OK;
}

#endif
