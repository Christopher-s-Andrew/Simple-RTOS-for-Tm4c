
#include <stdlib.h>
//#include "os.h"
#include "Queue.h"
#include "os.h"

/**
	a generic data container with an pointer to another data container of the same type
*/
struct listDataBox
{
	void* data;
	struct listDataBox* next;
};



/**
	PURPOSE:
		creates the queue
	PARAMETERS:
		none
	RETURN:
		struct queue* - a pointer to the queue structure
*/
struct queue* OS_queue_create()
{
	struct queue* p_createdQueue = malloc(sizeof(struct queue));
	p_createdQueue->top = 0;
	p_createdQueue->bottom = 0;
	p_createdQueue->queueMutex = 1;
	return p_createdQueue;
}

/**
	PURPOSE:
		pushes information onto the queue
	PARAMETERS:
		void* p_data -							 	a pointer to the data to be pushed onto the queue
	struct queue* p_workingQueue -	a pointer to the queue that is being worked on
	RETURN:
		none
*/
void OS_queue_push(void* p_data, struct queue* p_workingQueue )
{
	OS_Wait(&(p_workingQueue->queueMutex));

	//setup data box and insert data
	struct listDataBox* temp = malloc(sizeof(struct listDataBox));
	temp->data = p_data;
	temp->next = 0;

	//initial 
	if (p_workingQueue->bottom == 0 && p_workingQueue->top == 0)
	{
		p_workingQueue->bottom = p_workingQueue->top = temp;
	}

	p_workingQueue->bottom->next = temp;
	p_workingQueue->bottom = temp;

	OS_Signal(&(p_workingQueue->queueMutex));
}

/**
	PURPOSE:
		pops information into the queue
	PARAMETERS:
		struct queue* workingQueue - the queue that is being worked on
	RETURN:
	void*	- pointer to a peice of information in the queue,
*/
void* OS_queue_pop(struct queue* p_workingQueue)
{
	OS_Wait(&(p_workingQueue->queueMutex));
	if (p_workingQueue->top == 0)
	{
		OS_Signal(&(p_workingQueue->queueMutex));
		return 0;
	}
		

	struct listDataBox* temp = p_workingQueue->top;
	void* outputData = temp->data;
	p_workingQueue->top = p_workingQueue->top->next;

	if (p_workingQueue->top == 0)
		p_workingQueue->bottom = 0;
	free(temp);

	OS_Signal(&(p_workingQueue->queueMutex));
	return outputData;
}

/**
	PURPOSE:
		allows one to veiw the information on the top of the queue without removing it
	PARAMETERS:
		struct queue* workingQueue - the queue that is being worked on
	RETURN:
	void*	- pointer to a peice of information in the queue,
*/
void* OS_queue_peak(struct queue* p_workingQueue)
{
	OS_Wait(&(p_workingQueue->queueMutex));
	if (p_workingQueue->top != 0)
	{
		OS_Signal(&(p_workingQueue->queueMutex));
		return p_workingQueue->top->data;
	}
		
	OS_Signal(&(p_workingQueue->queueMutex));
	return 0;
} 
