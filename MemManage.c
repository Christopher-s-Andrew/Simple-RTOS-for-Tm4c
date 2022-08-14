#include <stdint.h>
//#include <stdio.h>
#include "MemManage.h"
#include "Filesystem.h" // to use FAT
char mem[5000];
struct memsegment *memlistf=(void*)mem; //initial mem chunk, mem is address

//to initialize mem
void initialize(){
	memlistf->size=5000-sizeof(struct memsegment);
	memlistf->next=NULL; //pointer to next is null because there is no next mem chunk
	memlistf->free=1; //mem segment is not allocated becuase of 1
}

//fucntion to allocate mem space
void *malloc(size_t sizem){
	int32_t *listcheck; // this variable to check whole list and return starting address of mem chuck
	struct memsegment *pt/*,*prev*/;
	//while(FAT[currSect] != 255){
	if(!(memlistf->size)){
		initialize();
	}
	pt=memlistf;
	while(pt->next!=NULL&&(((pt->size)<sizem)||((pt->free)==0))){
		pt=pt->next;
	}
	pt->size = sizem;
	pt->next = pt + sizem + sizeof(struct memsegment);
	
	
	if((pt->size)==sizem){
		pt->free=0;
		listcheck=(void*)(pt + sizeof(struct memsegment));
		return listcheck;
	}
	else{
		listcheck=NULL;
		return listcheck;
	}
}

void free(void* ptr){
	if(((void*)mem<=ptr)&&(ptr<=(void*)(mem+5000))){
	struct memsegment* pt=ptr;
	pt=pt-1;
	pt->free=1;
}
}

