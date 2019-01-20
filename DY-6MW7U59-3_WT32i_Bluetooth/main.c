#include <stm32f10x.h>

#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_i2c.h>
#include <stm32f10x_adc.h>
#include <misc.h>
#include "display_handler.h"
#include "command_handler.h"
#include "usart_opts.h"
#include "parser.h"
#include "config.h"

#ifndef NEW_PCB
#include "tea6420.h"  
#endif



extern uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];
extern uint8_t commandBuffer[COMMAND_BUFFER_SIZE];
extern uint8_t mode_interrupt;
extern enum EMainFSM main_fsm;

void SetupPeriph()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIMER_InitStructure;
	DMA_InitTypeDef DMA_InitStruct;
	I2C_InitTypeDef I2C_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	///-------------------------------------------------
	///USART1 for WT32i
	///-------------------------------------------------
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	//USART1 Tx PA9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//USART1 Rx PA10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
	      USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	///-------------------------------------------------
	///USART2 for Display
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

	USART_LINBreakDetectLengthConfig(USART2, USART_LINBreakDetectLength_11b);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

#ifdef USART_BREAK_DETECTION_LBD
	USART_ITConfig(USART2, USART_IT_LBD, ENABLE);
#else
	USART_ITConfig(USART2, USART_IT_ERR, ENABLE);
#endif

	///-------------------------------------------------
	///USART3 for Commands
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

	USART_InitStructure.USART_BaudRate = 13800;
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
	TIMER_InitStructure.TIM_Period = 300;     //30ms
	TIM_TimeBaseInit(TIM2, &TIMER_InitStructure);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

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
	TIMER_InitStructure.TIM_Period = 100;     // 10ms
	TIM_TimeBaseInit(TIM3, &TIMER_InitStructure);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);

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
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) & displayBuffer[0];
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
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) & commandBuffer[0];
	DMA_InitStruct.DMA_BufferSize = COMMAND_BUFFER_SIZE;
	DMA_Init(DMA1_Channel2, &DMA_InitStruct);

	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
	NVIC_EnableIRQ(DMA1_Channel2_IRQn);
	
#ifndef NEW_PCB
	///-------------------------------------------------
	///I2C for Audio Matrix
	///-------------------------------------------------	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x38;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 100000;

	I2C_Cmd(TEA6420_I2C, ENABLE);
	I2C_Init(TEA6420_I2C, &I2C_InitStructure);
#endif

	///-------------------------------------------------
	///GPIOs for control
	///-------------------------------------------------

	GPIO_InitStructure.GPIO_Pin = BT_STBY_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(BT_STBY_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(BT_STBY_PORT, BT_STBY_PIN);
	
#ifdef NEW_PCB
	GPIO_InitStructure.GPIO_Pin = DISPLAY_BREAK_SIGNAL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DISPLAY_BREAK_SIGNAL_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(DISPLAY_BREAK_SIGNAL_PORT, DISPLAY_BREAK_SIGNAL_PIN);
				  
#endif // NEW_PCB
	
	///-------------------------------------------------
	///ADC for remote
	///-------------------------------------------------

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = REMOTE_ADC_IN_PIN;
	GPIO_Init(REMOTE_ADC_IN_PORT, &GPIO_InitStructure);

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;

	ADC_RegularChannelConfig(ADC1, REMOTE_ADC_CHANNEL, 1, ADC_SampleTime_28Cycles5);
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_Cmd(ADC1, ENABLE);
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1)) ;
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1)) ;

	ADC_Cmd(ADC1, ENABLE);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void InitDisplay()
{
#ifdef NEW_PCB
	GPIO_SetBits(DISPLAY_BREAK_SIGNAL_PORT, DISPLAY_BREAK_SIGNAL_PIN);	
#else
	GPIO_DeInit(GPIOA);
	GPIO_DeInit(GPIOB);
	USART_DeInit(USART2);
	USART_DeInit(USART3);
#endif
	
	for (uint32_t i = 0; i < 900000; i++)
	{
		asm("NOP");
		//Dummy delay for display initialization
	}
	mode_interrupt = MODE_INTERRUPT_CYCLES * 2;
	
#ifdef NEW_PCB
	GPIO_ResetBits(DISPLAY_BREAK_SIGNAL_PORT, DISPLAY_BREAK_SIGNAL_PIN);
#else
	SetupPeriph();
	if (main_fsm != BT_ACTIVE)
		tea6420_AUX();
	else
		tea6420_Bluetooth();
#endif
}

void USART1_IRQHandler(void)
{
	if ((USART1->SR & USART_FLAG_RXNE) != (u16) RESET)
	{
		if (!isParsing())
			Parse_init();
		Parse(USART_ReceiveData(USART1));
	}
}

uint8_t d_idx, d_rcvcplt = 0;

void USART2_IRQHandler(void)
{

#ifdef USART_BREAK_DETECTION_LBD
	if ((USART2->SR & USART_FLAG_LBD) != (u16) RESET) // Break detection handler for display init detection
		{
			USART_ClearFlag(USART2, USART_FLAG_LBD);
			d_rcvcplt = 1;
			USART_ReceiveData(USART2);
			InitDisplay();
		}

#else
	if ((USART2->SR & (USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE))
	      != (u16) RESET) // Error handler for display init detection
		{
			d_rcvcplt = 1;
			USART_ReceiveData(USART2);
			InitDisplay();
		}
#endif

	if ((USART2->SR & USART_FLAG_RXNE) != (u16) RESET)
	{
		if (d_idx < DISPLAY_BUFFER_SIZE)
		{
			displayBuffer[d_idx++] = USART_ReceiveData(USART2);
			TIM2->CNT = 0;
			d_rcvcplt = 1;
		}
	}
}

uint8_t c_idx, c_rcvcplt = 0;
void USART3_IRQHandler(void)
{
	if ((USART3->SR & USART_FLAG_RXNE) != (u16) RESET)
	{
		if (c_idx < COMMAND_BUFFER_SIZE)
		{
			commandBuffer[c_idx++] = USART_ReceiveData(USART3);
			TIM3->CNT = 0;
			c_rcvcplt = 1;
		}
	}
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		if (d_rcvcplt)
		{
			HandleDisplayData();
			USART_ReceiveData(USART2);     //For clear IDLE flag
			d_rcvcplt = 0;
			d_idx = 0;
		}
	}
}
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		if (c_rcvcplt)
		{
			HandleCommandData();
			USART_ReceiveData(USART3);     //For clear IDLE flag
			c_rcvcplt = 0;
			c_idx = 0;
		}
	}
}

void DMA1_Channel7_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA1_IT_TC7);
	DMA_Cmd(DMA1_Channel7, DISABLE);
}

void DMA1_Channel2_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA1_IT_TC2);
	DMA_Cmd(DMA1_Channel2, DISABLE);
}

void SetupClock()
{
	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON);
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) ;
	RCC_HCLKConfig(RCC_SYSCLK_Div1);     // HCLK   = SYSCLK
	RCC_PCLK2Config(RCC_HCLK_Div1);     // PCLK2  = HCLK
	RCC_PCLK1Config(RCC_HCLK_Div2);     // PCLK1  = HCLK/2
	RCC_ADCCLKConfig(RCC_PCLK2_Div4);     // ADCCLK = PCLK2/4

	// PLLCLK = 8MHz * 9 = 72 MHz
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
	RCC_PLLCmd(ENABLE);
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) ;
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	while (RCC_GetSYSCLKSource() != 0x08) ;
}

int main(void)
{
	SetupClock();
	SetupPeriph();

	while(1)
	{

	}

	return 0;
}
