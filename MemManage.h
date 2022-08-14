//MemManage.h

#ifndef __MEMMANAGE_H
#define __MEMMANAGE_H  1

#include <stdint.h>
//#include <stdio.h>
#include <stddef.h> //for size_t
#include "Filesystem.h" //to use FAT
#include "eDisk.h"



struct memsegment{ //memory chuck
 struct memsegment *next; //next mem chuck
 size_t size;
 uint8_t free; //this is to check is mem chuck is free, if not set to 0
};

void initialize(void); //function called to initialize mem
void *malloc(size_t sizem); //function called to allocate space
void free(void* ptr); //to free mem chuck


#endif
