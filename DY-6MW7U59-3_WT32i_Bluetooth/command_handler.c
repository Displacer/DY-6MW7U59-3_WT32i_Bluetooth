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
 * OLD PCB
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
#include "iwrap.h"
#include "can.h"
#include "config.h"

#ifndef NEW_PCB
#include "tea6420.h"
#endif // !NEW_PCB


extern uint8_t btLastState;
extern enum PlaybackState playbackState;

uint8_t mode_interrupt = MODE_INTERRUPT_CYCLES;       // will check for 1 second for mode changes

uint8_t default_command[COMMAND_BUFFER_SIZE] =
{ 0x41, 0x00, 0x00, 0x00, 0x30, 0x00, 0x71 };
uint8_t commandBuffer[COMMAND_BUFFER_SIZE];

uint8_t press_delay = 0, act_aux = 0, button_val;

enum ButtonState
{
	PRESSED,
	LONG_PRESSED,
	RELEASED
} button_state = RELEASED;

enum EMainFSM main_fsm = NORMAL_STATE;

void ActivateAUX()
{
	act_aux = isAux ? 0 : 1;
}

uint16_t GetRemoteAdcData()
{
	uint16_t tmp = 0;
	for (uint8_t i = 0; i < 10; i++)
	{
		tmp += ADC_GetConversionValue(ADC1);
	}
	tmp /= 10;
	return tmp;
}

void Bluetooth_on()
{
	ClearDisplayBtString();
	ResetDisplayState();
	GPIO_SetBits(BT_STBY_PORT, BT_STBY_PIN);
#ifndef NEW_PCB
	tea6420_Bluetooth();
#endif // !NEW_PCB
	
}
void Bluetooth_off()
{	
	bt_Pause();
	GPIO_ResetBits(BT_STBY_PORT, BT_STBY_PIN);
#ifndef NEW_PCB
	tea6420_AUX();
#endif // !NEW_PCB
}

void SendCommand()
{
	uint8_t chksum = 0;
	for (int i = 0; i < COMMAND_BUFFER_SIZE - 1; i++)
	{
		chksum += commandBuffer[i];
	}
	commandBuffer[COMMAND_BUFFER_SIZE - 1] = chksum;
	USART3SendDMA();
}

uint8_t avrcp_trig = 0;
void HandleCommandData()
{
	if (CheckChksum(commandBuffer, COMMAND_BUFFER_SIZE) == ERROR)
	{
		ForceShowString("CMD:Csum err");
		return;
	}

	if (commandBuffer[1] & (CD_BUTTON | FM_BUTTON))
		mode_interrupt = MODE_INTERRUPT_CYCLES;  

	if (mode_interrupt > 0)
	{	
		mode_interrupt--;
		if (mode_interrupt == 0)
		{			
			if (btLastState == 1)
			{
				main_fsm = BT_ACTIVATE;
				btLastState = 0;
				return;
			}
			CheckMode();
		}
	}

	switch (commandBuffer[1])
	{
	case NOTHING:
		if (button_state == PRESSED)
		{			
			memcpy(commandBuffer, default_command, COMMAND_BUFFER_SIZE);
			commandBuffer[1] = button_val;
			if (press_delay++ > 8)
			{
				if (button_val & (FM_BUTTON | CD_BUTTON)) main_fsm = GOING_NORMAL_STATE;
				button_state = RELEASED;
				press_delay = 0;
				button_val = NOTHING;
			}
		}
		if (button_state == LONG_PRESSED)
		{
			button_val = NOTHING;
			button_state = RELEASED;
		}
		break;
	case CD_BUTTON:
	case FM_BUTTON:
		button_val = commandBuffer[1];
		if (button_state == LONG_PRESSED)
		{
			memcpy(commandBuffer, default_command, COMMAND_BUFFER_SIZE);
			break;
		}
		button_state = PRESSED;
		commandBuffer[1] = NOTHING;
		if (press_delay++ > PRESS_DELAY / 2)
		{
			button_state = LONG_PRESSED;
			press_delay = 0;
		}
		break;
	}

	switch (main_fsm)
	{
	case NORMAL_STATE:
		if (button_state == LONG_PRESSED)
		{
			if (button_val == FM_BUTTON)
			{							
				main_fsm = BT_ACTIVATE;
			}
			if (button_val == CD_BUTTON)
			{
				ActivateAUX();        //TODO: убрать костыль
			}
		}
		break;
	case BT_ACTIVATE:
		ActivateAUX();
		Bluetooth_on();
		main_fsm = BT_ACTIVE;
		break;
	case BT_ACTIVE:
		if (commandBuffer[1] == POWER_BUTTON)
		{
			if (avrcp_trig == 0)
			{
				bt_PlaybackStatusEventSubscribe();
				if (playbackState == play)
					bt_Pause();
				else
					bt_Play();
				avrcp_trig = 1;
			}
			commandBuffer[1] = NOTHING;
		}
		else if (commandBuffer[2] == BACKWARD_BUTTON)
		{
			if (avrcp_trig == 0)
			{
				bt_Prev();
				avrcp_trig = 1;
			}
		}
		else if (commandBuffer[2] == FORWARD_BUTTON)
		{
			if (avrcp_trig == 0)
			{
				bt_Next();
				avrcp_trig = 1;
			}
		}
		else
		{
			uint8_t adc_val = GetRemoteAdcData() / 100;
			switch (adc_val)
			{
			case 1:
				main_fsm = GOING_NORMAL_STATE;
				break;
			case 2:
			case 3:
				if (avrcp_trig == 0)
				{
					bt_Next();
					avrcp_trig = 1;
				}
				break;
			case 4:
				if (avrcp_trig == 0)
				{
					bt_Prev();
					avrcp_trig = 1;
				}
				break;
			default:
				avrcp_trig = 0;
				break;
			}
		}
		if (button_state == LONG_PRESSED && button_val == CD_BUTTON)
		{
			CanBeep(SHORT_BEEP);
			main_fsm = GOING_NORMAL_STATE;
		}
		break;
	case GOING_NORMAL_STATE:
		Bluetooth_off();
		main_fsm = NORMAL_STATE;
		break;
	}
	static uint8_t aux_press_delay = 0;
	if (act_aux)
	{
		memcpy(commandBuffer, default_command, COMMAND_BUFFER_SIZE);
		commandBuffer[1] = CD_BUTTON;
		if (aux_press_delay++ > PRESS_DELAY)
		{
			act_aux = 0;
			aux_press_delay = 0;
		}
	}
	SendCommand();
}

