// This file handles all semaphore functionality

#include <stdint.h>
#include "CortexM.h"
#include "Scheduler.h"
#include "Threads.h"


// ******** OS_InitSemaphore ************
// Initialize counting semaphore
// Inputs:  pointer to a semaphore
//          initial value of semaphore
// Outputs: none

void OS_InitSemaphore(int32_t *semaPt, int32_t value){
// ****IMPLEMENT THIS****
// Same as Lab 3
	(*semaPt)=value;
}

// ******** OS_Wait ************
// Decrement semaphore and block if less than zero
// Lab2 spinlock (does not suspend while spinning)
// Lab3 block if less than zero
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Wait(int32_t *semaPt){
// ****IMPLEMENT THIS****
// Same as Lab 3
	DisableInterrupts();
	(*semaPt)=(*semaPt)-1;
	if((*semaPt)<0){
		RunPt->blocked=semaPt;
		RunPt->blockedTime = 0;
		EnableInterrupts();
		OS_Suspend();
	}
	EnableInterrupts();
}

// ******** OS_Signal ************
// Increment semaphore
// wakeup longest waiting blocked thread if appropriate (resolution for waiting time is 1ms)
// Inputs:  pointer to a counting semaphore
// Outputs: none

void OS_Signal(int32_t *semaPt){
// ****IMPLEMENT THIS****

	tcbType *longestWaitingThread = 0;
	int32_t maxBlockTime = 0;
	
	DisableInterrupts();
	(*semaPt)=(*semaPt)+1;
	
	if((*semaPt)<=0){
	
		// find longest waiting thread (resolution is only 1ms) and unblock it
		for(int i=0; i<(numberOfThreads+numberOfPeriodic); i++){
			if( (tcbs[i].blocked == semaPt) && (tcbs[i].blockedTime >= maxBlockTime)){
				maxBlockTime = tcbs[i].blockedTime;
				longestWaitingThread = &tcbs[i];
			}
		}
		
		longestWaitingThread->blocked=0;
		longestWaitingThread->blockedTime = 0;
		
	}
	EnableInterrupts();
}
