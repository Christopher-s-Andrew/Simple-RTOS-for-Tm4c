
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>

void OS_InitSemaphore(int32_t *semaPt, int32_t value);
void OS_Wait(int32_t *semaPt);
void OS_Signal(int32_t *semaPt);

#endif
