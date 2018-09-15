#include <stm32f10x.h>

#include "parser.h"
#include "iwrap.h"
#include "display_handler.h"

uint16_t str_idx;
uint8_t buf[BUFF_SIZE];
uint8_t* argv[ARGS_COUNT];
uint8_t argc;
uint8_t text_flag;
uint8_t parsing = 0;

uint16_t _atoi(uint8_t*);
uint8_t isStrEqual(uint8_t*, uint8_t*);
int8_t findIndexOfToken(uint8_t*);

extern uint8_t displayStringBuffer[DISPLAY_STRING_SIZE];
extern uint8_t displayDataBuffer[DISPLAY_DATA_SIZE];
extern void resetDisplayState();

struct SongInfo {
	uint8_t* Title;
	uint8_t* Artist;
	uint8_t* Album;
} songInfo;

uint16_t _atoi(uint8_t* str) {
	uint16_t res = 0;
	for (uint8_t i = 0; str[i] != '\0'; ++i)
		res = res * 10 + str[i] - '0';
	return res;
}

int8_t findIndexOfToken(uint8_t* token) {
	for (int i = 0; i < argc; i++) {
		if (isStrEqual(argv[i], token))
			return i;
	}
	return -1;
}

void getSongInfo() {
	songInfo.Artist = argv[findIndexOfToken((uint8_t*) "ARTIST") + 2];
	songInfo.Album = argv[findIndexOfToken((uint8_t*) "ALBUM") + 2];
	songInfo.Title = argv[findIndexOfToken((uint8_t*) "TITLE") + 2];
}

HandleParseData() {

	uint16_t cnt = 0;
	uint8_t idx = 0;
	uint16_t unicode_char;
	/*do {
	 displayStringBuffer[cnt] = argv[0][cnt];
	 } while (argv[0][cnt] != 0 && cnt++ < DISPLAY_STRING_SIZE);*/

	if (isStrEqual(argv[0], (uint8_t*) "AVRCP")) {
		//displayStringBuffer[0] = 'A';
		if (isStrEqual(argv[2], (uint8_t*) "GET_ELEMENT_ATTRIBUTES_RSP")) {

			getSongInfo();
			cnt = 0;
			idx = 0;
			do {

				if (songInfo.Artist[idx] & 0xC0 && songInfo.Artist[idx+1] & 0x80) {
					unicode_char = songInfo.Artist[idx];
					unicode_char <<= 4;
					unicode_char |= songInfo.Artist[idx+1];
					if (unicode_char == 0xD090)	displayDataBuffer[cnt] = 'A';
					else displayDataBuffer[cnt] = '?';
					idx++;
				} else
					displayDataBuffer[cnt] = songInfo.Artist[idx];
			} while (songInfo.Artist[idx++] != 0 && cnt++ < DISPLAY_DATA_SIZE);
			idx = 0;
			//cnt++;
			displayDataBuffer[cnt++] = ' ';
			displayDataBuffer[cnt++] = '-';
			displayDataBuffer[cnt++] = ' ';

			do {
				displayDataBuffer[cnt] = *songInfo.Title++;
			} while (*songInfo.Title != 0 && cnt++ < DISPLAY_DATA_SIZE);

			cnt++;
			for (int i = cnt; i < DISPLAY_DATA_SIZE; i++) {
				displayDataBuffer[i] = 0;

			}
			resetDisplayState();

			return;
		}
		if (isStrEqual(argv[2], (uint8_t*) "REGISTER_NOTIFICATION_RSP")) {
			if (isStrEqual(argv[3], (uint8_t*) "CHANGED")) {
				bt_GetAVRCP_metadata();
				bt_TrackChangedEventSubscribe();
			}
		}
		return;
	}

	if (isStrEqual(argv[0], (uint8_t*) "CONNECT")) {
		//displayStringBuffer[0] = 'A';
		if (isStrEqual(argv[2], (uint8_t*) "A2DP")) {
			//displayStringBuffer[0] = 'P';
			//bt_TrackChangedEventSubscribe();
		}

		return;
	}

	if (isStrEqual(argv[0], (uint8_t*) "A2DP")) {
		if (isStrEqual(argv[1], (uint8_t*) "STREAMING")) {
			if (isStrEqual(argv[2], (uint8_t*) "STOP")) {
				//displayStringBuffer[0] = 0;
			}
			if (isStrEqual(argv[2], (uint8_t*) "START")) {
				//bt_GetAVRCP_metadata();
			}
		}
		//displayStringBuffer[0] = 'B';
		return;
	}

}

void Parse_init() {
	parsing = 1;
	str_idx = 0;
	argc = 1;
	argv[0] = buf;
	text_flag = 0;
}

uint16_t token_lenght;

void Parse(uint8_t ch) {
	if (str_idx >= BUFF_SIZE || argc >= ARGS_COUNT)
		return;

	buf[str_idx] = ch;

	if (buf[str_idx] == '\n') {
		buf[str_idx] = 0;
		parsing = 0;
		HandleParseData();
		return;
	}

	if (text_flag) {
		if (token_lenght--) {
			str_idx++;
			return;
		}

		buf[str_idx] = 0;
		str_idx++;
		text_flag = 0;
	}

	if (buf[str_idx] == '\"') {

		argv[argc - 1] = &buf[str_idx + 1];
		str_idx++;
		token_lenght = _atoi(argv[argc - 2]);
		text_flag = 1;
		return;
	}

	if (buf[str_idx] == ' ' && !text_flag) {
		argv[argc] = &buf[str_idx + 1];
		argc++;
		buf[str_idx] = 0;
	}
	str_idx++;
}

uint8_t isStrEqual(uint8_t* s1, uint8_t* s2) {
	uint8_t i = 0;
	while (s1[i] == s2[i] && s1[i] != '\0' && s2[i] != '\0')
		i++;
	if (s1[i] == '\0' && s2[i] == '\0')
		return 1;
	return 0;
}
