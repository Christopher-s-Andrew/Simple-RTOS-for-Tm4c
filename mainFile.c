// Lab4.c Faiza Badarpura-Christopher Andrew-Patrick Kelling
// Runs on either MSP432 or TM4C123
// Starter project to Lab 4.  Take sensor readings, process the data,
// and output the results.  Specifically, this program will
// measure steps using the accelerometer, audio sound amplitude using
// the microphone, temperature using the TMP006, and light using the
// OPT3001.
// Daniel and Jonathan Valvano
// August 22, 2016

/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Introduction to the MSP432 Microcontroller",
   ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2016

   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
   ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2016

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include <stdint.h>
#include "BSP.h"
#include "Profile.h"
#include "Texas.h"
#include "CortexM.h"
#include "os.h"
#include "our_stdio.h"
#include "Filesystem.h"	// only used for erasing the file system (for testing)

#include <stdlib.h>
#include "Queue.h"

//********************** Testing the OS Scheduler ******************************** /
// Task   Type           When to Run
// Task0  data producer  periodically every 20 ms (sleep)
// Task1  data consumer  after TaskA finishes
// Task2  data producer  periodically every 50 ms (sleep)
// Task3  data consumer  after TaskC finishes
// Task4  low prior   	 only runs when Priority boost occurs
// Task5  high prior		 almost constantly  
// Task6  periodic thread - runs depending on Periodic schedule
// RunOnEdge	I/O handler	- runs when button is pressed

int32_t sAB,sCD,edgeSema;
//struct queue* testQueue;
int32_t CountA,CountB,CountC,CountD,CountE,CountF,CountG,CountH;
void Task0(void){ // producer highest priority
  CountA = 0;
  while(1){
    CountA++;
    TExaS_Task0();
    Profile_Toggle0();
    OS_Signal(&sAB);  // TaskB can proceed
    OS_Sleep(20);     
  }
}
void Task1(void){ // consumer
  CountB = 0;
  while(1){
    CountB++;
    OS_Wait(&sAB);  // signaled by TaskA
    TExaS_Task1();
    Profile_Toggle1();
  }
}
void Task2(void){ // producer
  CountC = 0;
  while(1){
    CountC++;
    TExaS_Task2();
    Profile_Toggle2();
    OS_Signal(&sCD);  // TaskD can proceed
    OS_Sleep(50);     
  }
}

void Task3(void){ // consumer
  CountD = 0;
  while(1){
    CountD++;
    OS_Wait(&sCD);  // signaled by TaskC
    TExaS_Task3();
    Profile_Toggle3();
  }
}

void Task4(void){ // never sleeps, should only run only when priority boost occurs
  CountE = 0;
  while(1){
    CountE++;
    TExaS_Task4();
    Profile_Toggle4();
  }
}

void Task5(void){ // should run almost constantly
  CountF = 0;
  while(1){
    CountF++;
    TExaS_Task5();
    Profile_Toggle5();
		OS_Suspend();
  }
}

void Task6(void){ // periodic thread
	// this should run almost exactly every 100ms
  CountG++;
	TExaS_Task6();
  Profile_Toggle6();
}

void RunOnEdge(void){
	while(1){
		OS_Wait(&edgeSema);
		OS_File_Format();
	}
}


// Testing FileSystem
void FileTest(void){
	OS_File_Format();
	
	// Write Test
	FILE* fp = fopen("ab", "a");
	if( fp != 0){
		fputc('a',fp);
		fputc('b',fp);
		fputc('c',fp);
		fclose(fp);
		fp = 0;
	}
	
	// Read Test
	fp = fopen("ab", "r");
	char c;
	int count = 0;
	if( fp != 0){
		while( c = fgetc(fp), c != EOF){
			// just visually inspect what c is here
			count++;
		}
	}
	fclose(fp);
	fp = 0;	// clear dangling pointer
	
	
	// Append to already existing file Test
	fp = fopen("ab", "a");
	if( fp != 0){
		fputc('d',fp);
		fputc('e',fp);
		fputc('f',fp);
		fclose(fp);
		fp = 0;
	}
	
	// Second File Write Test
	fp = fopen("bc", "a");
	if( fp != 0){
		fputc('1',fp);
		fputc('2',fp);
		fputc('3',fp);
		fclose(fp);
		fp = 0;
	}
}

int main_PK(void){
  OS_Init();
  Profile_Init();  // initialize the 7 hardware profiling pins
  OS_InitSemaphore(&sAB, 0);
  OS_InitSemaphore(&sCD, 0);
	OS_InitSemaphore(&edgeSema,0);
	
	
	// Main threads
	int32_t threads[7] = { (int32_t)&Task0, (int32_t)&Task1, (int32_t)&Task2, (int32_t)&Task3, 
													(int32_t)&Task4, (int32_t)&Task5, (int32_t)&RunOnEdge};
	OS_AddThreads(7,threads);
													
	// Run TaskG every 100ms	
	OS_AddPeriodicEventThread(&Task6, 100);	
	CountG = 0;
	
	// edge triggered semaphore signal
	OS_EdgeTrigger_Init(&edgeSema, 3);
													
	// Function that is used to test the file I/O functions
	FileTest();
	
  TExaS_Init(LOGICANALYZER, 1000); // initialize the Lab 4 grader
//  TExaS_Init(GRADESTEP1, 1000);    // initialize the Lab 4 grader
  OS_Launch(BSP_Clock_GetFreq()/1000);
  return 0;             // this never executes
}

////////////////////////////////////////////////////////////////////////////////////////////







//******************************** Chris' Demo *********************************************//

struct gate QGate;
struct gate LCDGate;
struct queue* Q;

uint16_t x, y;
		uint8_t b;
		
int32_t blankSignal;

//blindly polls input button and joystick dumping joystick data into a global
void TaskA(void)
{
	BSP_Joystick_Init();
	BSP_Button1_Init();
	OS_gate_init(&QGate);
	Q = OS_queue_create();
	while(1)
	{
		
		
		BSP_Joystick_Input(&x,&y,&b);
		if(!BSP_Button1_Input())
		{
			
			OS_gate_entry(&QGate);
			uint16_t* joyData  = malloc(sizeof(uint16_t));
			*joyData = x;
			//OS_Sleep(1); //make sure it can hit the trigger
			OS_queue_push(joyData,Q);
			OS_gate_exit(&QGate);
			
			OS_Sleep(500);
		}
	}
}

//manges lcd display and display results
void TaskB(void)
{
	//thread init
	BSP_Button2_Init();
	BSP_LCD_FillScreen(0); 
	OS_gate_init(&LCDGate);
	while(1)
	{
		OS_gate_entry(&LCDGate);
		//check if you need to dumpt items from the queue
		if(!BSP_Button2_Input())
		{
			//attempt to draw out 10 numbers from the queue
			//null pointers returned are printed as 0
				for(int i = 0; i< 10; i++)
				{
					BSP_LCD_SetCursor(0,2+i);
					
					//gate is used to protect the queue from the data input
					//queues have a internal mutex technicly but this does not interfer and demonstrates gates functioning
					OS_gate_entry(&QGate);
					if(OS_queue_peak(Q))
					{
						int* tempData = OS_queue_pop(Q);
						BSP_LCD_OutUDec4(*tempData, 0xFFFF);
						free(tempData);
					}
					else
					{
						BSP_LCD_OutUDec4(0, 0xFFFF);
					}
					OS_gate_exit(&QGate);	
				}
			
			OS_Signal(&blankSignal);
			OS_Sleep(500);
			
		}
		
		//display the current joystick values
		BSP_LCD_SetCursor(1,1);
		BSP_LCD_OutUDec4(x, 0xFFFF);
		
		OS_gate_exit(&LCDGate);
		OS_Sleep(100);
	}
}

//aperiodic event thread, waits 5 seconds after being triggered before blanking the screen
void TaskC(void)
{
	while(1)
	{
	//BSP_LCD_FillScreen(0);  
	OS_Wait(&blankSignal);
	OS_Sleep(5000);
	OS_gate_entry(&LCDGate);
	BSP_LCD_FillScreen(0); 
	OS_gate_exit(&LCDGate);
		
	}
}

//make sure the program does not think it broke and provides a very basic heartbeat
void TaskD(void)
{
	int spining = 0;
	while(1)
		spining++;
}

int main_Chris(void){
  OS_Init();
	BSP_LCD_Init();
	OS_InitSemaphore(&blankSignal,0);
	
	// Create Queue
	//testQueue = OS_queue_create();

	// Main threads
	int32_t threads[4] = { (int32_t)&TaskA, (int32_t)&TaskB, (int32_t)&TaskC,(int32_t)&TaskD};
	OS_AddThreads(4,threads);
													
//  TExaS_Init(GRADESTEP1, 1000);    // initialize the Lab 4 grader
  OS_Launch(BSP_Clock_GetFreq()/1000);
  return 0;             // this never executes
}

//////////////////////////////////////////////////////////////////////////////////////////////

int main(void){
	main_Chris();
	return 0;
}

