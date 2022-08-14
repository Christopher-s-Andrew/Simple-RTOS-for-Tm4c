
#include <stdint.h>
#include "Threads.h"
#include "CortexM.h"
#include "BSP.h"

#define BOOSTPERIOD 100		// boost period in ms


void schedulerInit(){
	
	// Set initial PriorityQueue values to NULL (0)
	for(int i=0; i<LOWESTPRIORITY; i++){
		PriorityQueues[i] = 0;
	}
	usedFullTimeslice = 1; // set bc scheduler doesn't run before first task
	numberOfPeriodic = 0;	// number of Periodic threads starts at 0
	
	BSP_PeriodicTask_Init(&runperiodicevents, 1000, 1);
}

void boostPriority(void){
	
	for(int i=0; i<numberOfThreads; i++){
		// Set prev pointer
		if( i==0 )
			tcbs[i].prev = &tcbs[numberOfThreads-1];
		else
			tcbs[i].prev = &tcbs[i-1];
		
		// Set next pointer
		if( (i+1) == numberOfThreads)
			tcbs[i].next = &tcbs[0];
		else
			tcbs[i].next = &tcbs[i+1];
		
		// Reset priority of all threads to 0
		tcbs[i].Priority = 0;
	}
	
	// Reset Priority Queues
	for(int i=0; i<LOWESTPRIORITY; i++){
		PriorityQueues[i] = 0;
	}
	// Current thread is pointed to in highest priority Queue
	PriorityQueues[0] = RunPt;

}


// Function that is run every 1ms to update counters and reset Priorities
void static runperiodicevents(void){
	// Decrement sleep counters every ms
	// Increment blockedTime counters every ms
	static int32_t boostCount = 0;
	boostCount++;
	
	if( boostCount == BOOSTPERIOD){
		boostPriority();
		boostCount = 0;
	}
	
	for(int i=0; i<numberOfPeriodic; i++){
		if(tcbsPeriodic[i].counter != 0)
			tcbsPeriodic[i].counter -= 1;		// decrement counter if > 0
		
		if(tcbsPeriodic[i].counter == 0){			// if counter == 0 -> run thread
			(*tcbsPeriodic[i].fnc)();				// run if counter = 0
			tcbsPeriodic[i].counter = tcbsPeriodic[i].period;		// reset counter after running
		}
	}
	
	for(int i=0;i<(numberOfThreads);i++){
		
		if(tcbs[i].sleep)
			tcbs[i].sleep--;
		
		if(tcbs[i].blocked)
			tcbs[i].blockedTime++;
		
	}
}

void updatePriorityQueues(){
	// Update priority for thread that just ended //
	
	uint32_t newPriority;
	
	
	if( usedFullTimeslice == 1 ){
		
		// Increment current Priority Queue
		if( RunPt->next == RunPt && (RunPt->Priority < LOWESTPRIORITY))
			PriorityQueues[ RunPt->Priority] = 0; // queue is now empty
		else
			PriorityQueues[ RunPt->Priority] = RunPt->next; // increment queue to next tcb
		
		// downgrade thread priority
		uint32_t newPriority = RunPt->Priority;
		if( newPriority < LOWESTPRIORITY){
			newPriority = RunPt->Priority + 1;
			RunPt->Priority = newPriority;
		}
		
		// Patch former Queue and remove RunPt from Queue
		RunPt->prev->next = RunPt->next; // point prev tcb's *next to next tcb
		RunPt->next->prev = RunPt->prev;
		
		// Add RunPt to new Queue
		if( (PriorityQueues[ newPriority ]) == 0 ){
			// empty priority queue
			PriorityQueues[ newPriority ] = RunPt;
			RunPt->next = RunPt;
			RunPt->prev = RunPt;
		}
		else{
			// priority queue has threads in it, add to end of queue
			RunPt->prev = PriorityQueues[ newPriority ]->prev; // set RunPt prev
			RunPt->next = PriorityQueues[ newPriority ];				// set RunPt next
			PriorityQueues[ newPriority ]->prev->next = RunPt;
			PriorityQueues[ newPriority ]->prev = RunPt;
		}
		
	}else{
		newPriority = RunPt->Priority; // Priority is the same
		
		// increment current Priority Queue
		PriorityQueues[ newPriority ] = RunPt->next;
	}
	
}
	
void Scheduler(void){      // every time slice
// ****IMPLEMENT THIS****
// highest priority thread not blocked and not sleeping 
// If there are multiple highest priority (not blocked, not sleeping) run these round robin
	
	updatePriorityQueues(); //update RunPt priority queue
	usedFullTimeslice = 1; 
	
	uint32_t currPriority = 0; //start at minimum priority
	tcbType *pt;
	
	RunPt = 0;
	pt = PriorityQueues[ currPriority ];
	while( RunPt == 0){
		
		if(pt != 0){
			if( ((pt->blocked)==0) && ((pt->sleep)==0))
				RunPt = pt;
			else
				pt = pt->next;
		}
		
		// increment currPriority if necessary
		if( pt == PriorityQueues[currPriority]){
			currPriority++;
			pt = PriorityQueues[currPriority];
		}
		
		// resets currPriority to 0 if necessary (this loop runs until it can schedule something)
		if( currPriority > LOWESTPRIORITY ){
			currPriority = 0;
			pt = PriorityQueues[currPriority];
		}

	}
	
}



//******** OS_Suspend ***************
// Called by main thread to cooperatively suspend operation
// Inputs: none
// Outputs: none
// Will be run again depending on sleep/block status

void OS_Suspend(){
	usedFullTimeslice = 0;
  STCURRENT = 0;        // any write to current clears it
  INTCTRL = 0x04000000; // trigger SysTick
// next thread gets a full time slice
}

// ******** OS_Sleep ************
// place this thread into a dormant state
// input:  number of msec to sleep
// output: none
// OS_Sleep(0) implements cooperative multitasking

void OS_Sleep(uint32_t sleepTime){
// ****IMPLEMENT THIS****
// set sleep parameter in TCB
	RunPt->sleep=sleepTime;
// suspend, stops running
	OS_Suspend();

}

