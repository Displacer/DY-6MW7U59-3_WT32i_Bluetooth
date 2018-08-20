#include <stm32f10x.h>

#include "parser.h"

uint16_t str_idx;
uint8_t buf[BUFF_SIZE];
uint8_t* argv[ARGS_COUNT];
uint8_t argc;
uint8_t text_flag;

uint16_t _atoi(uint8_t*);
uint8_t isStrEqual(uint8_t*, uint8_t*);
int8_t findIndexOfToken(uint8_t*);

struct SongInfo {
	uint8_t* Title;
	uint8_t* Artist;
	uint8_t* Album;
} songInfo;

/*
 int main()
 {
 int i;
 uint8_t* str = (uint8_t*)"AVRCP 1 GET_ELEMENT_ATTRIBUTES_RSP COUNT 6 TITLE 23 \"Осколок Льда\" ARTIST 8 \"Ария\" ALBUM 12 \"Химера\" TRACK_NUMBER 1 \"9\" TOTAL_TRACK_NUMBER 2 \"10\" GENRE 6 \"Rock\"\r"; //183
 uint8_t* str1 = (uint8_t*)"AVRCP 1 GET_ELEMENT_ATTRIBUTES_RSP COUNT 6 TITLE 35 \"Wait For Me (Ghost Note Symphonies)\" ARTIST 12 \"Rise Against\" ALBUM 33 \"The Ghost Note Symphonies, Vol. 1\" TRACK_NUMBER 1 \"8\" TOTAL_TRACK_NUMBER 2 \"10\" GENRE 6 \"Рок\"\r"; //222
 uint8_t* str2 = (uint8_t*)"AVRCP 1 GET_ELEMENT_ATTRIBUTES_RSP COUNT 6 TITLE 0 \"\" ARTIST 0 \"\" ALBUM 0 \"\" TRACK_NUMBER 0 \"\" TOTAL_TRACK_NUMBER 0 \"\" GENRE 0 \"\"\r"; //129
 Parse_init();
 for (int i = 0; i < 130; i++)
 {
 Parse(str2[i]);
 }

 //Show();

 //Parse((uint8_t*)"AVRCP 1 GET_ELEMENT_ATTRIBUTES_RSP COUNT 6 TITLE 23 \"Осколок Льда\" ARTIST 8 \"Ария\" ALBUM 12 \"Химера\" TRACK_NUMBER 1 \"9\" TOTAL_TRACK_NUMBER 2 \"10\" GENRE 6 \"Rock\"", 190);
 //cout << "eee" << endl;



 cin >> i;
 return 0;
 }*/

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

void Show() {
	if (isStrEqual(argv[0], (uint8_t*) "AVRCP")) {
		/*
		 getSongInfo();
		 cout << "Artist - " << songInfo.Artist << endl;
		 cout << "Album - " << songInfo.Album << endl;
		 cout << "Title - " << songInfo.Title << endl;

		 */

	}

	for (int i = 0; i < argc; i++) {
		//cout << argv[i] << endl;
	}
}

void Parse_init() {
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

	if (buf[str_idx] == '\r') {
		buf[str_idx] = 0;
		Show();
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
