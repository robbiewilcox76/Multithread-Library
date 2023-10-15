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
	for(int i=0; i<100000000; i++) {}
	printf("first down\n");
	for(int i=0; i<100000000; i++) {}
	printf("second down\n");
	for(int i=0; i<100000000; i++) {}
	printf("third down\n");
	for(int i=0; i<100000000; i++) {}
	printf("fourth down\n");
	for(int i=0; i<100000000; i++) {}
	printf("fifth down\n");
	printf("\nI am gay\n");
	worker_exit(NULL);
}


int main(int argc, char **argv) {
	worker_t thread1, thread2, thread3;
	int num1 = worker_create(&thread1, NULL, (void*)&fun, 6);
	//int num2 = worker_create(&thread2, NULL, (void*)&fun, 6); 
	//int num3 = worker_create(&thread2, NULL, (void*)&fun, 6); 
	worker_yield();
	worker_join(thread1, NULL);
	//worker_join(thread3, NULL);
	printf("\nother thread\n");
	//worker_join(thread2, NULL);
	//while(1){}
	return 0;
}
