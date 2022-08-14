#include "Queue.h"
#include <stdlib.h>
#include <stdio.h>

int main()
{
	struct queue* didIScrewUpMem = OS_queue_create();

	for (int i = 0; i < 100; i++)
	{
		int* tempData = malloc(sizeof(int));
		*tempData = i;
		printf("\nPush in %d", *tempData);
	//	printf("\n%d", *tempData);
		OS_queue_push(tempData, didIScrewUpMem);


	}

	printf("START OUTPUT\n");
	for (int i = 0; i < 110; i++)
	{

		int* temp = 0;
		temp = (int*)OS_queue_peak(didIScrewUpMem);
			if (temp != 0)
				printf("\nPeak %d", *temp);
		
		temp = (int*)OS_queue_pop(didIScrewUpMem);

		if (temp != 0)
			printf("\n%d", *temp);
	}
}