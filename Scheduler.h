
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

void schedulerInit(void);
void OS_Suspend(void);
void OS_Sleep(uint32_t sleepTime);

#endif
