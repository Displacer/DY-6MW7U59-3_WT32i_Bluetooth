#ifndef __DISPLAYHANDLER_H__
#define __DISPLAYHANDLER_H__
#define DISPLAY_BUFFER_SIZE 23
#define BLUETOOTH_CHAR 0x86
#define ACC_OFF 0x40

void HandleDisplayData(void);
void SendDisplayData(void);
uint8_t Translit(uint8_t);
uint8_t isAux;




#endif
