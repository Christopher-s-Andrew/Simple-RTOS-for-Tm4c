
#include <stdint.h>
#include "stdio.h"
#include "Filesystem.h"
#include "BSP.h"

#define BUFSIZ 512
#define FOPEN_MAX 1
#define NULL 0
#define FILENAME_MAX 2;

uint8_t stdBuff[BUFSIZ];

uint8_t fileIsOpen = 0;
struct fileStruct openFile;
fpos_t currSectorPos = 0;

// function used to open a file in a certain mode
FILE* fopen(const char *filename, const char *mode){
	
	if( fileIsOpen == 1 || ( mode[0] != 'r' && mode[0] != 'a') )
		return 0;	
		
	uint8_t fileNumber = getFilenum(filename);
	
	// trying to read file that doesn't exist
	if( fileNumber == 255){
		if( *mode == 'a'){
			// create new file
			fileNumber = OS_File_New(filename);
			if( fileNumber == 255)
				return 0; // filesystem is full
			else{
					// New file is open, clear the buffer (all bits are 1)
				for(int i=0; i<512; i++){
					stdBuff[i] = 0xFF;
				}
				OS_File_Append(fileNumber, stdBuff);
			}
		}
		else
			return 0;
	}
	
	currSectorPos = 0;
	fileIsOpen = 1;
	openFile.fileNumber = fileNumber;
	openFile.mode = *mode;
	openFile.currSector = 0; // only used for reads (from start of file)
	
	if( *mode == 'r'){
		// pull beginning of file into buffer
		OS_File_Read( fileNumber, 0, stdBuff);		 								 
	}else{
		uint8_t loc = OS_File_Size(fileNumber) - 1;
		OS_File_Read( fileNumber, OS_File_Size(fileNumber), stdBuff); // Get last sector of file
	}
	
	return &openFile;
}

// Function to close the file
void fclose( FILE* fp){
	if( fp == &openFile){
		
		if( openFile.mode == 'a'){
			// save buffer in last sector location
			OS_File_RewriteLastSector(openFile.fileNumber, stdBuff);
		}
		fileIsOpen = 0;
		openFile.mode = 0;
		openFile.fileNumber = 255;
		openFile.currSector = 0;
	}
}

// This function uses a Buffer to append characters
//	to the file. If it reaches the end of the Buffer,
//	it will Flash that buffer to memory, erase the Buffer,
//	and append a new one to the file, which will correspond to the current buffer
int fputc(char c, FILE *fp){
	if( fp != &openFile || openFile.mode != 'a')
		return -1;
	
	// Find the next free location
	int i=0;
	while( i<512 && stdBuff[i] != 0xFF){
		i++;
	}
	
	if( i==512){ // no null chars, need to append a sector to file
		
		// save current buffer to file
		OS_File_RewriteLastSector(openFile.fileNumber, stdBuff);
		
		// Clear the buffer
		for(int j=1; j<512; j++){
			stdBuff[j] = 0;
		}
		
		// Add new empty sector at end of file
		if( OS_File_Append(openFile.fileNumber, stdBuff) == 255)
			return -1;
		
		stdBuff[0] = c;
		
	}else{	// append to buffer
		stdBuff[i] = c;
	}
	return 0;
}

char fgetc( FILE *fp){
	if( fp != &openFile || openFile.mode != 'r')
		return -1;
	
	if( currSectorPos == 512){
		// get next sector
		openFile.currSector++;
		currSectorPos = 0;
		
		if( OS_File_Read(openFile.fileNumber, openFile.currSector, stdBuff) == 255)
			return -1; // EOF
	}
	
	char c = stdBuff[currSectorPos];
	currSectorPos++; // increment file pos
	
	// note: 0xFF means EOF 
	
	return c;
}



