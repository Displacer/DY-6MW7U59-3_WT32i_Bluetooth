/*
 * idx	16			17			18			19			20			21
 * 01 - SAT			FM			NAME		RPT			DISC IN		C
 * 02 - XM			1 (FM)		TRACK		D. (RPT)	1			D
 * 04 - MD			2			ST			RDM			2			C
 * 08 - AUTO.P		3			RDS			D. (RDM)	3			I
 * 10 - CD			4			PTY			REG			4			R
 * 20 - MP3			LP			DISC		TAG			5			C
 * 40 - HD RADIO	2 (LP)		FOLDER		TP			6			L
 * 80 - CT			4			WMA			NAME		LOAD (LED)	E
 *
 *
 *
 *
 */

#include <stm32f10x.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_adc.h>
#include <stm32f10x_gpio.h>
#include "display_handler.h"
#include "command_handler.h"
#include "usart_opts.h"

extern enum Mode mode;
extern struct SongInfo;

uint8_t defaultScreen[DISPLAY_BUFFER_SIZE] = { 0x21, 0xFE, '-', 'M', 'I', 'T',
		'S', 'U', 'B', 'I', 'S', 'H', 'I', '-', 0xFF, 0xFF, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x9D };
uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];
uint8_t displayRxBuffer[DISPLAY_BUFFER_SIZE];
uint8_t displayStringBuffer[DISPLAY_STRING_SIZE];
uint8_t displayDataBuffer[DISPLAY_DATA_SIZE];

int greets_counter = 0;
uint8_t isAux;
uint8_t btLastState = 0;

enum displayState {
	begin, scroll, end
} display_state;

void GetMode() {
	if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_4)) // if bt module activated
		return;
	if (displayRxBuffer[16] & 0x10) { //All magic constants - for details see table
		mode = CD;
		return;
	}
	if (displayRxBuffer[17] & 0x01) {
		mode = FM;
		return;
	}
	if (displayRxBuffer[3] == 'W') {
		if (displayRxBuffer[2] == 'M') {
			mode = MW;
			return;
		}
		if (displayRxBuffer[2] == 'L') {
			mode = LW;
			return;
		}
	}
	if (displayRxBuffer[6] == 'A' && displayRxBuffer[7] == 'U'
			&& displayRxBuffer[8] == 'X') {
		mode = AUX;
		return;
	}
	//mode = Normal;
}

int offset;
uint8_t offset_step;
uint8_t delay;
uint8_t DISPLAY_STRING_DELAY = 30;
uint8_t OFFSET_STEP_DELAY = 2;

void resetDisplayState() {
	display_state = begin;
	offset = 0;
	delay = 0;
}

void HandleDisplayData() {
	if (CheckChksum(displayRxBuffer, DISPLAY_BUFFER_SIZE) == ERROR)
		return;

	for (uint8_t i = 0; i < DISPLAY_BUFFER_SIZE; i++) {
		displayBuffer[i] = displayRxBuffer[i];
	}

	//CD change ' on : (for details see table)
	if (displayBuffer[16] && 0x10 && displayBuffer[11] == '\'')
		displayBuffer[11] = ':';

	isAux = displayRxBuffer[6] == 'A' && displayRxBuffer[7] == 'U'
			&& displayRxBuffer[8] == 'X';

	if (mode == Bluetooth) {

		if (*displayDataBuffer != 0) {

			switch (display_state) {

			case begin:
				if (delay < DISPLAY_STRING_DELAY)
					delay++;
				else
					display_state = scroll;
				break;
			case scroll:
				if (displayDataBuffer[offset + DISPLAY_STRING_SIZE] != 0) {
					if (offset_step++ == OFFSET_STEP_DELAY) {
						offset++;
						offset_step = 0;
					}

				}

				else {
					display_state = end;
				}
				break;
			case end:
				if (delay > 0)
					delay--;
				else {
					display_state = begin;
					offset = 0;
				}
				break;
			}

			for (uint8_t i = 0; i < DISPLAY_STRING_SIZE; i++) {

					displayStringBuffer[i] = displayDataBuffer[i + offset];
			}
		}

		if (isAux) {

			if (*displayStringBuffer == 0) {
				displayBuffer[2] = BLUETOOTH_CHAR;
				displayBuffer[3] = ' ';
				displayBuffer[4] = 'B';
				displayBuffer[5] = 'L';
				displayBuffer[6] = 'U';
				displayBuffer[7] = 'E';
				displayBuffer[8] = 'T';
				displayBuffer[9] = 'O';
				displayBuffer[10] = 'O';
				displayBuffer[11] = 'T';
				displayBuffer[12] = 'H';
				displayBuffer[13] = ' ';
			} else {

				for (uint8_t i = 0; i < DISPLAY_STRING_SIZE; i++) {
					if (displayStringBuffer[i] == 0)
						displayBuffer[i + 2] = ' ';
					else
						displayBuffer[i + 2] = displayStringBuffer[i];
				}
			}

		}
	}

	for (int i = 0; i < DISPLAY_BUFFER_SIZE; i++) {
		if (greets_counter < 30) {

			displayBuffer[i] = defaultScreen[i];
			continue;
		}
		if (i < 2 || i > 13)
			continue;
		else {
			// for future bluetooth metadata
			// displayBuffer[i] = Translit(displayBuffer[i]);

		}

	}

	if (displayBuffer[0] == ACC_OFF) {
		greets_counter = 0; // ACC OFF byte for dimm display
		if (mode == Bluetooth) {
			btLastState = 1;
			Bluetooth_off();
		}
		mode = PowerOff;
	} else
		greets_counter++;

//	cnt++;
//	cnt2 = cnt / 5;
//	displayBuffer[2] = mode + 48;
	//displayBuffer[3] = btLastState + 48;
//	displayBuffer[4] = (ADC_GetConversionValue(ADC1) / 1000) % 10 + 48;
//	displayBuffer[5] = (ADC_GetConversionValue(ADC1) / 100) % 10 + 48;
//	displayBuffer[6] = (ADC_GetConversionValue(ADC1) / 10) % 10 + 48;
//	displayBuffer[7] = ADC_GetConversionValue(ADC1) % 10 + 48;
	//displayBuffer[14] = 0x00;
	SendDisplayData();
}
void SendDisplayData() {
	uint8_t chksum = 0;
	for (int i = 0; i < DISPLAY_BUFFER_SIZE - 1; i++) {
		chksum += displayBuffer[i];
	}
	displayBuffer[DISPLAY_BUFFER_SIZE - 1] = chksum;
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
