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
 */

#include <stm32f10x.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_gpio.h>
#include "command_handler.h"
#include "usart_opts.h"
#include "tea6420.h"

extern enum Mode mode;

uint8_t default_command[COMMAND_BUFFER_SIZE] = { 0x41, 0x00, 0x00, 0x00, 0x30,
		0x00, 0x71 };
uint8_t commandBuffer[COMMAND_BUFFER_SIZE];

uint8_t press_cnt = 0, act_aux = 0;

void ActivateAUX() {
	act_aux = 1;
}

void Bluetooth_on() {
	mode = Bluetooth;
	tea6420_Bluetooth();
}
void Bluetooth_off() {
	mode = Normal;
	tea6420_AUX();
}

void HandleCommandData() {
	if (CheckChksum(commandBuffer, COMMAND_BUFFER_SIZE) == ERROR)
		return;

	if (act_aux && press_cnt < PRESS_DELAY) {
		for (int i = 0; i < COMMAND_BUFFER_SIZE; i++)
			commandBuffer[i] = default_command[i];
		commandBuffer[1] = 0x20;
		press_cnt++;
		SendCommand();
		return;
	} else if (act_aux) {
		act_aux = 0;
		press_cnt = PRESS_DELAY;
	}
	if (mode == Bluetooth) {
		if (press_cnt > 0) //block input for press delay
				{
			press_cnt--;
			for (int i = 0; i < COMMAND_BUFFER_SIZE; i++)
				commandBuffer[i] = default_command[i];
			SendCommand();
			return;
		}

		if (commandBuffer[1] == FM_BUTTON || commandBuffer[1] == CD_BUTTON) // return to normal state
			Bluetooth_off();

		if (commandBuffer[1] == 0x02) //power button
				{
			GPIO_SetBits(GPIOC, BT_PLAY);
		} else if (commandBuffer[2] == 0x01) // prev
				{
			GPIO_SetBits(GPIOC, BT_PREV);
		} else if (commandBuffer[2] == 0x02) // next
				{
			GPIO_SetBits(GPIOC, BT_NEXT);
		} else
			GPIO_ResetBits(GPIOC, BT_PLAY | BT_PREV | BT_NEXT);

		SendCommand();
		return;
	}
	if (mode == Normal) {

		if (commandBuffer[1] == FM_BUTTON) { // Hold FM button
			commandBuffer[1] = 0x00; // dummy
			press_cnt++;
			if (press_cnt > PRESS_DELAY) {
				press_cnt = 0;
				ActivateAUX();
				Bluetooth_on();
			}
			SendCommand();
			return;
		}

		if (press_cnt > 0) {
			commandBuffer[1] = FM_BUTTON;
			press_cnt--;
		}

		SendCommand();
	}

}
void SendCommand() {
	uint8_t chksum = 0;
	for (int i = 0; i < COMMAND_BUFFER_SIZE - 1; i++) {
		chksum += commandBuffer[i];
	}
	commandBuffer[COMMAND_BUFFER_SIZE - 1] = chksum;
	USART3SendDMA();
}