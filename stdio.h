
#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>

#define EOF -1

// Defines the FILE structure which *FILE will point to
struct fileStruct{
	uint8_t fileNumber;
	uint8_t mode;
	uint8_t currSector;
};
typedef struct fileStruct FILE;

// position in file, currently only used internally
typedef uint16_t fpos_t;

// Function used to open a file, works like normal stdio.h function
//	-only has 'r' (read) and 'a' (append) modes
FILE *fopen(const char *filename, const char *mode);

// Function to close file
void fclose( FILE* fp);

// Append a character to a file (for 'a' mode only)
int fputc(char c, FILE *fp);

// Get a character from the file and advance file position
char fgetc( FILE *fp);

#endif

