/*
 * change track event
 * AVRCP PDU 31 2
 * AVRCP 0 REGISTER_NOTIFICATION_RSP INTERIM TRACK_CHANGED ffffffff ffffffff
 * AVRCP 0 REGISTER_NOTIFICATION_RSP CHANGED TRACK_CHANGED ffffffff ffffffff
 *
 * Get song Info
 * AVRCP PDU 20 2 1 2
 * AVRCP 0 GET_ELEMENT_ATTRIBUTES_RSP COUNT 2 TITLE 37 "Поколение дворников" ARTIST 16 "Аквариум"
 *
 * Get status
 * AVRCP PDU 30
 * AVRCP 0 GET_PLAY_STATUS_RSP 000506b6 0002419c PLAYING
 *                              lenght  position status
 *
 * Get bt device name 
 * NAME 40:9C:28:68:A0:05
 * NAME 40:9C:28:68:A0:05 "iPhone Nick"
 * NAME ERROR 0x104 00:07:80:FF:FF:F2 HCI_ERROR_PAGE_TIMEOUT
 * 
 * AVRCP REWIND
 * AVRCP FAST_FORWARD
 *
 *
 * */

#ifndef __IWRAP_H__
#define __IWRAP_H__

#include <stm32f10x.h>

#define BT_DEVICE_ADDR_SIZE 17
#define BT_DEVICE_NAME_SIZE 14 + 1 //14 - maximum length of nRF51 SDK + 1 for null
#define IWRAP_TX_BUFFER_SIZE 32

#define BT_LIST_CMD "LIST \r\n"
#define EVENT_PLAYBACK_STATUS_CHANGED "AVRCP PDU 31 1 \r\n"
#define EVENT_TRACK_CHANGED "AVRCP PDU 31 2 \r\n"
#define AVRCP_GET_ARTIST_TITLE "AVRCP PDU 20 2 1 2 \r\n"
#define AVRCP_PLAY "AVRCP PLAY \r\n"
#define AVRCP_PAUSE "AVRCP PAUSE \r\n"
#define AVRCP_NEXT "AVRCP FORWARD \r\n"
#define AVRCP_PREV "AVRCP BACKWARD \r\n"
#define AVRCP_GET_PLAY_STATUS "AVRCP PDU 30 \r\n"

uint8_t iwrap_tx_buffer[IWRAP_TX_BUFFER_SIZE];

enum PlaybackState
{
   stop, pause, play
} playbackState;

void bt_Play();
void bt_Pause();
void bt_Next();
void bt_Prev();
void bt_TrackChangedEventSubscribe();
void bt_PlaybackStatusEventSubscribe();
void bt_GetAVRCP_metadata();
void bt_GetBtDeviceAddres();
void bt_GetDeviceName();

#endif
