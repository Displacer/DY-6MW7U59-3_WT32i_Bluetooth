#include "iwrap.h"
#include "usart_opts.h"
#include <string.h>
#include <stdio.h>

uint8_t bt_device_addr[BT_DEVICE_ADDR_SIZE];
uint8_t bt_device_name[BT_DEVICE_NAME_SIZE];


void bt_GetDeviceName()
{
	if (*bt_device_addr == 0x00) return;
	char tmp[5 + BT_DEVICE_ADDR_SIZE + 4];
	sprintf(tmp, "NAME %s\r\n", bt_device_addr);
	//memcpy(tmp, (uint8_t*)"NAME ", 5);
	//memcpy(&tmp[5], bt_device_addr, BT_DEVICE_ADDR_SIZE);
	//memcpy(&tmp[5 + BT_DEVICE_ADDR_SIZE], (uint8_t*)" \r\n\0", 4);
	USART1Send(tmp);
}
void bt_GetBtDeviceAddres()
{
	USART1Send(BT_LIST_CMD);
}

void bt_Play() 
{
	USART1Send(AVRCP_PLAY);
}
void bt_Pause()
{
	USART1Send(AVRCP_PAUSE);
}
void bt_Next() 
{
	USART1Send(AVRCP_NEXT);
}
void bt_Prev() 
{
	USART1Send(AVRCP_PREV);
}

void bt_TrackChangedEventSubscribe()
{
	USART1Send(EVENT_TRACK_CHANGED);
}

void bt_PlaybackStatusEventSubscribe()
{
	USART1Send(EVENT_PLAYBACK_STATUS_CHANGED);
}

void bt_GetAVRCP_metadata() 
{
	USART1Send(AVRCP_GET_ARTIST_TITLE);
}

