
#ifndef THREADS_H
#define THREADS_H

#include <stdint.h>

// Define Thread info
#define MAXNUMTHREADS  10        // maximum number of threads
#define NUMPERIODIC 5        // maximum number of periodic threads
#define LOWESTPRIORITY 16		// lowest possible priority
#define STACKSIZE   100      // number of 32-bit words in stack per thread


// ************* Public Structures *********************
struct tcb{
  int32_t *sp;       // pointer to stack (valid for threads not running)
  struct tcb *next;  // linked-list pointer
	struct tcb *prev;	 // linked-list previous pointer
	int32_t *blocked; // nonzero if blocked on this semaphore
	int32_t sleep; // nonzero if this thread is sleeping
	uint8_t Priority; // 0 is highest, 254 lowest
	uint32_t blockedTime;
};

// Create tcb blocks and space for thread stacks
typedef struct tcb tcbType;
extern tcbType tcbs[MAXNUMTHREADS+NUMPERIODIC];
extern tcbType *RunPt;
extern int32_t Stacks[MAXNUMTHREADS+NUMPERIODIC][STACKSIZE];
void static runperiodicevents(void);

// Priority Queues Array (array of tcb pointers)
extern tcbType *PriorityQueues[LOWESTPRIORITY + 1];
extern int32_t usedFullTimeslice;
extern int32_t numberOfThreads;
extern int32_t numberOfPeriodic;


// Periodic TCBs 
struct tcbPeriodic{
  void (*fnc)(void);       // pointer to function (valid for threads not running
	int32_t counter;	// pointer to semaphore that is blocking thread (if blocked)
	int32_t period;
};
typedef struct tcbPeriodic tcbPeriodicType;
extern tcbPeriodicType tcbsPeriodic[NUMPERIODIC]; // create tcbsPeriodic array with maximum pthread length


// ****** Public Functions ********
int OS_AddThreads(int32_t numThreads, int32_t *threads );
int OS_AddPeriodicEventThread(void(*thread)(void), uint32_t period);

#endif
