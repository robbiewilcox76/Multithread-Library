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

worker_mutex_t mutex;
int x = 0;

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

void mutex_test(int *a){
	for(int i = 0; i < 10000000; i++){
		worker_mutex_lock(&mutex);
		x++;
		worker_mutex_unlock(&mutex);
	}
	worker_exit(NULL);
}


int main(int argc, char **argv) {
	worker_t thread1, thread2, thread3, thread4, thread5;
	int mutex_retval = worker_mutex_init(&mutex, NULL);
	int num1 = worker_create(&thread1, NULL, (void*)&mutex_test, 6);
	int num2 = worker_create(&thread2, NULL, (void*)&mutex_test, 6); 
	int num3 = worker_create(&thread3, NULL, (void*)&mutex_test, 6); 
	int num4 = worker_create(&thread4, NULL, (void*)&mutex_test, 6);
	int num5 = worker_create(&thread5, NULL, (void*)&mutex_test, 6);
	worker_yield();
	worker_join(thread1, NULL);
	worker_join(thread2, NULL);
	worker_join(thread3, NULL);
	worker_join(thread4, NULL);
	worker_join(thread5, NULL);
	worker_mutex_destroy(&mutex);
	printf("%d\n", x);
	printf("\nother thread\n");
	//while(1){}
	return 0;
}
