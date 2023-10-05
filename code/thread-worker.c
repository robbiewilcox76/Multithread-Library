// File:	thread-worker.c

// List all group member's name: Fulton Wilcox III, Sean Patrick
// username of iLab: frw14, smp429
// iLab Server: ilab4

#include "thread-worker.h"
#define STACK_SIZE SIGSTKSZ

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;
int counter = 0;

tcb *threadQueue = NULL;
int threadQueueSize = 0;

void enqueue(tcb *threadQueue, tcb thread);


// INITAILIZE ALL YOUR OTHER VARIABLES HERE
// YOUR CODE HERE


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
	getcontext(&control_block->context);
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
	control_block->thread_id = ++counter;
	control_block->status = ready;
	control_block->priority = 0; //don't know what to do with this, we're not there yet

	//is there only 1 argument for every call?
	makecontext(&control_block->context,(void *)&function, 1, arg);
	enqueue(threadQueue, control_block); //tcb itself serves as node because it has next pointer
	fprintf("%d", threadQueueSize);
	
    return 0;
};

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	// - change worker thread's state from Running to Ready
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context

	// YOUR CODE HERE
	
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

void enqueue(tcb *threadQueue, tcb thread) {
	if(threadQueue = NULL) threadQueue = thread;
	else {
		while(threadQueue->next != NULL) threadQueue = threadQueue->next;
		threadQueue->next = thread;
	}
	thread->next = NULL;
	threadQueueSize+=1;
}

