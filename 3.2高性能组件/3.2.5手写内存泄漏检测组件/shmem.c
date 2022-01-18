

// shmget


#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>

// mmap
#if 0
void *shm_mmap_alloc(int size) {

	void *addr = mmap(NULL, size, PROT_READ|PROT_WRITE, 
		MAP_ANON | MAP_SHARED, -1, 0);
	if (addr == MAP_FAILED) {
		return NULL;
	}
	return addr;

}

int shm_mmap_free(void *addr, int size) {

	return munmap(addr, size);

}
#else


void *shm_mmap_alloc(int size) {

	int fd = open("/dev/zero", O_RDWR);

	void *addr = mmap(NULL, size, PROT_READ|PROT_WRITE, 
		 MAP_SHARED, fd, 0);
	close(fd);
	
	if (addr == MAP_FAILED) {
		return NULL;
	}
	
	return addr;

}

int shm_mmap_free(void *addr, int size) {

	return munmap(addr, size);

}

#endif

//

// a.jpg

// fd = open("a.jpg", "r");

// int length = read(fd, buffer, 1024)

int main() {

	char *addr= (char *)shm_mmap_alloc(1024 * 1024);

	pid_t pid = fork();
	if (pid == 0) {

		int i = 0;

		while (i < 26) {
			addr[i] = 'a' + i ++;
			addr[i] = '\0';
			sleep(1);
		}

		
	} else if (pid > 0) {

		int i = 0;
		while (i ++ < 26) {
			
			printf("client : %s\n", addr);
			sleep(1);
				
		}
		
	}

	shm_mmap_free(addr, 1024);

}


