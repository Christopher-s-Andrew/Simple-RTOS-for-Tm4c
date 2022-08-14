
#include <stdint.h>
#include "Threads.h"
#include "CortexM.h"


tcbType *PriorityQueues[LOWESTPRIORITY + 1];
tcbType tcbs[MAXNUMTHREADS+NUMPERIODIC];
tcbType *RunPt;
int32_t Stacks[MAXNUMTHREADS+NUMPERIODIC][STACKSIZE];
int32_t usedFullTimeslice;
int32_t numberOfThreads;
int32_t numberOfPeriodic;
tcbPeriodicType tcbsPeriodic[NUMPERIODIC]; // create tcbsPeriodic array with maximum pthread length



void SetInitialStack(int i){
	// Sets initial stack values
	tcbs[i].sp = &Stacks[i][STACKSIZE-16]; // thread stack pointer 
	Stacks[i][STACKSIZE-1] = 0x01000000; // Thumb bit
	Stacks[i][STACKSIZE-3] = 0x14141414; // R14
	Stacks[i][STACKSIZE-4] = 0x12121212; // R12
	Stacks[i][STACKSIZE-5] = 0x03030303; // R3
	Stacks[i][STACKSIZE-6] = 0x02020202; // R2
	Stacks[i][STACKSIZE-7] = 0x01010101; // R1
	Stacks[i][STACKSIZE-8] = 0x00000000; // R0
	Stacks[i][STACKSIZE-9] = 0x11111111; // R11
	Stacks[i][STACKSIZE-10] = 0x10101010; // R10
	Stacks[i][STACKSIZE-11] = 0x09090909; // R9
	Stacks[i][STACKSIZE-12] = 0x08080808; // R8
	Stacks[i][STACKSIZE-13] = 0x07070707; // R7
	Stacks[i][STACKSIZE-14] = 0x06060606; // R6
	Stacks[i][STACKSIZE-15] = 0x05050505; // R5
	Stacks[i][STACKSIZE-16] = 0x04040404; // R4
}



//******** OS_AddThreads ***************
// Add eight main threads to the scheduler
// Inputs: function pointers to eight void/void main threads
//         priorites for each main thread (0 highest)
// Outputs: 1 if successful, 0 if this thread can not be added
// This function will only be called once, after OS_Init and before OS_Launch
int OS_AddThreads(int32_t numThreads, int32_t *threads ){

	numberOfThreads = numThreads;	// set global numberOfThreads value
	int32_t status;
	status = StartCritical();
	
	PriorityQueues[0] = &tcbs[0];
	
	// Setup tcb and Stack for each thread
	for(int i=0; i<numThreads; i++){


		// Set prev pointer
		if( i==0 )
			tcbs[i].prev = &tcbs[numThreads-1];
		else
			tcbs[i].prev = &tcbs[i-1];
		
		// Set next pointer
		if( (i+1) == numThreads)
			tcbs[i].next = &tcbs[0];
		else
			tcbs[i].next = &tcbs[i+1];
		
		SetInitialStack(i); Stacks[i][STACKSIZE-2] = threads[i]; // PC to address of thread
		tcbs[i].blocked = 0; 
		tcbs[i].sleep = 0;
		tcbs[i].Priority = 0;
		tcbs[i].blockedTime = 0;
	}
	
  RunPt = &tcbs[0];       // thread 0 will run first
  EndCritical(status);
  return 1;               // successful
}



//******** OS_AddPeriodicEventThread ***************
// Add one background periodic event thread
// Typically this function receives the highest priority
// Inputs: pointer to a void/void event thread function
//         period given in units of OS_Launch (Lab 3 this will be msec)
// Outputs: 1 if successful, 0 if this thread cannot be added
// It is assumed that the event threads will run to completion and return
// It is assumed the time to run these event threads is short compared to 1 msec
// These threads cannot spin, block, loop, sleep, or kill
// These threads can call OS_Signal
int OS_AddPeriodicEventThread(void(*thread)(void), uint32_t period){
// ****IMPLEMENT THIS****
	
	if(numberOfPeriodic >= NUMPERIODIC)
		return 0;
	
	tcbsPeriodic[numberOfPeriodic].fnc = thread;
	tcbsPeriodic[numberOfPeriodic].period = period;
	tcbsPeriodic[numberOfPeriodic].counter = 0;	// run at start
	
	numberOfPeriodic++;
  return 1;
}
