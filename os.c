// os.c
// Advanced RTOS Implementation

#include <stdint.h>
#include "os.h"
#include "CortexM.h"
#include "BSP.h"
#include "../inc/tm4c123gh6pm.h"
#include "Filesystem.h"
#include "Scheduler.h"
#include "Semaphore.h"
#include "Threads.h"
#include "stdio.h"

// function definitions in osasm.s
void StartOS(void);

// ******** OS_Init ************
// Initialize operating system, disable interrupts
// Initialize OS controlled I/O: periodic interrupt, bus clock as fast as possible
// Initialize OS global variables
// Inputs:  none
// Outputs: none
void OS_Init(void){
  DisableInterrupts();
  BSP_Clock_InitFastest();	// set processor clock to fastest speed
	
	
	// Scheduler Init
	schedulerInit();

	// filesystem init
	OS_MountDirectory();
}


//******** OS_Launch ***************
// Start the scheduler, enable interrupts
// Inputs: number of clock cycles for each time slice
// Outputs: none (does not return)
// Errors: theTimeSlice must be less than 16,777,216
void OS_Launch(uint32_t theTimeSlice){
  STCTRL = 0;                  // disable SysTick during setup
  STCURRENT = 0;               // any write to current clears it
  SYSPRI3 =(SYSPRI3&0x00FFFFFF)|0xE0000000; // priority 7
  STRELOAD = theTimeSlice - 1; // reload value
  STCTRL = 0x00000007;         // enable, core clock and interrupt arm
  StartOS();                   // start on the first task
}


#define FSIZE 10    // can be any size
uint32_t PutI;      // index of where to put next
uint32_t GetI;      // index of where to get next
uint32_t Fifo[FSIZE];
int32_t CurrentSize;// 0 means FIFO empty, FSIZE means full
uint32_t LostData;  // number of lost pieces of data

// ******** OS_FIFO_Init ************
// Initialize FIFO.  The "put" and "get" indices initially
// are equal, which means that the FIFO is empty.  Also
// initialize semaphores to track properties of the FIFO
// such as size and busy status for Put and Get operations,
// which is important if there are multiple data producers
// or multiple data consumers.
// Inputs:  none
// Outputs: none
void OS_FIFO_Init(void){
// ****IMPLEMENT THIS****
// Same as Lab 3
	PutI = GetI = 0;
	OS_InitSemaphore(&CurrentSize,0);
	LostData = 0;
}


// ******** OS_FIFO_Put ************
// Put an entry in the FIFO.  Consider using a unique
// semaphore to wait on busy status if more than one thread
// is putting data into the FIFO and there is a chance that
// this function may interrupt itself.
// Inputs:  data to be stored
// Outputs: 0 if successful, -1 if the FIFO is full
int OS_FIFO_Put(uint32_t data){
// ****IMPLEMENT THIS****
// Same as Lab 3
	if(CurrentSize==FSIZE){
		LostData++;
		return -1;
	} else{
		Fifo[PutI] = data;
		PutI = (PutI+1)%FSIZE;
		OS_Signal(&CurrentSize);
		return 0; // success
	}
}


// ******** OS_FIFO_Get ************
// Get an entry from the FIFO.  Consider using a unique
// semaphore to wait on busy status if more than one thread
// is getting data from the FIFO and there is a chance that
// this function may interrupt itself.
// Inputs:  none
// Outputs: data retrieved
uint32_t OS_FIFO_Get(void){
	uint32_t data;
// ****IMPLEMENT THIS****
// Same as Lab 3
	OS_Wait(&CurrentSize);
	data = Fifo[GetI];
	GetI = (GetI+1)%FSIZE;
  return data;
}



//****edge-triggered event************
int32_t *edgeSemaphore;
// ******** OS_EdgeTrigger_Init ************
// Initialize button1, PD6, to signal on a falling edge interrupt
// Inputs:  semaphore to signal
//          priority
// Outputs: none
void OS_EdgeTrigger_Init(int32_t *semaPt, uint8_t priority){
	edgeSemaphore = semaPt;
//***IMPLEMENT THIS***
SYSCTL_RCGCGPIO_R |= 0x00000008; // 1) activate clock for Port D
while((SYSCTL_PRGPIO_R&0x00000008) == 0){};/// allow time for clock to stabilize
// 2) no need to unlock PD6
	
	GPIO_PORTD_LOCK_R = 0x4C4F434B;	// 2) unlock PD7
	GPIO_PORTD_CR_R = 0xFF;
	
GPIO_PORTD_AMSEL_R &= ~0x80;// 3) disable analog on PD6
GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xF0FFFFFF)+0x00000000;// 4) configure PD6 as GPIO
GPIO_PORTD_DIR_R &= ~0x80;// 5) make PD6 input
GPIO_PORTD_AFSEL_R &= ~0x80;// 6) disable alt funct on PD6
GPIO_PORTD_PUR_R &= ~0x80;// disable pull-up on PD6
GPIO_PORTD_DEN_R |= 0x80;// 7) enable digital I/O on PD6  
GPIO_PORTD_IS_R &= ~0x80;// (d) PD6 is edge-sensitive 
GPIO_PORTD_IBE_R &= ~0x80;//     PD6 is not both edges 
GPIO_PORTD_IEV_R &= ~0x80;//     PD6 is falling edge event 
GPIO_PORTD_ICR_R = 0x80;// (e) clear PD6 flag
GPIO_PORTD_IM_R |= 0x80;// (f) arm interrupt on PD6
NVIC_PRI0_R = (NVIC_PRI0_R&0x1FFFFFFF)|((uint32_t)(priority) << 29);// priority on Port D edge trigger is NVIC_PRI0_R	31 – 29
NVIC_EN0_R = 0x00000008;// enable is bit 3 in NVIC_EN0_R
 }


 // ******** OS_EdgeTrigger_Restart ************
// restart button1 to signal on a falling edge interrupt
// rearm interrupt
// Inputs:  none
// Outputs: none
void OS_EdgeTrigger_Restart(void){
	//***IMPLEMENT THIS***
	GPIO_PORTD_IM_R |= 0x80;// rearm interrupt 3 in NVIC
	GPIO_PORTD_ICR_R = 0x80;// clear flag6
}

void GPIOPortD_Handler(void){
	//***IMPLEMENT THIS***
	GPIO_PORTD_ICR_R = 0x80;	// step 1 acknowledge by clearing flag
	OS_Signal(edgeSemaphore);  // step 2 signal semaphore (no need to run scheduler)
	GPIO_PORTD_IM_R &= ~0x80;  // step 3 disarm interrupt to prevent bouncing to create multiple signals
}

void OS_gate_init(struct gate* gate)
{
	gate->gate_block = 0;
	gate->GateMutex = 1;
	gate->restorePriority = LOWESTPRIORITY;
}


void OS_gate_entry(struct gate* gate)
{
	DisableInterrupts();
	//enter the gate
	//check if something is blocking the gate
	//if something is blocking, perform priority adjustment stuff
	//if not do nothing and prodece to the wait

	if(gate->gate_block == 0 && gate->GateMutex == 1) //makes sure there is nothing waiting to avoid something entering and being unlocked to continue and there is nothing in the gate
	{
		//store runPt into the block and store its current priority
		gate->gate_block = RunPt;
		gate->restorePriority = RunPt->Priority;
	}
	else if(gate->gate_block != 0) //if priority adjustment is needed
	{
		if(RunPt->Priority < gate->gate_block->Priority)
		{
			if( gate->gate_block->next == gate->gate_block && (gate->gate_block->Priority < LOWESTPRIORITY))
				PriorityQueues[gate->gate_block->Priority] = 0; // queue is now empty
			else
				PriorityQueues[ gate->gate_block->Priority] = gate->gate_block->next;
			
			// Patch former Queue and remove gate->gate_block from Queue
			gate->gate_block->prev->next = gate->gate_block->next; // point prev tcb's *next to next tcb
			gate->gate_block->next->prev = gate->gate_block->prev;
			
			//assign new priority here
			gate->gate_block->Priority = RunPt->Priority;
			
			if((PriorityQueues[ gate->gate_block->Priority ]) == 0)
			{
			// empty priority queue
			PriorityQueues[ gate->gate_block->Priority ] = gate->gate_block;
			gate->gate_block->next = gate->gate_block;
			gate->gate_block->prev = gate->gate_block;
			}
			else
			{
				// priority queue has threads in it, add to end of queue
				gate->gate_block->prev = PriorityQueues[ gate->gate_block->Priority ]->prev; // set RunPt prev
				gate->gate_block->next = PriorityQueues[ gate->gate_block->Priority ];				// set RunPt next
				PriorityQueues[ gate->gate_block->Priority ]->prev->next = gate->gate_block;
				PriorityQueues[ gate->gate_block->Priority ]->prev = gate->gate_block;
			}
			
			PriorityQueues[ gate->gate_block->Priority ] = gate->gate_block->next; //inserts the changed priority thread at the end of the queue
		}
	}

	//wait for run to run, may need to swap this for a safer version of OS_wait()
	OS_Wait(&gate->GateMutex);

	//setup yourself as the new gate thread
	DisableInterrupts();
	gate->gate_block = RunPt;
	gate->restorePriority = RunPt->Priority;
	EnableInterrupts();
	}

void OS_gate_exit(struct gate* gate)
{
		DisableInterrupts();

		if( RunPt->next == RunPt && (RunPt->Priority < LOWESTPRIORITY))
			PriorityQueues[RunPt->Priority] = 0; // queue is now empty
		else
			PriorityQueues[RunPt->Priority] = RunPt->next;
		
		// Patch former Queue and remove gate->gate_block from Queue
		RunPt->prev->next = RunPt->next; // point prev tcb's *next to next tcb
		RunPt->next->prev = RunPt->prev;
		
		//assign new priority here
		RunPt->Priority = gate->restorePriority;
		
		if((PriorityQueues[RunPt->Priority]) == 0)
		{
		// empty priority queue
		PriorityQueues[RunPt->Priority ] = RunPt;
		RunPt->next = RunPt;
		RunPt->prev = RunPt;
		}
		else
		{
			// priority queue has threads in it, add to end of queue
			RunPt->prev = PriorityQueues[ RunPt->Priority ]->prev; // set RunPt prev
			RunPt->next = PriorityQueues[RunPt->Priority ];				// set RunPt next
			PriorityQueues[ RunPt->Priority ]->prev->next = RunPt;
			PriorityQueues[ RunPt->Priority ]->prev = RunPt;
		}
		
		//PriorityQueues[ gate->gate_block->Priority ] = gate->gate_block->next;
		
		
		gate->gate_block = 0;
		gate->restorePriority = LOWESTPRIORITY;
		OS_Signal(&gate->GateMutex);
		EnableInterrupts();
	}

