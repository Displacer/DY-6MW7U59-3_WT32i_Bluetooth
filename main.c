#include <stm32f10x.h>
#include <stm32f10x_conf.h>

#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_dma.h>
#include <misc.h>
#include "display_handler.h"
#include "command_handler.h"
#include "usart_opts.h"

extern uint8_t displayRxBuffer[DISPLAY_BUFFER_SIZE];
extern uint8_t commandRxBuffer[COMMAND_BUFFER_SIZE];
extern uint8_t displayTxBuffer[DISPLAY_BUFFER_SIZE];
extern uint8_t commandTxBuffer[COMMAND_BUFFER_SIZE];

void SetupPeriph() {
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIMER_InitStructure;
	DMA_InitTypeDef DMA_InitStruct;

	///-------------------------------------------------
	///USART2 init
	///-------------------------------------------------
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	//USART2 Tx PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//USART2 Rx PA3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 14400;
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits = USART_StopBits_2;
	USART_InitStructure.USART_Parity = USART_Parity_Even;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	USART_Cmd(USART2, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2, USART_IT_ERR, ENABLE);

	///-------------------------------------------------
	///USART3 init
	///-------------------------------------------------
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	//USART3 Tx PB10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//USART3 Rx PB11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	USART_Init(USART3, &USART_InitStructure);
	USART_Cmd(USART3, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

	///-------------------------------------------------
	///TIM2 for USART2
	///-------------------------------------------------

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseStructInit(&TIMER_InitStructure);
	TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIMER_InitStructure.TIM_Prescaler = 7200;
	TIMER_InitStructure.TIM_Period = 300; //30ms
	TIM_TimeBaseInit(TIM2, &TIMER_InitStructure);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

	/* NVIC Configuration */
	/* Enable the TIM2_IRQn Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	///-------------------------------------------------
	///TIM3 for USART3
	///-------------------------------------------------

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructInit(&TIMER_InitStructure);
	TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIMER_InitStructure.TIM_Prescaler = 7200;
	TIMER_InitStructure.TIM_Period = 100; // 10ms
	TIM_TimeBaseInit(TIM3, &TIMER_InitStructure);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);

	/* NVIC Configuration */
	/* Enable the TIM3_IRQn Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	///-------------------------------------------------
	///DMA Tx for USART2
	///-------------------------------------------------

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & (USART2->DR);
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) & displayTxBuffer[0];
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStruct.DMA_BufferSize = DISPLAY_BUFFER_SIZE;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStruct.DMA_Priority = DMA_Priority_Low;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel7, &DMA_InitStruct);

	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
	DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);
	NVIC_EnableIRQ(DMA1_Channel7_IRQn);

	///-------------------------------------------------
	///DMA Tx for USART3
	///-------------------------------------------------

	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & (USART3->DR);
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) & commandTxBuffer[0];
	DMA_InitStruct.DMA_BufferSize = COMMAND_BUFFER_SIZE;
	DMA_Init(DMA1_Channel2, &DMA_InitStruct);

	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
	NVIC_EnableIRQ(DMA1_Channel2_IRQn);

	/////////////////////////

}

void InitDisplay() {
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_DeInit(GPIOA);
	GPIO_DeInit(GPIOB);
	USART_DeInit(USART2);
	USART_DeInit(USART3);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_2);
	for (uint32_t i = 0; i < 1000000; i++) {
		//Dummy delay for display initialization
	}
	//SetupPeriph();
}

uint8_t d_idx, d_rcvcplt = 0;
void USART2_IRQHandler(void) {
	if ((USART2->SR & (USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
			!= (u16) RESET) // Error handler for display init detection
			{


		d_rcvcplt = 1;
		USART_ReceiveData(USART2);
		InitDisplay();
		SetupPeriph();
	}

	else if ((USART2->SR & USART_FLAG_RXNE) != (u16) RESET) {
		if (d_idx < DISPLAY_BUFFER_SIZE) {
			displayRxBuffer[d_idx++] = USART_ReceiveData(USART2);
			TIM2->CNT = 0;
			d_rcvcplt = 1;
		}
	}

}

uint8_t c_idx, c_rcvcplt = 0;
void USART3_IRQHandler(void) {

	if ((USART3->SR & USART_FLAG_RXNE) != (u16) RESET) {
		if (c_idx < DISPLAY_BUFFER_SIZE) {
			commandRxBuffer[c_idx++] = USART_ReceiveData(USART3);
			TIM3->CNT = 0;
			c_rcvcplt = 1;
		}
	}

}

void TIM2_IRQHandler(void) {
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		if (d_rcvcplt) {
			GPIOC->ODR ^= GPIO_Pin_13;
			HandleDisplayData();
			USART_ReceiveData(USART2); //For clear IDLE flag
			d_rcvcplt = 0;
			d_idx = 0;
		}
	}
}
void TIM3_IRQHandler(void) {
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		if (c_rcvcplt) {
			HandleCommandData();
			USART_ReceiveData(USART3); //For clear IDLE flag
			c_rcvcplt = 0;
			c_idx = 0;
		}
	}
}

void DMA1_Channel7_IRQHandler(void) {
	DMA_ClearITPendingBit(DMA1_IT_TC7);
	DMA_Cmd(DMA1_Channel7, DISABLE);
}

void DMA1_Channel2_IRQHandler(void) {
	DMA_ClearITPendingBit(DMA1_IT_TC2);
	DMA_Cmd(DMA1_Channel2, DISABLE);
}

void SetupClock() {
	RCC_DeInit(); /* RCC system reset(for debug purpose)*/
	RCC_HSEConfig(RCC_HSE_ON); /* Enable HSE                         */

	/* Wait till HSE is ready                                               */
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
		;

	RCC_HCLKConfig(RCC_SYSCLK_Div1); /* HCLK   = SYSCLK                */
	RCC_PCLK2Config(RCC_HCLK_Div1); /* PCLK2  = HCLK                  */
	RCC_PCLK1Config(RCC_HCLK_Div2); /* PCLK1  = HCLK/2                */
	RCC_ADCCLKConfig(RCC_PCLK2_Div4); /* ADCCLK = PCLK2/4               */

	/* PLLCLK = 8MHz * 9 = 72 MHz                                           */
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

	RCC_PLLCmd(ENABLE); /* Enable PLL                     */

	/* Wait till PLL is ready                                               */
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		;

	/* Select PLL as system clock source                                    */
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

	/* Wait till PLL is used as system clock source                         */
	while (RCC_GetSYSCLKSource() != 0x08)
		;
}

int main(void) {
	SetupClock();
	InitDisplay();
	SetupPeriph();

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOC, &GPIO_InitStructure);
	while (1) {

	}

	return 0;
}
