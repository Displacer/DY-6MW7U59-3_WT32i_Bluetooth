#include <stm32f10x.h>
#include <stm32f10x_usart.h>
#include "display_handler.h"
#include "usart_opts.h"

extern enum Mode mode;
extern uint8_t press_cnt;
uint8_t defaultScreen[DISPLAY_BUFFER_SIZE] = { 0x21, 0xFE, '-', 'M', 'I', 'T',
		'S', 'U', 'B', 'I', 'S', 'H', 'I', '-', 0xFF, 0xFF, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x9D };
//uint8_t defaultScreen[DISPLAY_BUFFER_SIZE] = {0x21, 0xFE, 
//	'Y', 'E', 'A', 'H', ' ', 'B', 'I', 'T', 'C', 'H', '!', ' ', 
//0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9D};
uint8_t displayTxBuffer[DISPLAY_BUFFER_SIZE];
uint8_t displayRxBuffer[DISPLAY_BUFFER_SIZE];

int greets_counter = 0;
int cnt = 600;
int cnt2 = 0;
void HandleDisplayData() {
	if (CheckChksum(displayRxBuffer, DISPLAY_BUFFER_SIZE) == ERROR)
		return;



	for (int i = 0; i < DISPLAY_BUFFER_SIZE; i++) {
		if (greets_counter < 30) {

			displayTxBuffer[i] = defaultScreen[i];
			continue;
		}
		if (i < 2 || i > 13) {
			displayTxBuffer[i] = displayRxBuffer[i];
			continue;
		} else {
			displayTxBuffer[i] = Translit(displayRxBuffer[i]);
		}

	}

	if (displayTxBuffer[0] == 0x40) greets_counter = 0; // ACC OFF byte for dimm display
	else greets_counter++;


	if (mode == Bluetooth)
	{
		if (displayTxBuffer[6] == 'A' && displayTxBuffer[7] == 'U' && displayTxBuffer[8] == 'X')
		{
			displayTxBuffer[2] = 0x86;
			displayTxBuffer[3] = ' ';
			displayTxBuffer[4] = 'B';
			displayTxBuffer[5] = 'L';
			displayTxBuffer[6] = 'U';
			displayTxBuffer[7] = 'E';
			displayTxBuffer[8] = 'T';
			displayTxBuffer[9] = 'O';
			displayTxBuffer[10] = 'O';
			displayTxBuffer[11] = 'T';
			displayTxBuffer[12] = 'H';
			displayTxBuffer[13] = ' ';
		}
	}
//	cnt++;
//	cnt2 = cnt / 5;
//	displayTxBuffer[2] = cnt2;
//	displayTxBuffer[3] = cnt2 / 100 + 48;
//	displayTxBuffer[4] = (cnt2 / 10) % 10 + 48;
//	displayTxBuffer[5] = cnt2 % 10 + 48;
	SendDisplayData();
}
void SendDisplayData() {
	uint8_t chksum = 0;
	for (int i = 0; i < DISPLAY_BUFFER_SIZE - 1; i++) {
		chksum += displayTxBuffer[i];
	}
	displayTxBuffer[DISPLAY_BUFFER_SIZE - 1] = chksum;
	USART2SendDMA();
}

uint8_t Translit(uint8_t ch) {
	if (ch < 192)
		return ch;
	uint8_t out;

	switch (ch - 32) {
	case 192:
		out = 65;
		break;
	case 193:
		out = 66;
		break;
	case 194:
		out = 86;
		break;
	case 195:
		out = 71;
		break;
	case 196:
		out = 68;
		break;
	case 197:
		out = 69;
		break;
	case 198:
		out = 90;
		break;
	case 199:
		out = 90;
		break;
	case 200:
		out = 73;
		break;
	case 201:
		out = 73;
		break;
	case 202:
		out = 75;
		break;
	case 203:
		out = 76;
		break;
	case 204:
		out = 77;
		break;
	case 205:
		out = 78;
		break;
	case 206:
		out = 79;
		break;
	case 207:
		out = 80;
		break;
	case 208:
		out = 82;
		break;
	case 209:
		out = 83;
		break;
	case 210:
		out = 84;
		break;
	case 211:
		out = 85;
		break;
	case 212:
		out = 70;
		break;
	case 213:
		out = 72;
		break;
	case 214:
		out = 67;
		break;
	case 215:
		out = 52;
		break;
	case 216:
		out = 87;
		break;
	case 217:
		out = 87;
		break;
	case 218:
		out = 98;
		break;
	case 219:
		out = 73;
		break;
	case 220:
		out = 98;
		break;
	case 221:
		out = 69;
		break;
	case 222:
		out = 85;
		break;
	case 223:
		out = 65;
		break;

	}
	switch (ch) {
	case 192:
		out = 65;
		break;
	case 193:
		out = 66;
		break;
	case 194:
		out = 86;
		break;
	case 195:
		out = 71;
		break;
	case 196:
		out = 68;
		break;
	case 197:
		out = 69;
		break;
	case 198:
		out = 90;
		break;
	case 199:
		out = 90;
		break;
	case 200:
		out = 73;
		break;
	case 201:
		out = 73;
		break;
	case 202:
		out = 75;
		break;
	case 203:
		out = 76;
		break;
	case 204:
		out = 77;
		break;
	case 205:
		out = 78;
		break;
	case 206:
		out = 79;
		break;
	case 207:
		out = 80;
		break;
	case 208:
		out = 82;
		break;
	case 209:
		out = 83;
		break;
	case 210:
		out = 84;
		break;
	case 211:
		out = 85;
		break;
	case 212:
		out = 70;
		break;
	case 213:
		out = 72;
		break;
	case 214:
		out = 67;
		break;
	case 215:
		out = 52;
		break;
	case 216:
		out = 87;
		break;
	case 217:
		out = 87;
		break;
	case 218:
		out = 98;
		break;
	case 219:
		out = 73;
		break;
	case 220:
		out = 98;
		break;
	case 221:
		out = 69;
		break;
	case 222:
		out = 85;
		break;
	case 223:
		out = 65;
		break;

	}
	return out;

}
