#ifndef __TEA6420_H__
#define __TEA6420_H__
#define TEA6420_I2C I2C1
#define TEA6420_ADDRESS 0x98
#define AUDIO_OUT 0x60
#define BT_IN 0x00
#define AUX_IN 0x04
#define GAIN_0 0x18
#define GAIN_2 0x10
#define GAIN_4 0x08
#define GAIN_6 0x00


void tea6420_init();
void tea6420_Bluetooth();
void tea6420_AUX();




#endif
