#include <usart_opts.h>
#include <display_handler.h>
#include <command_handler.h>
#include <stm32f10x.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_dma.h>

void UARTSend(USART_TypeDef* usart, const unsigned char *pucBuffer,
		unsigned long ulCount) {
	//
	// Loop while there are more characters to send.
	//
	while (ulCount--) {
		USART_SendData(usart, *pucBuffer++);
		/* Loop until the end of transmission */
		while (USART_GetFlagStatus(usart, USART_FLAG_TC) == RESET) {
		}
	}
}

void USART1Send(const unsigned char *buf) {
	while (*buf != '\0') {
		USART_SendData(USART1, *buf++);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {
		}
	}

}

void USART2SendDMA() {
	/* Restart DMA Channel*/
	DMA_Cmd(DMA1_Channel7, DISABLE);
	DMA1_Channel7->CNDTR = DISPLAY_BUFFER_SIZE;
	DMA_Cmd(DMA1_Channel7, ENABLE);
}

void USART3SendDMA() {
	/* Restart DMA Channel*/
	DMA_Cmd(DMA1_Channel2, DISABLE);
	DMA1_Channel2->CNDTR = COMMAND_BUFFER_SIZE;
	DMA_Cmd(DMA1_Channel2, ENABLE);
}
uint8_t CheckChksum(uint8_t* buff, uint8_t size) {
	uint8_t chksum = 0;
	for (int i = 0; i < size - 1; i++) {
		chksum += buff[i];
	}
	if (buff[size - 1] != chksum)
		return ERROR;
	return SUCCESS;
}
