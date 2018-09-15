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
 *
 *
 *
 *
 *
 *
 *
 *
 *
 * */




#ifndef __IWRAP_H
#define __IWRAP_H

#define EVENT_TRACK_CHANGED "AVRCP PDU 31 2 \r\n"
#define AVRCP_GET_ARTIST_TITLE "AVRCP PDU 20 2 1 2 \r\n"
#define AVRCP_PLAY "AVRCP PLAY \r\n"
#define AVRCP_PAUSE "AVRCP PAUSE \r\n"
#define AVRCP_NEXT "AVRCP FORWARD \r\n"
#define AVRCP_PREV "AVRCP BACKWARD \r\n"
#define AVRCP_GET_PLAY_STATUS "AVRCP PDU 30 \r\n"


void bt_Play();
void bt_Pause();
void bt_Next();
void bt_Prev();
void bt_TrackChangedEventSubscribe();
void bt_GetAVRCP_metadata();

#endif
