#ifndef __COMMANDHANDLER_H__
#define __COMMANDHANDLER_H__
#define MODE_ITTERUPT_CYCLES 30
#define COMMAND_BUFFER_SIZE 7
#define PRESS_DELAY 66 // 2 sec = 30ms * 66
#define POWER_BUTTON 0x02
#define CD_BUTTON 0x20
#define FM_BUTTON 0x40
#define BT_PLAY GPIO_Pin_15
#define BT_PREV GPIO_Pin_14
#define BT_NEXT GPIO_Pin_13

void HandleCommandData(void);
void Bluetooth_on(void);
void Bluetooth_off(void);


#endif
