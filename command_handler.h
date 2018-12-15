#ifndef __COMMANDHANDLER_H__
#define __COMMANDHANDLER_H__

#define COMMAND_BUFFER_SIZE 7

#define POWER_BUTTON 0x02
#define CD_BUTTON 0x20
#define FM_BUTTON 0x40
#define FORWARD_BUTTON 0x02
#define BACKWARD_BUTTON 0x01
#define NOTHING 0x00

enum EMainFSM
{
   NORMAL_STATE,
   BT_PREPARE,
   BT_ACTIVATE,
   BT_ACTIVE,
   BT_SHUTTING_DOWN,
   GOING_NORMAL_STATE,
};
void HandleCommandData(void);
void Bluetooth_on(void);
void Bluetooth_off(void);

#endif
