#ifndef __COMMAND_QUEUE_H__
#define __COMMAND_QUEUE_H__
#include <stm32f10x.h>

#define MAX_DELAY 0xFF

void IncTick(void);
void ExecuteWithDelay(void(*ptr)(), uint8_t delay);

#endif
