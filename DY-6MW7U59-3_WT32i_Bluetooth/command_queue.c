#include <stm32f10x.h>
#include "command_queue.h"
#include "config.h"

int8_t front = 0;
int8_t rear = -1;
uint8_t queue_items_count = 0;
uint8_t exec_delay_timer = MAX_DELAY;

typedef struct DelayedCallback_s
{
	void(*func)();
	uint8_t exec_delay;
} DelayedCallback_t;

DelayedCallback_t DelayedCallbacks[MAX_QUEUE_COUNT_CALLBACKS];

uint8_t isFull()
{
	return queue_items_count == MAX_QUEUE_COUNT_CALLBACKS;
}
uint8_t isEmpty()
{
	return queue_items_count == 0;
}

void ExecuteWithDelay(void(*ptr)(), uint8_t delay)
{
	if (!isFull())
	{
		if (rear == MAX_QUEUE_COUNT_CALLBACKS - 1)
		{
			rear = -1;
		}
		rear++;
		DelayedCallbacks[rear].func = ptr;
		DelayedCallbacks[rear].exec_delay = delay;
		queue_items_count++;
	}
	else return;
}


void CheckEvent()
{
	if (!isEmpty() && exec_delay_timer == MAX_DELAY)
	{
		exec_delay_timer = 0;
	}

	if (exec_delay_timer != MAX_DELAY)
	{
		if (exec_delay_timer == DelayedCallbacks[front].exec_delay)
		{
			uint8_t tmp = front;
			front++;
			exec_delay_timer = MAX_DELAY;			
			if (front == MAX_QUEUE_COUNT_CALLBACKS)
			{
				front = 0;
			}
			queue_items_count--;
			DelayedCallbacks[tmp].func();		
		}
	}
}

void IncTick() //called every 100 ms (every frame on display)
{	
	CheckEvent();
	if (exec_delay_timer != MAX_DELAY)
	{
		exec_delay_timer++;
	}
}
