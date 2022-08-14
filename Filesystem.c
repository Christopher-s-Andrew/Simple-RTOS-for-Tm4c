// Creates and manages the filesystem

#include <stdint.h>
#include "eDisk.h"

uint8_t Buff[512]; // temporary buffer used during file I/O
uint8_t Directory[256], FAT[256];
uint8_t Filenames[512];
int32_t bDirectoryLoaded =0; // 0 means disk on ROM is complete, 1 means RAM version active

//*****MountDirectory******
// if directory and FAT are not loaded in RAM,
// bring it into RAM from disk
void OS_MountDirectory(void){ 
// if bDirectoryLoaded is 0, 
//    read disk sector 255 and populate Directory and FAT
//    set bDirectoryLoaded=1
// if bDirectoryLoaded is 1, simply return
// **write this function**
	
	if( bDirectoryLoaded)
		return;
	
	eDisk_ReadSector( Buff, 255);
	// Load FAT and Directory arrays
	int i,j;
	for(i=0, j=256; i<256; i++, j++){
		Directory[i] = Buff[i]; // Directory are bytes 0-255
		FAT[i] = Buff[j];				// FAT is bytes 256-511
	}
	
	// Load Filenames
	eDisk_ReadSector( Filenames, 254);
	
	bDirectoryLoaded = 1;
}


// Get the filenumber associated with a filename
uint8_t getFilenum( const char* filename){
	// filename is limited to 2 chars
	uint8_t num = 0;
	char f1 = *filename;
	char f2 = *(filename + 1);
	while( (Filenames[num*2] != f1 || Filenames[num*2+1] != f2) && num < 255){
		num++;
	}
	
	return num;
}


// Return the index of the last sector in the file
// associated with a given starting sector.
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t lastsector(uint8_t start){
// **write this function**
	
	if(start == 255 || start == 254){
		return 255;
	}
	
	int i=0;
	uint8_t currSect = start;
	while( FAT[currSect] != 255 ){
		
		currSect = FAT[currSect]; // go to next sector
		
		i++; // checks for infinite loops
		if( i>254)
			return 255; // error
	}
	
  return currSect; // no error
}

// Return the index of the first free sector.
// Note: This function will loop forever without returning
// if a file has no end or if (Directory[255] != 255)
// (i.e. the FAT is corrupted).
uint8_t findfreesector(void){
// **write this function**
  
	int i=0;
	int fs = -1;
	int ls;
	
	while(1){
		ls = lastsector( Directory[i] );
		if( ls == 255 )
			return (uint8_t)(fs + 1);
		else {
			fs = (fs > ls)? fs : ls;
			i++;
			
			if( i== 254)
				return 255;
		}
	}

}

// Append a sector index 'n' at the end of file 'num'.
// This helper function is part of OS_File_Append(), which
// should have already verified that there is free space,
// so it always returns 0 (successful).
// Note: This function will loop forever without returning
// if the file has no end (i.e. the FAT is corrupted).
uint8_t appendfat(uint8_t num, uint8_t n){
// **write this function**
  
	uint8_t i = Directory[num];
	if( i == 255){
		Directory[num] = n;
		return 0;
	}
	
	
	while( FAT[i] != 255 ){
		i = FAT[i];
	}
	FAT[i] = n;
  return 0;
}

//********OS_File_New*************
// Returns a file number of a new file for writing
// Inputs: filename (2 chars)
// Outputs: number of a new file
// Errors: return 255 on failure or disk full
uint8_t OS_File_New(const char *filename){
// **write this function**
  
	uint8_t i=0;
	
	if( !bDirectoryLoaded ){
		OS_MountDirectory();
	}
	
	while( Directory[i] != 255 && i<254){
		i++;
	}	
	
	if( i < 254){
		Filenames[2*i] = *filename;
		Filenames[2*i+1] = *(filename+1);
	}
  return i;
}

//********OS_File_Size*************
// Check the size of this file
// Inputs:  num, 8-bit file number, 0 to 254
// Outputs: 0 if empty, otherwise the number of sectors
// Errors:  none
uint8_t OS_File_Size(uint8_t num){
// **write this function**
  
	uint8_t size = 1;
	
	if( Directory[num] == 255)
		return 0;	// File is empty
	
	uint8_t currSect = Directory[num];
	
	while( FAT[currSect] != 255){
		currSect = FAT[currSect]; // go to next sector
		size++; // increment sect count
		if( size==254)
			return 255; // error
	}
	
	
  return size; // return number of files
}

//********OS_File_Append*************
// Save 512 bytes into the file
// Inputs:  num, 8-bit file number, 0 to 254
//          buf, pointer to 512 bytes of data
// Outputs: 0 if successful
// Errors:  255 on failure or disk full
uint8_t OS_File_Append(uint8_t num, uint8_t buf[512]){
  
	if( !bDirectoryLoaded ){
		OS_MountDirectory();
	}
		
	uint8_t freeSect = findfreesector();
	
	if( freeSect != 255){
		appendfat(num, freeSect);
		eDisk_WriteSector( buf, freeSect);
		return 0; // success
	}
		
  return 255;
}

//********* OS_File_RewriteLastSector ***********/
//	This will take the last sector of a file and rewrite it
//	This allows the stdio.h file functions to append to a file without having to add a new sector
uint8_t OS_File_RewriteLastSector(uint8_t num, uint8_t buf[512]){
	
	if( !bDirectoryLoaded ){
		OS_MountDirectory();
	}
		
	uint8_t lastSect = lastsector( Directory[num] ); // get last sector of directory
	eDisk_WriteSector( buf, lastSect);
	
	return 255;
}


//********OS_File_Read*************
// Read 512 bytes from the file
// Inputs:  num, 8-bit file number, 0 to 254
//          location, logical address, 0 to 254
//          buf, pointer to 512 empty spaces in RAM
// Outputs: 0 if successful
// Errors:  255 on failure because no data
uint8_t OS_File_Read(uint8_t num, uint8_t location,
                     uint8_t buf[512]){
	// **write this function**
  if( !bDirectoryLoaded ){
		OS_MountDirectory();
	}
		
	if( location >= OS_File_Size(num) ){
		return 255;
	}
						
	uint8_t sector = Directory[num]; // get start sector
	
	for(int i=0; i<location; i++){
		sector = FAT[sector];
	}
	
	eDisk_ReadSector(buf, sector);
											 
  return 0; // replace this line
}

//********OS_File_Flush*************
// Update working buffers onto the disk
// Power can be removed after calling flush
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Flush(void){
// **write this function**

	int i,j;
	if( bDirectoryLoaded ){
		for(i=0, j=256; i<256; i++, j++){
			Buff[i] = Directory[i];
			Buff[j] = FAT[i];
		}
		
		eDisk_WriteSector(Buff, 255);
		eDisk_WriteSector(Filenames, 254);
		return 0;
	}
	
  return 255; // replace this line
}

//********OS_File_Format*************
// Erase all files and all data
// Inputs:  none
// Outputs: 0 if success
// Errors:  255 on disk write failure
uint8_t OS_File_Format(void){
// call eDiskFormat
// clear bDirectoryLoaded to zero
// **write this function**

	eDisk_Format();
	bDirectoryLoaded = 0;
  return 0; // replace this line
}
