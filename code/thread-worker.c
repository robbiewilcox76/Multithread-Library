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
#define DEBUG 0

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;

// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE

//variables that I (Robbie) made
int thread_counter = 0; //counts # of threads
tcb *threadQueue; //queue
//int threadQueueSize = 0; 
int has_been_called = 0; //to see if this is first call of worker_create
ucontext_t scheduler; //context for scheduler
ucontext_t benchmark; //context for benchmarks
ucontext_t initial; //context for initial call
struct itimerval sched_timer; //timer
tcb *curThread; //currently running thread

static void signal_handler();
static void enable_timer();
static void disable_timer();

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
	if (getcontext(&control_block->context) < 0){
		perror("getcontext");
		exit(1);
	}
	void *stack=malloc(STACK_SIZE);
	if (stack == NULL){
		perror("Failed to allocate stack");
		return 1;
	}

	//stack and context
	control_block->context.uc_link=NULL;
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

	//is there only 1 argument for every call?
	makecontext(&control_block->context,(void *)function, 1, arg);
	enqueue(control_block);
	if(!has_been_called) {
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
		curThread->thread_status = ready;
		swapcontext(&scheduler, &curThread->context);
	}
	
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread

	// YOUR CODE HERE
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	disable_timer();
	if(DEBUG) printf("inside scheduler\n");
	while(!isEmpty()) {
		if(DEBUG) printf("swapping\n");
		enable_timer();
		dequeue();
		printQueue();
		if(curThread != NULL) swapcontext(&scheduler, &curThread->context);
		//enqueue(curThread);
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
	// Choose PSJF
#else 
	// Choose MLFQ
#endif

}

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
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

void enqueue(tcb *thread) {
	if(threadQueue == NULL) threadQueue = thread;
	else {
		tcb *temp = threadQueue;
		while(temp->next != NULL) temp = temp->next;
		temp->next = thread;
	}
	thread->next = NULL;
}

void dequeue() {
	curThread = threadQueue;
	threadQueue = threadQueue->next;
}

int isEmpty() {
	return threadQueue == NULL;
}

void printQueue() {
	tcb *temp = threadQueue;
	while(temp != NULL) {
		printf("thread %d, ", temp->thread_id);
		temp = temp->next;
	}
}

void toString(tcb *thread) {
	printf("Thread id: %d\nStatus: %d\n\n", thread->thread_id, thread->thread_status);
}

static void signal_handler(int signum) {
	if(DEBUG) puts("signal received\n");
	enqueue(curThread);
	if(curThread != NULL ) swapcontext(&curThread->context, &scheduler);
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
	getcontext(&scheduler);
	void* stack = malloc(SIGSTKSZ);
	scheduler.uc_link=NULL;
	scheduler.uc_stack.ss_sp=stack;
	scheduler.uc_stack.ss_size=STACK_SIZE;
	scheduler.uc_stack.ss_flags=0;
	if(DEBUG) printf("scheduler/benchmark context created\n");

	makecontext(&scheduler, (void *)&schedule, 0, NULL);
	setup_timer();
	setcontext(&scheduler);
	has_been_called = 1;
	return 0;
}

