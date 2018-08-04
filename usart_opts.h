#ifndef __USARTOPTS_H__
#define __USARTOPTS_H__

#include <stm32f10x.h>
#include <stm32f10x_usart.h>

enum Mode {
	CD, FM, MW, LW, AUX, Bluetooth, Normal, PowerOff //normal for capability, in future will be deleted
} mode;



void UARTSend(USART_TypeDef* usart, const unsigned char *pucBuffer,
		unsigned long ulCount);
void USART2SendDMA();
void USART3SendDMA();
uint8_t CheckChksum(uint8_t* buff, uint8_t size);

#endif
