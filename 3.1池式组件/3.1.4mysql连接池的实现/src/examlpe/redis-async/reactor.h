
#ifndef _MARK_REACTOR_
#define _MARK_REACTOR_

#include <unistd.h>
#include <sys/epoll.h>

#include <stdlib.h>

#include <string.h>

#define MAX_EVENT_NUM 512

typedef struct {
    int epfd; // epoll
    int listenfd;
    int stop;
    struct epoll_event evs[MAX_EVENT_NUM];
} reactor_t;

reactor_t * create_reactor() {
    reactor_t *r = (reactor_t *)malloc(sizeof(*r));
    r->epfd = epoll_create(1);
    r->listenfd = 0;
    r->stop = 0;
    memset(r->evs, 0, sizeof(struct epoll_event) * MAX_EVENT_NUM);
    return r;
}

void release_reactor(reactor_t * r) {
    close(r->epfd);
    free(r);
}

typedef void (*event_callback_fn)(int fd, int events, void *privdata);

typedef struct {
    int fd;
    reactor_t *r;
    event_callback_fn read_fn;
    event_callback_fn write_fn;
    event_callback_fn accept_fn;
} event_t;

// int clientfd = accept(listenfd, addr, sz);
int add_event(int epfd, int fd, int events, void *privdata) {
    struct epoll_event ev;
	ev.events = events; // 读 还是写事件
	ev.data.ptr = privdata;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		return 1;
	}
	return 0;
}

int del_event(int epfd, int fd) {
	epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}

int enable_event(int epfd, int fd, void *privdata, int readable, int writeable) {
	struct epoll_event ev;
	ev.events = (readable ? EPOLLIN : 0) | (writeable ? EPOLLOUT : 0);
	ev.data.ptr = privdata;
	if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
		return 1;
	}
	return 0;
}

// 测试io
// 1. 增删改  事件处理
// 2. 事件循环 取出事件
void eventloop_once(reactor_t * r) {
    int n = epoll_wait(r->epfd, r->evs, MAX_EVENT_NUM, -1);
    for (int i = 0; i < n; i++) {
        int mask = 0;
        struct epoll_event *e = &r->evs[i];
        if (e->events & EPOLLIN) mask |= EPOLLIN;
        if (e->events & EPOLLOUT) mask |= EPOLLOUT;
        if (e->events & EPOLLERR) mask |= EPOLLIN|EPOLLOUT;
        if (e->events & EPOLLHUP) mask |= EPOLLIN|EPOLLOUT;
        if (mask & EPOLLIN) {
            event_t *et = (event_t*) e->data.ptr;
            if (et->fd == r->listenfd) {
                if (et->accept_fn)
                    et->accept_fn(et->fd, mask, et);
            } else {
                if (et->read_fn)
                    et->read_fn(et->fd, mask, et);
            }
        }
        if (mask & EPOLLOUT) {
            event_t *et = (event_t*) e->data.ptr;
            if (et->write_fn)
                et->write_fn(et->fd, mask, et);
        }
    }
}

void stop_eventloop(reactor_t * r) {
    r->stop = 1;
}

void eventloop(reactor_t * r) {
    while (!r->stop) {
        eventloop_once(r);
    }
}

#endif