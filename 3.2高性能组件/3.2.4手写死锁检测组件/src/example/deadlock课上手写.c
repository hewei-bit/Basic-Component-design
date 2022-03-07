

#define _GNU_SOURCE
#include <dlfcn.h>

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <stdlib.h>
#include <stdint.h>


typedef unsigned long int uint64;

// 
#if 1

#define MAX		100

enum Type {PROCESS, RESOURCE};

struct source_type {

	uint64 id;
	enum Type type;

	uint64 lock_id;
	int degress;
};

struct vertex {

	struct source_type s;
	struct vertex *next;

};

//
struct task_graph {

	struct vertex list[MAX]; // <selfid , thid>
	int num;

	struct source_type locklist[MAX]; // thid = get_threadid_from_mutex(mutex);
	int lockidx;

	pthread_mutex_t mutex;
};

struct task_graph *tg = NULL;
int path[MAX+1];
int visited[MAX]; //
int k = 0;
int deadlock = 0;

struct vertex *create_vertex(struct source_type type) {

	struct vertex *tex = (struct vertex *)malloc(sizeof(struct vertex ));

	tex->s = type;
	tex->next = NULL;

	return tex;

}


int search_vertex(struct source_type type) {

	int i = 0;

	for (i = 0;i < tg->num;i ++) {

		if (tg->list[i].s.type == type.type && tg->list[i].s.id == type.id) {
			return i;
		}

	}

	return -1;
}

void add_vertex(struct source_type type) {

	if (search_vertex(type) == -1) {

		tg->list[tg->num].s = type;
		tg->list[tg->num].next = NULL;
		tg->num ++;

	}

}


int add_edge(struct source_type from, struct source_type to) {

	add_vertex(from);
	add_vertex(to);

	struct vertex *v = &(tg->list[search_vertex(from)]);

	while (v->next != NULL) {
		v = v->next;
	}

	v->next = create_vertex(to);

}


int verify_edge(struct source_type i, struct source_type j) {

	if (tg->num == 0) return 0;

	int idx = search_vertex(i);
	if (idx == -1) {
		return 0;
	}

	struct vertex *v = &(tg->list[idx]);

	while (v != NULL) {

		if (v->s.id == j.id) return 1;

		v = v->next;
		
	}

	return 0;

}


int remove_edge(struct source_type from, struct source_type to) {

	int idxi = search_vertex(from);
	int idxj = search_vertex(to);

	if (idxi != -1 && idxj != -1) {

		struct vertex *v = &tg->list[idxi];
		struct vertex *remove;

		while (v->next != NULL) {

			if (v->next->s.id == to.id) {

				remove = v->next;
				v->next = v->next->next;

				free(remove);
				break;

			}

			v = v->next;
		}

	}

}


void print_deadlock(void) {

	int i = 0;

	printf("deadlock : ");
	for (i = 0;i < k-1;i ++) {

		printf("%ld --> ", tg->list[path[i]].s.id);

	}

	printf("%ld\n", tg->list[path[i]].s.id);

}

int DFS(int idx) {

	struct vertex *ver = &tg->list[idx];
	if (visited[idx] == 1) {

		path[k++] = idx;
		print_deadlock();
		deadlock = 1;
		
		return 0;
	}

	visited[idx] = 1;
	path[k++] = idx;

	while (ver->next != NULL) {

		DFS(search_vertex(ver->next->s));
		k --;
		
		ver = ver->next;

	}

	
	return 1;

}


int search_for_cycle(int idx) {

	

	struct vertex *ver = &tg->list[idx];
	visited[idx] = 1;
	k = 0;
	path[k++] = idx;

	while (ver->next != NULL) {

		int i = 0;
		for (i = 0;i < tg->num;i ++) {
			if (i == idx) continue;
			
			visited[i] = 0;
		}

		for (i = 1;i <= MAX;i ++) {
			path[i] = -1;
		}
		k = 1;

		DFS(search_vertex(ver->next->s));
		ver = ver->next;
	}

}





#endif



typedef int (*pthread_mutex_lock_t)(pthread_mutex_t *mutex);
pthread_mutex_lock_t pthread_mutex_lock_f;


typedef int (*pthread_mutex_unlock_t)(pthread_mutex_t *mutex);
pthread_mutex_unlock_t pthread_mutex_unlock_f;


pthread_mutex_t mtx1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx4 = PTHREAD_MUTEX_INITIALIZER;


int search_lock(uint64 mutex) {
	int idx = 0;

	for (idx = 0;idx < tg->lockidx;idx ++) {
		if (mutex == tg->locklist[idx].lock_id) {
			return idx;
		}
	}

	return -1;
}

int search_empty_lock() {
	int idx = 0;

	for (idx = 0;idx < tg->lockidx;idx ++) {
		if (0 == tg->locklist[idx].lock_id) {
			return idx;
		}
	}

	return tg->lockidx;
}



void beforelock(uint64 threadid, uint64 mutex) {

	int idx = 0; //lockidx  sum
	for (idx = 0;idx < tg->lockidx;idx ++) { // 
		    //self             thid
		if (mutex == tg->locklist[idx].lock_id) {

			//self --> thid
			struct source_type from;
			from.id = threadid;
			add_vertex(from);

			struct source_type to;
			to.id = tg->locklist[idx].id;
			add_vertex(to);

			if (!verify_edge(from, to)) {
				add_edge(from, to);
			}

		}

	}

}

//int inc(*)

void afterlock(uint64 threadid, uint64 mutex) {

	int idx = 0;
	if (-1 == (idx = search_lock(mutex))) { //

		int emp_idx = search_empty_lock(mutex);
		tg->locklist[emp_idx].id = threadid;
		tg->locklist[emp_idx].lock_id = mutex;

		tg->lockidx ++; //atomic

	} else {

		struct source_type from;
		from.id = threadid;
		add_vertex(from);

		struct source_type to;
		to.id = tg->locklist[idx].id;
		add_vertex(to);

		if (!verify_edge(from, to)) {
			remove_edge(from, to);
		}
	
		tg->locklist[idx].id = threadid;
	}
	

}


void afterunlock(uint64 threadid, uint64 mutex) {

	int idx = search_lock(mutex);

	if (tg->locklist[idx].degress == 0) {

		tg->locklist[idx].id = 0;
		tg->locklist[idx].lock_id = 0;
		
	}

}

//
int pthread_mutex_lock(pthread_mutex_t *mutex) {

	printf("pthread_mutex_lock selfid %ld, mutex: %p\n", pthread_self(), mutex);
	beforelock(pthread_self(), mutex);
	
	pthread_mutex_lock_f(mutex);

	afterlock(pthread_self(), mutex);

}

int pthread_mutex_unlock(pthread_mutex_t *mutex) {

	printf("pthread_mutex_unlock\n");

	pthread_mutex_unlock_f(mutex);
	afterunlock(pthread_self(), mutex);
	
}


static int init_hook() {
	//
	//dlopen();
	pthread_mutex_lock_f = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	
	pthread_mutex_unlock_f = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
}





void *thread_routine_a(void *arg) {

	printf("thread_routine a \n");
	pthread_mutex_lock(&mtx1);
	sleep(1);	
	
	pthread_mutex_lock(&mtx2);

	pthread_mutex_unlock(&mtx2);

	pthread_mutex_unlock(&mtx1);

	printf("thread_routine a exit\n");

}

void *thread_routine_b(void *arg) {

	printf("thread_routine b \n");
	pthread_mutex_lock(&mtx2);
	sleep(1);

	pthread_mutex_lock(&mtx3);

	pthread_mutex_unlock(&mtx3);

	pthread_mutex_unlock(&mtx2);

	printf("thread_routine b exit \n");
// -----
	pthread_mutex_lock(&mtx1);
}


void *thread_routine_c(void *arg) {

	printf("thread_routine c \n");
	pthread_mutex_lock(&mtx3);
	sleep(1);

	pthread_mutex_lock(&mtx4);

	pthread_mutex_unlock(&mtx4);

	pthread_mutex_unlock(&mtx3);

	printf("thread_routine c exit \n");
}


void *thread_routine_d(void *arg) {

	printf("thread_routine d \n");
	pthread_mutex_lock(&mtx4);
	sleep(1);

	pthread_mutex_lock(&mtx1);

	pthread_mutex_unlock(&mtx1);

	pthread_mutex_unlock(&mtx4);

	printf("thread_routine d exit \n");
}


int main() {
#if 1
	init_hook();
	
	pthread_t tid1, tid2, tid3, tid4;

	pthread_create(&tid1, NULL, thread_routine_a, NULL);
	pthread_create(&tid2, NULL, thread_routine_b, NULL);
	pthread_create(&tid3, NULL, thread_routine_c, NULL);
	pthread_create(&tid4, NULL, thread_routine_d, NULL);

	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	pthread_join(tid3, NULL);
	pthread_join(tid4, NULL);
#else

	tg = (struct task_graph*)malloc(sizeof(struct task_graph));
	tg->num = 0;

	struct source_type v1;
	v1.id = 1;
	add_vertex(v1);

	struct source_type v2;
	v2.id = 2;
	add_vertex(v2);

	struct source_type v3;
	v3.id = 3;
	add_vertex(v3);

	struct source_type v4;
	v4.id = 4;
	add_vertex(v4);

	add_edge(v1, v2);
	add_edge(v2, v3);
	add_edge(v1, v3);

	add_edge(v3, v4);
	add_edge(v4, v1);

	search_for_cycle(search_vertex(v1));

#endif
}




