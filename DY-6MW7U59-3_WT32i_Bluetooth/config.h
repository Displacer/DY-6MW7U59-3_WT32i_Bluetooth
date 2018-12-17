#ifndef __CONFIG_H__
#define __CONFIG_H__

#define NULL 0
#define USART_BREAK_DETECTION_LBD  //if defined, break on rx line will detect via LBD iterrupt, otherwise via ERROR on line.
#define MODE_ITTERUPT_CYCLES 30 // 30 * 30ms = 0.5 sec
#define PRESS_DELAY 66 // 2 sec = 30ms * 66
#define BT_STBY_PIN GPIO_Pin_4
#define REMOTE_ADC_IN_PIN GPIO_Pin_1
#define DISPLAY_BREAK_SIGNAL_PIN GPIO_Pin_2
#define MAX_QUEUE_COUNT_CALLBACKS 10


#endif
