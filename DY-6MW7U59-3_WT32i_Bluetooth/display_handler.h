#ifndef __DISPLAYHANDLER_H__
#define __DISPLAYHANDLER_H__
#include <stm32f10x.h>

#define DISPLAY_BUFFER_SIZE 23
#define DISPLAY_STRING_SIZE 12
#define DISPLAY_DATA_SIZE 300
#define BLUETOOTH_CHAR 0x86
#define ACC_OFF 0x40

void ResetDisplayState(void);
void ClearDisplayString(void);
void ClearDisplayBtString(void);
void HandleDisplayData(void);
void CheckMode(void);
void ForceShowString(const char*);
uint8_t isAux;

#endif
