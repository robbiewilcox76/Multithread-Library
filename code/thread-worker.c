// File:	thread-worker.c

// List all group member's name: Fulton Wilcox III, Sean Patrick
// username of iLab: frw14, smp429
// iLab Server: ilab4
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <ucontext.h>
#include <sys/time.h>
#include "thread-worker.h"

#define STACK_SIZE SIGSTKSZ
#define DEBUG 1

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;

// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE

/* Robbie's variables */
int thread_counter = 0; //counts # of threads
tcb *threadQueue; //thread run queue (MLFQ level 1) (main queue for PSJF scheduling)
//int threadQueueSize = 0; 
ucontext_t scheduler; //context for scheduler
ucontext_t benchmark; //context for benchmarks
ucontext_t context_main; //context for main thread creation call
struct itimerval sched_timer; //timer
tcb *curThread; //currently running thread
int initialcall = 1;

/* Sean's variables */

/* MLFQ levels 2-4 */
tcb *threadQueue_level_2; 
tcb *threadQueue_level_3; 
tcb *threadQueue_level_4; 
/* Blocked thread queue */
tcb *blockedQueue; 
/* Terminated thread queue*/
tcb *terminatedQueue; 

/* Thread 1 is the main thread */

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {

	// - create Thread Control Block (TCB)
	// - create and initialize the context of this worker thread
	// - allocate space of stack for this thread to run
	// after everything is set, push this thread into run queue and 
	// - make it ready for the execution.

	// YOUR CODE HERE
	
	//creates tcb, gets context, makes stack
	tcb* control_block = malloc(sizeof(tcb));
	create_tcb(thread, control_block, function, arg);
	threadQueue = enqueue(control_block, threadQueue);
	if(initialcall) {
		//create context for scheduler and benchmark program
		scheduler_benchmark_create_context();
	}
    return 0;
};

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context

	if(curThread != NULL) {
		if(DEBUG) printf("yielding...");
		curThread->thread_status = ready;
		tot_cntx_switches++;
		swapcontext(&curThread->context, &scheduler);
	}
	
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	if(DEBUG) printf("Thread %d terminating\n", curThread->thread_id);
	disable_timer();
	curThread->thread_status = terminated;
	if(value_ptr) curThread->return_value = value_ptr;
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread

	// YOUR CODE HERE

	tcb *joining_thread = search(thread, threadQueue);
	if(joining_thread == NULL){
		joining_thread = search(thread, blockedQueue);
		if(joining_thread == NULL) { 
			joining_thread = search(thread, terminatedQueue);
			if(joining_thread == NULL) { if(DEBUG) printf("join error: search returned null\n"); exit(1);}
		}
	}
	if(DEBUG)printf("found %d, searched with %d\n", joining_thread->thread_id, thread);

	while(joining_thread->thread_status != terminated) { tot_cntx_switches++; swapcontext(&curThread->context, &scheduler); }

	//terminatedQueue = terminatedQueue->next;
	if(value_ptr) *value_ptr = joining_thread->return_value; //save return value
	if(joining_thread->stack) free(joining_thread->stack); //free thread memory
	free(joining_thread);

	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE

	if(mutex == NULL) return -1;
	if(mutex->initialized == 1) return -1;

	mutex->initialized = 1;
	mutex->locked = 0;
	mutex->lock_owner = NULL;
	if(DEBUG) printf("mutex initialized\n");

	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

    // - use the built-in test-and-set atomic function to test the mutex
    // - if the mutex is acquired successfully, enter the critical section
    // - if acquiring mutex fails, push current thread into block list and
    // context switch to the scheduler thread

    // YOUR CODE HERE
	if(mutex == NULL){ if(DEBUG) printf("mutex is null\n"); return -1; }
	if(mutex->initialized == 0){ if(DEBUG) printf("mutex uninitialized\n"); return -1; }

	//will return initial value of mutex->locked
	while(__atomic_test_and_set(&mutex->locked, 1)){ //if lock was acquired by another thread
		if(DEBUG) printf("blocking thread %d\n", curThread->thread_id);
		curThread->thread_status = blocked;
		tot_cntx_switches++;
		swapcontext(&curThread->context, &scheduler);
	}
	
	//if lock can be acquired, keep track of lock owner
	mutex->lock_owner = curThread;
	//printf("locking mutex\n");

    return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.
	
	// YOUR CODE HERE

	if(mutex == NULL){ if(DEBUG) printf("mutex is null\n"); return -1; }
	if(mutex->initialized == 0 || mutex->locked == 0){ if(DEBUG) printf("mutex unlocked or uninitialized\n"); return -1; }
	if(mutex->lock_owner != curThread){ if(DEBUG) printf("access denied\n"); return -1; }

	//remove threads from blocked queue and add to thread queue
	if(!isEmpty(blockedQueue)){
		if(DEBUG) printf("removing thread from blocked queue\n");
		tcb* temp = blockedQueue;
		blockedQueue = blockedQueue->next;
		temp->thread_status = ready;
		enqueue(temp, threadQueue);
		if(DEBUG) {printf("thread queue: "); printQueue(threadQueue);}
	}

	//release the lock
	mutex->locked = 0;
	mutex->lock_owner = NULL;
	//printf("unlocking mutex\n");

	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init

	//check for valid mutex and lock status
	if(mutex == NULL) {printf("mutex is null\n"); return -1;}
	if(mutex->initialized == 0 || mutex->locked == 1) {printf("mutex locked or uninitialized\n"); return -1;}

	if(DEBUG) printf("destroying mutex\n");
	mutex->initialized = 0;
	mutex->locked = 0;
	mutex->lock_owner = NULL;

	return 0;
};

/* scheduler */
static void schedule() {
	if(DEBUG) printf("inside scheduler\n");
	while(!isEmpty(threadQueue)) {
		disable_timer();
		threadQueue = dequeue(threadQueue);
		if(DEBUG) printf("swapping to thread %d\n", curThread->thread_id);
		curThread->thread_status = running;
		enable_timer();
		if(curThread != NULL) { 
			tot_cntx_switches++; 
			swapcontext(&scheduler, &curThread->context);
			curThread->quantums_elapsed++;
		}
		if(DEBUG) {printf("thread queue: "); printQueue(threadQueue);}
		if(curThread->thread_status != terminated && curThread->thread_status != blocked){ 
			curThread->thread_status = ready;
			threadQueue = enqueue(curThread, threadQueue);
		}
		else if(curThread->thread_status == blocked){
			blockedQueue = enqueue(curThread, blockedQueue);
			if(DEBUG) {printf("blocked queue: "); printQueue(blockedQueue);}
		}
		else if(curThread->thread_status == terminated){
			terminatedQueue = enqueue(curThread, terminatedQueue);
			if(DEBUG) {printf("terminated queue: "); printQueue(terminatedQueue);}
		}
	}
	// - every time a timer interrupt occurs, your worker thread library 
	// should be contexted switched from a thread context to this 
	// schedule() function

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	// if (sched == PSJF)
	//		sched_psjf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

// - schedule policy
#ifndef MLFQ
	sched_psjf();
#else 
	sched_mlfq();
#endif

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE

	//need to find thread with lowest quantums elapsed
}


/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void) {

       fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
       fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
       fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}


// Feel free to add any other functions you need

// YOUR CODE HERE

static void create_tcb(worker_t * thread, tcb* control_block, void *(*function)(void*), void * arg) {
	if (getcontext(&control_block->context) < 0){
		perror("getcontext");
		exit(1);
	}
	void *stack=malloc(STACK_SIZE);
	if (stack == NULL){
		perror("Failed to allocate stack");
		exit(1);
	}

	//stack and context
	control_block->context.uc_link=&scheduler;
	control_block->context.uc_stack.ss_sp=stack;
	control_block->context.uc_stack.ss_size=STACK_SIZE;
	control_block->context.uc_stack.ss_flags=0;
	control_block->stack = stack;

	//other attributes
	control_block->thread_id = thread_counter;
	*thread = control_block->thread_id;
	thread_counter++;
	control_block->thread_status = ready;
	control_block->priority = 0; //don't know what to do with this, we're not there yet
	control_block->quantums_elapsed = 0;
	makecontext(&control_block->context,(void *)function, 1, arg);
}

tcb* enqueue(tcb *thread, tcb *queue) {
	if(queue == NULL) queue = thread;
	else {
		tcb *temp = queue;
		while(temp->next != NULL) temp = temp->next;
		temp->next = thread;
	}
	thread->next = NULL;
	return queue;
}

tcb* dequeue(tcb *queue) {
	curThread = queue;
	queue = queue->next;
	return queue;
}

static tcb* search(worker_t thread, tcb* queue) {
	tcb* temp = queue;
	while(temp != NULL) {
		if(temp->thread_id == thread) return temp;
		temp = temp->next;
	}
	return NULL;
}

int isEmpty(tcb *queue) {
	return queue == NULL;
}

void printQueue(tcb *queue) {
	tcb *temp = queue;
	while(temp != NULL) {
		printf("thread %d, ", temp->thread_id);
		temp = temp->next;
	}
	printf("\n\n");
}

void toString(tcb *thread) {
	printf("Thread id: %d\nStatus: %d\n\n", thread->thread_id, thread->thread_status);
}

static void signal_handler(int signum) {
	if(DEBUG) puts("signal received\n");
	if(curThread != NULL ) {
		tot_cntx_switches++;
		swapcontext(&curThread->context, &scheduler);
	}
}

static void enable_timer() {
	sched_timer.it_interval.tv_usec = TIME_US; 
	sched_timer.it_interval.tv_sec = TIME_S;

	sched_timer.it_value.tv_usec = TIME_US;
	sched_timer.it_value.tv_sec = TIME_S;
	setitimer(ITIMER_PROF, &sched_timer, NULL);
}

static void disable_timer() {
	sched_timer.it_interval.tv_usec = 0; 
	sched_timer.it_interval.tv_sec = 0;

	sched_timer.it_value.tv_usec = 0;
	sched_timer.it_value.tv_sec = 0;
	setitimer(ITIMER_PROF, &sched_timer, NULL);
}

void setup_timer() {
	// Use sigaction to register signal handler
	struct sigaction sa;
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &signal_handler;
	sigaction (SIGPROF, &sa, NULL);

	// Set up what the timer should reset to after the timer goes off

	sched_timer.it_interval.tv_usec = TIME_US; 
	sched_timer.it_interval.tv_sec = TIME_S;

	sched_timer.it_value.tv_usec = TIME_US;
	sched_timer.it_value.tv_sec = TIME_S;

	// Set the timer up (start the timer)
	setitimer(ITIMER_PROF, &sched_timer, NULL);
}

int scheduler_benchmark_create_context() {
	initialcall = 0;
	getcontext(&scheduler);
	void* stack = malloc(SIGSTKSZ);
	scheduler.uc_link=NULL;
	scheduler.uc_stack.ss_sp=stack;
	scheduler.uc_stack.ss_size=STACK_SIZE;
	scheduler.uc_stack.ss_flags=0;
	if(DEBUG) printf("scheduler/benchmark context created\n");

	makecontext(&scheduler, (void *)&schedule, 0, NULL);
	setup_timer();

	getcontext(&context_main);

	tcb *mainTCB = malloc(sizeof(tcb));

	//other attributes
	mainTCB->thread_id = thread_counter;
	thread_counter++;
	mainTCB->thread_status = ready;
	mainTCB->priority = 0; //don't know what to do with this, we're not there yet
	mainTCB->quantums_elapsed = 0;
	threadQueue = enqueue(mainTCB, threadQueue);
	tot_cntx_switches++;
	swapcontext(&mainTCB->context, &scheduler);
	return 0;
}

