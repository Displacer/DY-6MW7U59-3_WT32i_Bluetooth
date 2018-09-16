#ifndef __DISPLAYHANDLER_H__
#define __DISPLAYHANDLER_H__
#define DISPLAY_BUFFER_SIZE 23
#define DISPLAY_STRING_SIZE 12
#define DISPLAY_DATA_SIZE 300
#define BLUETOOTH_CHAR 0x86
#define ACC_OFF 0x40

void HandleDisplayData(void);
void SendDisplayData(void);
void GetMode(void);
uint8_t isAux;




#endif
