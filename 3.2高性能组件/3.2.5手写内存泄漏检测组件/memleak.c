
//#define __USE_GNU

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>

#include <mcheck.h>

#if 0
extern void *__libc_malloc(size_t size);
int enable_malloc_hook = 1;

extern void __libc_free(void* p);
int enable_free_hook = 1;


// func --> malloc() { __builtin_return_address(0)}

// callback --> func --> malloc() { __builtin_return_address(1)}

// main --> callback --> func --> malloc() { __builtin_return_address(2)}


//calloc, realloc
void *malloc(size_t size) {

	if (enable_malloc_hook) {
		enable_malloc_hook = 0;

		void *p = __libc_malloc(size);

		void *caller = __builtin_return_address(1); // 1
		
		char buff[128] = {0};
		sprintf(buff, "./mem/%p.mem", p);

		FILE *fp = fopen(buff, "w");
		fprintf(fp, "[+%p] --> addr:%p, size:%ld\n", caller, p, size);
		fflush(fp);

		//fclose(fp); //free
		
		enable_malloc_hook = 1;
		
		return p;
		
	} else {
		return __libc_malloc(size);
	}
	

	return NULL;
}


void free(void *p) {
	if (enable_free_hook) {

		enable_free_hook = 0;

		char buff[128] = {0};
		sprintf(buff, "./mem/%p.mem", p);

		if (unlink(buff) < 0) { // no exist
			printf("double free: %p\n", p);
		}
		
		__libc_free(p);

		// rm -rf p.mem
		enable_free_hook = 1;

	} else {
		__libc_free(p);
	}
}

#elif 1


void *malloc_hook(size_t size, const char *file, int line) {

	void *p = malloc(size);

	char buff[128] = {0};
	sprintf(buff, "./mem/%p.mem", p);

	FILE *fp = fopen(buff, "w");
	fprintf(fp, "[+%s:%d] --> addr:%p, size:%ld\n", file, line, p, size);
	fflush(fp);	

	fclose(fp);

	return p;
}

void free_hook(void *p, const char *file, int line) {

	char buff[128] = {0};
	sprintf(buff, "./mem/%p.mem", p);

	if (unlink(buff) < 0) { // no exist
		printf("double free: %p\n", p);
		return ;
	}
	
	free(p);

}


#if 0
#define malloc(size)	malloc_hook(size, __FILE__, __LINE__)
#define free(p)			free_hook(p, __FILE__, __LINE__)
#endif


#elif 0

typedef void *(*malloc_hook_t)(size_t size, const void *caller);
malloc_hook_t malloc_f;

typedef void (*free_hook_t)(void *p, const void *caller);
free_hook_t free_f;

int replaced = 0;

void mem_trace(void);
void mem_untrace(void);



void *malloc_hook_f(size_t size, const void *caller) {

	mem_untrace();
	void *ptr = malloc(size);
	//printf("+%p: addr[%p]\n", caller, ptr);

	char buff[128] = {0};
	sprintf(buff, "./mem/%p.mem", ptr);

	FILE *fp = fopen(buff, "w");
	fprintf(fp, "[+%p] --> addr:%p, size:%ld\n", caller, ptr, size);
	fflush(fp);

	fclose(fp); //free
	
	mem_trace();
		
	return ptr;
	
}

void *free_hook_f(void *p, const void *caller) {

	mem_untrace();
	//printf("-%p: addr[%p]\n", caller, p);

	char buff[128] = {0};
	sprintf(buff, "./mem/%p.mem", p);

	if (unlink(buff) < 0) { // no exist
		printf("double free: %p\n", p);
		return ;
	}
	
	free(p);
	mem_trace();
	
}

void mem_trace(void) { //mtrace

	replaced = 1;
	malloc_f = __malloc_hook; //malloc --> 
	free_f = __free_hook;

	__malloc_hook = malloc_hook_f;
	__free_hook = free_hook_f;
}

void mem_untrace(void) {
	
	__malloc_hook = malloc_f;
	__free_hook = free_f;
	replaced = 0;
}

#endif

/*

 @ ./memleak:[0x400645] + 0x257c450 0x14


 */

// addr2line -f -e memleak -a 0x4006f7
int main() {
#if 0
	void *p1 = malloc(10);
	void *p2 = malloc(20); //calloc, realloc

	free(p1);

	void *p3 = malloc(20);
	void *p4 = malloc(20);

	free(p2);
	free(p4);
	free(p4);
	
#elif 1

	mem_trace();

	void *p1 = malloc(10);
	void *p2 = malloc(20); //calloc, realloc

	free(p1);

	void *p3 = malloc(20);
	void *p4 = malloc(20);

	free(p2);
	free(p4);

	mem_untrace();

#else

	mtrace();

	void *p1 = malloc(10);
	void *p2 = malloc(20); //calloc, realloc

	free(p1);

	void *p3 = malloc(20);
	void *p4 = malloc(20);

	free(p2);
	free(p4);

	muntrace();

#endif
	return 0;
}



