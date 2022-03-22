
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <errno.h>
#include <sys/epoll.h>

#define BUFFER_LENGTH		1024
#define LISTEN_PORT			100

struct sockitem {
	int sockfd;
	int (*callback)(int fd, int events, void *arg);
	char recvbuffer[BUFFER_LENGTH]; // tcp 粘包拆包    界定数据包
	char sendbuffer[BUFFER_LENGTH]; // 
	int rlength;
	int slength;
};

struct reactor {
	int epfd;
	struct epoll_event events[512];
};

struct reactor *eventloop = NULL;

int recv_cb(int fd, int events, void *arg);

int send_cb(int fd, int events, void *arg) {
	struct sockitem *si = (struct sockitem*)arg;
	write(fd, si->sendbuffer, si->slength);
	// send(fd, si->sendbuffer, si->slength, 0);

	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	//ev.data.fd = clientfd;
	si->sockfd = fd;
	si->callback = recv_cb;
	ev.data.ptr = si;
	epoll_ctl(eventloop->epfd, EPOLL_CTL_MOD, fd, &ev);
}

int recv_cb(int fd, int events, void *arg) {
	struct sockitem *si = (struct sockitem*)arg;
	struct epoll_event ev;
	int ret = read(fd, si->recvbuffer, BUFFER_LENGTH);
	// int ret = recv(fd, si->recvbuffer, BUFFER_LENGTH, 0);
	if (ret < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) { //
			return -1;
		} else {
			
		}
		ev.events = EPOLLIN;
		//ev.data.fd = fd;
		epoll_ctl(eventloop->epfd, EPOLL_CTL_DEL, fd, &ev);
		close(fd);
		free(si);
	} else if (ret == 0) { //
		// 
		printf("disconnect %d\n", fd);
		ev.events = EPOLLIN;
		//ev.data.fd = fd;
		epoll_ctl(eventloop->epfd, EPOLL_CTL_DEL, fd, &ev);
		close(fd);
		free(si);
		
	} else {
		printf("Recv %d bytes: %s", ret, si->recvbuffer);

		si->rlength = ret;
		memcpy(si->sendbuffer, si->recvbuffer, si->rlength);
		si->slength = si->rlength;

		struct epoll_event ev;
		ev.events = EPOLLOUT | EPOLLET;
		//ev.data.fd = clientfd;
		si->sockfd = fd;
		si->callback = send_cb;
		ev.data.ptr = si;

		epoll_ctl(eventloop->epfd, EPOLL_CTL_MOD, fd, &ev);

	}
}

int accept_cb(int fd, int events, void *arg) {
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(struct sockaddr_in));
	socklen_t client_len = sizeof(client_addr);
	
	int clientfd = accept(fd, (struct sockaddr*)&client_addr, &client_len);
	if (clientfd <= 0) return -1;

	char str[INET_ADDRSTRLEN] = {0};
	printf("recv from %s at port %d\n", inet_ntop(AF_INET, &client_addr.sin_addr, str, sizeof(str)),
		ntohs(client_addr.sin_port));

	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	//ev.data.fd = clientfd;

	struct sockitem *si = (struct sockitem*)malloc(sizeof(struct sockitem));
	si->sockfd = clientfd;
	si->callback = recv_cb;
	ev.data.ptr = si;
	
	epoll_ctl(eventloop->epfd, EPOLL_CTL_ADD, clientfd, &ev);
	
	return clientfd;
}


int init_sock(short port) {

	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0) {
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
		return -2;
	}

	if (listen(listenfd, 5) < 0) {
		return -3;
	}
	printf ("listen port : %d\n", port);

	return listenfd;
}

int main(int argc, char *argv[]) {
	int port = atoi(8080);

	eventloop = (struct reactor*)malloc(sizeof(struct reactor));

	eventloop->epfd = epoll_create(1);

	int listenfd = init_sock(port);

	struct epoll_event ev;
	ev.events = EPOLLIN;
	
	struct sockitem *si = (struct sockitem*)malloc(sizeof(struct sockitem));
	si->sockfd = listenfd;
	si->callback = accept_cb;
	ev.data.ptr = si;
	
	epoll_ctl(eventloop->epfd, EPOLL_CTL_ADD, listenfd, &ev);

	while (1) {  // read(fd, buf, sz)
		int nready = epoll_wait(eventloop->epfd, eventloop->events, 512, -1);
		if (nready < -1) {
			break;
		}

		for (int i = 0; i < nready; i ++) {
			if (eventloop->events[i].events & EPOLLIN) {
				struct sockitem *si = (struct sockitem*)eventloop->events[i].data.ptr;
				si->callback(si->sockfd, eventloop->events[i].events, si);
			}

			if (eventloop->events[i].events & EPOLLOUT) {
				struct sockitem *si = (struct sockitem*)eventloop->events[i].data.ptr;
				si->callback(si->sockfd, eventloop->events[i].events, si);
			}
			
			// if (eventloop->events[i].events & EPOLLERR) {
			// }
		}
	}
}

