#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include <time.h>

int current_tick() {
    int t = 0;
    struct timespec ti;
	clock_gettime(CLOCK_MONOTONIC, &ti);
	t = (int)ti.tv_sec * 1000;
	t += ti.tv_nsec / 1000000;
    return t;
}

int main(int argc, char **argv) {
    unsigned int j, isunix = 0;
    redisContext *c;
    redisReply *reply;
    const char *hostname = "127.0.0.1";

    int port = 6379;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds

    c = redisConnectWithTimeout(hostname, port, timeout);

    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    int num = (argc > 1) ? atoi(argv[1]) : 1000;

    int before = current_tick();

    for (int i=0; i<num; i++) {
        reply = redisCommand(c,"INCR counter");
        printf("INCR counter: %lld\n", reply->integer);
        freeReplyObject(reply);
    }

    int used = current_tick()-before;

    printf("after %d exec redis command, used %d ms\n", num, used);

    /* Disconnects and frees the context */
    redisFree(c);

    return 0;
}
