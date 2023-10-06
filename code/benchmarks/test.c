#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../thread-worker.h"

/* A scratch program template on which to call and
 * test thread-worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

void fun(int c) {
	printf("testing multithreading : %d\n", c);
}

int main(int argc, char **argv) {
	worker_t thread;
	int num = worker_create(&thread, NULL, (void*)&fun, i); 

	return 0;
}
