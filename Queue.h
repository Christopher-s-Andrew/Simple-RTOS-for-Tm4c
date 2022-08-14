#ifndef QUEUE_H 
#define QUEUE_H
struct queue
{
	struct listDataBox* top;
	struct listDataBox* bottom;
	int queueMutex;
};

/**
	PURPOSE:
		creates the queue
	PARAMETERS:
		none
	RETURN:
		struct queue* - a pointer to the queue structure
*/
struct queue* OS_queue_create(void);

/**
	PURPOSE:
		pushes information onto the queue
	PARAMETERS:
		void* p_data -							 	a pointer to the data to be pushed onto the queue
	struct queue* p_workingQueue -	a pointer to the queue that is being worked on
	RETURN:
		none
*/
void OS_queue_push(void* p_data, struct queue* p_workingQueue);

/**
	PURPOSE:
		pops information into the queue
	PARAMETERS:
		struct queue* workingQueue - the queue that is being worked on
	RETURN:
	void*	- pointer to a peice of information in the queue,
*/
void* OS_queue_pop(struct queue* p_workingQueue);

/**
	PURPOSE:
		allows one to veiw the information on the top of the queue without removing it
	PARAMETERS:
		struct queue* workingQueue - the queue that is being worked on
	RETURN:
	void*	- pointer to a peice of information in the queue,
*/
void* OS_queue_peak(struct queue* workingQueue);

#endif
