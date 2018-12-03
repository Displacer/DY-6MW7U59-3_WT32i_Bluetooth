/*
 * Buttons codes
 * 41 00 00 00 30 00 71 not pressed
 *
 * 41 00 00 00 30 01 72 tune+
 * 41 00 00 00 30 09 7A tune-
 * 41 00 00 00 30 10 01 vol up
 * 41 00 00 00 30 90 01 vol dn
 *
 * 41 00 00 01 30 00 72 page
 * 41 00 00 02 30 00 73 disp
 * 41 00 00 04 30 00 75 pty
 * 41 00 00 08 30 00 79 tp
 *
 * 41 00 01 00 30 00 72 prev
 * 41 00 02 00 30 00 73 next
 * 41 00 04 00 30 00 75 1
 * 41 00 08 00 30 00 79 2
 * 41 00 10 00 30 00 81 3
 * 41 00 20 00 30 00 91 4
 * 41 00 40 00 30 00 B1 5
 * 41 00 80 00 30 00 F1 6
 *
 * 41 01 00 00 30 00 72 tune push
 * 41 02 00 00 30 00 73 power
 * 41 08 00 00 30 00 79 eject
 * 41 10 00 00 30 00 81 load
 * 41 20 00 00 30 00 91 CD
 * 41 40 00 00 30 00 B1 FM
 *
 * Remote
 * Mode - 800-900
 * Next - 1000-1100
 * Prev - 1200-1300
 * Vol+ - 1500
 * Vol- - 1700
 * Not pressed 2800
 *
 */

#include <stm32f10x.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_adc.h>
#include <string.h>
#include "command_handler.h"
#include "display_handler.h"
#include "usart_opts.h"
#include "tea6420.h"
#include "iwrap.h"
#include "config.h"

extern enum Mode mode;
extern uint8_t btLastState;
extern void ClearDisplayString();
extern void resetDisplayState();
extern enum PlaybackState playbackState;

uint8_t Mode_itterupt = MODE_ITTERUPT_CYCLES; // will check for 1 second for mode changes

uint8_t default_command[COMMAND_BUFFER_SIZE] = { 0x41, 0x00, 0x00, 0x00, 0x30,
		0x00, 0x71 };
uint8_t commandBuffer[COMMAND_BUFFER_SIZE];

uint8_t press_cnt = 0, act_aux = 0;

enum ButtonState {
	NOT_PRESSED, PRESSED, LONG_PRESSED, RELEASED
} button_state = NOT_PRESSED;
enum EMainFSM {
	NORMAL_STATE,
	BT_PREPARE,
	BT_ACTIVATE,
	BT_ACTIVE,
	BT_SHUTTING_DOWN,
	GOING_NORMAL_STATE,
	GOING_AUX
} main_fsm = NORMAL_STATE;

void ActivateAUX() {
	act_aux = !isAux;
}

void Bluetooth_on() {
	ClearDisplayString();
	mode = Bluetooth;
	GPIO_SetBits(GPIOA, BT_STBY_PIN);
	tea6420_Bluetooth();
}
void Bluetooth_off() {
	ClearDisplayString();
	resetDisplayState();
	playbackState = play;
	GPIO_ResetBits(GPIOA, BT_STBY_PIN);
	tea6420_AUX();
}

void SendCommand() {
	uint8_t chksum = 0;
	for (int i = 0; i < COMMAND_BUFFER_SIZE - 1; i++) {
		chksum += commandBuffer[i];
	}
	commandBuffer[COMMAND_BUFFER_SIZE - 1] = chksum;
	USART3SendDMA();
}

uint8_t fm_button_released = 1;
uint8_t avrcp_trig = 0;
//TODO: Delay for command send
void HandleCommandData() {
	if (CheckChksum(commandBuffer, COMMAND_BUFFER_SIZE) == ERROR)
		return;

	if (commandBuffer[1] == POWER_BUTTON || commandBuffer[1] == CD_BUTTON
			|| commandBuffer[1] == FM_BUTTON)
		Mode_itterupt = MODE_ITTERUPT_CYCLES; // if power/cd/fm pressed

	if (Mode_itterupt > 0) {
		Mode_itterupt--;
		if (Mode_itterupt == 0) {
			GetMode();
			if (btLastState == 1) {
				if (mode != AUX)
					ActivateAUX();
				Bluetooth_on();
				btLastState = 0;
			}
		}
	}

#define FSM
#ifdef FSM

	switch (commandBuffer[1]) {
	case NOTHING:
		button_state = NOT_PRESSED;
		break;
	case FM_BUTTON:
		button_state = PRESSED;
		if (press_cnt++ > PRESS_DELAY && main_fsm == NORMAL_STATE) {
			button_state = LONG_PRESSED;
			//press_cnt = PRESS_DELAY + 10;
		}
	}

	switch (main_fsm) {
	case NORMAL_STATE:
		if (button_state == LONG_PRESSED) {
			main_fsm = BT_PREPARE;
		}
		SendCommand();
	case BT_PREPARE:
		memcpy(commandBuffer, default_command, COMMAND_BUFFER_SIZE);
		commandBuffer[1] = CD_BUTTON;
		if (press_cnt-- > 0)
			SendCommand(); //Activating AUX
		else
			main_fsm = BT_ACTIVATE;
	case BT_ACTIVATE:
		Bluetooth_on();
		main_fsm = BT_ACTIVE;
	case BT_ACTIVE:
		if (commandBuffer[1] == FM_BUTTON || commandBuffer[1] == CD_BUTTON)
			main_fsm = GOING_NORMAL_STATE; // return to normal state

		if (commandBuffer[1] == POWER_BUTTON) {
			if (avrcp_trig == 0) {
				if (playbackState == play)
					bt_Pause();
				else
					bt_Play();
				avrcp_trig = 1;
			}
			commandBuffer[1] = 0x00;
		} else if (commandBuffer[2] == BACKWARD_BUTTON) {
			if (avrcp_trig == 0) {
				bt_Prev();
				avrcp_trig = 1;
			}
		} else if (commandBuffer[2] == FORWARD_BUTTON) {
			if (avrcp_trig == 0) {
				bt_Next();
				avrcp_trig = 1;
			}
		} else {
			uint8_t adc_val = ADC_GetConversionValue(ADC1) / 100;
			switch (adc_val) {
			case 8:
			case 9:
				main_fsm = GOING_NORMAL_STATE;
				break;
			case 10:
			case 11:
				if (avrcp_trig == 0) {
					bt_Next();
					avrcp_trig = 1;
				}
				break;
			case 12:
			case 13:
				if (avrcp_trig == 0) {
					bt_Prev();
					avrcp_trig = 1;
				}
				break;
			default:
				avrcp_trig = 0;
				break;
			}
		}
		SendCommand();
	case BT_SHUTTING_DOWN:
	case GOING_NORMAL_STATE:
		Bluetooth_off();
		main_fsm = NORMAL_STATE;
	case GOING_AUX:
		break;
	}

#elif

	if (act_aux && press_cnt < PRESS_DELAY) {
		for (int i = 0; i < COMMAND_BUFFER_SIZE; i++)
		commandBuffer[i] = default_command[i];
		commandBuffer[1] = CD_BUTTON;
		press_cnt++;
		SendCommand();
		return;
	} else if (act_aux) {
		act_aux = 0;
		press_cnt = PRESS_DELAY;
	}
	if (mode == Bluetooth) {
		if (press_cnt > 0 || !fm_button_released) //block input for press delay
		{
			if (commandBuffer[1] != FM_BUTTON)
			fm_button_released = 1;
			press_cnt--;
			for (int i = 0; i < COMMAND_BUFFER_SIZE; i++)
			commandBuffer[i] = default_command[i];
			SendCommand();
			return;
		}

		if (commandBuffer[1] == FM_BUTTON || commandBuffer[1] == CD_BUTTON) // return to normal state
		Bluetooth_off();

		if (commandBuffer[1] == POWER_BUTTON) {
			if (avrcp_trig == 0) {
				if (playbackState == play)
				bt_Pause();
				else
				bt_Play();
				avrcp_trig = 1;
			}
			commandBuffer[1] = 0x00;
		} else if (commandBuffer[2] == BACKWARD_BUTTON) {
			if (avrcp_trig == 0) {
				bt_Prev();
				avrcp_trig = 1;
			}
		} else if (commandBuffer[2] == FORWARD_BUTTON) {
			if (avrcp_trig == 0) {
				bt_Next();
				avrcp_trig = 1;
			}
		} else {
			///ADC handler

			uint8_t adc_val = ADC_GetConversionValue(ADC1) / 100;
			switch (adc_val) {
				case 8:
				case 9:
				Bluetooth_off();
				break;
				case 10:
				case 11:
				if (avrcp_trig == 0) {
					bt_Next();
					avrcp_trig = 1;
				}
				break;
				case 12:
				case 13:
				if (avrcp_trig == 0) {
					bt_Prev();
					avrcp_trig = 1;
				}
				break;

				default:
				avrcp_trig = 0;
				break;

			}
		}

		SendCommand();
		return;
	}
	if (mode != Bluetooth) {

		if (commandBuffer[1] == FM_BUTTON) { // Hold FM button
			commandBuffer[1] = NOTHING;
			fm_button_released = 0;
			press_cnt++;
			if (press_cnt > PRESS_DELAY) {
				press_cnt = 0;
				if (mode != AUX)
				ActivateAUX();
				Bluetooth_on();
			}
			SendCommand();
			return;
		}

		if (press_cnt > 0 && !act_aux) {
			fm_button_released = 1;
			commandBuffer[1] = FM_BUTTON;
			press_cnt--;
		}

		SendCommand();
	}
#endif

}

