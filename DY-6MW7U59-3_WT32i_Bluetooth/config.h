#ifndef __CONFIG_H__
#define __CONFIG_H__

#define NEW_PCB

#define NULL 0
#define USART_BREAK_DETECTION_LBD  //if defined, break on RX line will detect via LBD iterrupt, otherwise via ERROR on line.
#define MODE_INTERRUPT_CYCLES 33 // 33 * 30ms = 1 sec
#define PRESS_DELAY 66 // 2 sec = 30ms * 66

#define CAN1_ReMap // Закоментировать, если нет ремапинга портов

#ifndef CAN1_ReMap
#define CAN1_GPIO_PORT			GPIOA
#define CAN1_RX_SOURCE			GPIO_Pin_11				// RX-порт
#define CAN1_TX_SOURCE			GPIO_Pin_12				// TX-порт
#define CAN1_Periph				RCC_APB2Periph_GPIOA	// Порт перифирии
#else
#define CAN1_GPIO_PORT			GPIOB
#define CAN1_RX_SOURCE			GPIO_Pin_8				// RX-порт
#define CAN1_TX_SOURCE			GPIO_Pin_9				// TX-порт
#define CAN1_Periph				RCC_APB2Periph_GPIOB	// Порт перифирии
#endif



#ifdef NEW_PCB

#define BT_STBY_PIN GPIO_Pin_13
#define BT_STBY_PORT GPIOC
#define REMOTE_ADC_IN_PIN GPIO_Pin_7
#define REMOTE_ADC_IN_PORT GPIOA
#define REMOTE_ADC_CHANNEL ADC_Channel_7
#define DISPLAY_BREAK_SIGNAL_PIN GPIO_Pin_4
#define DISPLAY_BREAK_SIGNAL_PORT GPIOA

#else

#define BT_STBY_PIN GPIO_Pin_4
#define BT_STBY_PORT GPIOA
#define REMOTE_ADC_IN_PIN GPIO_Pin_1
#define REMOTE_ADC_IN_PORT GPIOA
#define REMOTE_ADC_CHANNEL ADC_Channel_1

#endif


#define MAX_QUEUE_COUNT_CALLBACKS 10

#define DISPLAY_STRING_DELAY 10
#define OFFSET_STEP_DELAY 1


#endif
