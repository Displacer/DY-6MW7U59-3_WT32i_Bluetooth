#include "iwrap.h"
#include "usart_opts.h"



void bt_Play() {
	USART1Send(AVRCP_PLAY);
	//bt_TrackChangedEventSubscribe();
	//bt_PlaybackStatusEventSubscribe();
	//bt_GetAVRCP_metadata();
}
void bt_Pause()
{
	USART1Send(AVRCP_PAUSE);
}
void bt_Next() {
	USART1Send(AVRCP_NEXT);
}
void bt_Prev() {
	USART1Send(AVRCP_PREV);
}

void bt_TrackChangedEventSubscribe() {
	USART1Send(EVENT_TRACK_CHANGED);
}

void bt_PlaybackStatusEventSubscribe() {
	USART1Send(EVENT_PLAYBACK_STATUS_CHANGED);
}

void bt_GetAVRCP_metadata() {
	USART1Send(AVRCP_GET_ARTIST_TITLE);
}

