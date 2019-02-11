/*
 * idx  16        17      18       19        20          21
 * 01 - SAT       FM      NAME     RPT       DISC IN     C
 * 02 - XM        1 (FM)  TRACK    D. (RPT)  1           D
 * 04 - MD        2       ST       RDM       2           C
 * 08 - AUTO.P    3       RDS      D. (RDM)  3           I
 * 10 - CD        4       PTY      REG       4           R
 * 20 - MP3       LP      DISC     TAG       5           C
 * 40 - HD RADIO  2 (LP)  FOLDER   TP        6           L
 * 80 - CT        4       WMA      NAME      LOAD (LED)  E
 */

#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include <string.h>
#include "display_handler.h"
#include "command_handler.h"
#include "usart_opts.h"
#include "config.h"
#include "iwrap.h"
#include "command_queue.h"
#include <stm32f10x_adc.h>
#include <stdio.h>
#include "can.h"

extern enum EMainFSM main_fsm;
extern uint8_t mode_interrupt;

uint8_t defaultScreen[DISPLAY_BUFFER_SIZE] = { 0x21, 0xFE, '-', 'M', 'I', 'T', 'S', 'U', 'B', 'I', 'S', 'H', 'I', '-', 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9D };
uint8_t displayBuffer[DISPLAY_BUFFER_SIZE];
uint8_t displayDataBuffer[DISPLAY_DATA_SIZE];
uint8_t displayBtDataBuffer[DISPLAY_DATA_SIZE];

uint8_t greets_counter = 0;
uint8_t btLastState = 0;
uint8_t force_show = 0;
int16_t offset;              
uint8_t offset_step;
uint8_t delay;


enum displayState
{
	begin,
	scroll,
	end,
	reverse_scroll
} display_state;



void CheckMode()
{
	if (GPIO_ReadOutputDataBit(BT_STBY_PORT, BT_STBY_PIN)) // if bt module activated
		{
			if (!(isAux && main_fsm == BT_ACTIVE))
			{				
				ForceShowString("BT shutting down from CheckMode! Fix it!\0");
				main_fsm = GOING_NORMAL_STATE;
			}			
		}
}



void ResetDisplayState()
{
	if (force_show) return;
	display_state = begin;
	offset = 0;
	delay = 0;
}
void ClearDisplayString()
{
	memset(displayDataBuffer, 0x00, DISPLAY_DATA_SIZE);
}

void ClearDisplayBtString()
{
	memset(displayBtDataBuffer, 0x00, DISPLAY_DATA_SIZE);
}
void ForceShowString(const char* str)
{
	if (force_show) return;	
	ResetDisplayState();
	ClearDisplayString();
	strcpy((char*)displayDataBuffer, str);
	/*
	for (int i = 0; i < DISPLAY_DATA_SIZE; i++)
	{
		displayDataBuffer[i] = str[i];
		if (str[i] == 0x00) break;
	}
	*/
	force_show = 1;
}


void SendDisplayData()
{
	uint8_t chksum = 0;
	for (int i = 0; i < DISPLAY_BUFFER_SIZE - 1; i++)
	{
		chksum += displayBuffer[i];
	}
	displayBuffer[DISPLAY_BUFFER_SIZE - 1] = chksum;
	USART2SendDMA();
}
extern uint8_t commandBuffer[COMMAND_BUFFER_SIZE];
void HandleDisplayData()
{
	IncTick();
	static uint8_t frame_delay = 0;		
		
	if (CheckChksum(displayBuffer, DISPLAY_BUFFER_SIZE) == ERROR)
	{
		ForceShowString("DIS:Csum err");
		return;
	}
		
	
	isAux = memcmp(&displayBuffer[6], (uint8_t*) "AUX", 3) == 0;

	//CD mode change ' on : (for details see table)
	if(displayBuffer[16] & 0x10 && displayBuffer[11] == '\'')
	{
		displayBuffer[11] = ':';
	}
	
	if (!isAux && force_show)
	{		
		force_show = 0;
		ResetDisplayState();
	}

	if (!force_show)
	{
		ClearDisplayString();
		if (isAux && (main_fsm == BT_ACTIVE  || frame_delay < 5))
		{
			if (main_fsm != BT_ACTIVE)
			{			
				frame_delay++;
			}
			else
			{
				frame_delay = 0;
			}			
			if (*displayBtDataBuffer == 0x00 || playbackState == stop)
			{
				displayDataBuffer[0] = BLUETOOTH_CHAR;
				memcpy(&displayDataBuffer[1], (uint8_t*) " Bluetooth ", 11);
				ResetDisplayState();
			}		
			else
			{				
				memcpy(displayDataBuffer, displayBtDataBuffer, DISPLAY_DATA_SIZE);
			}		
		}
		else
		{
			if (!isAux) frame_delay = 100;
			ResetDisplayState();
			memcpy(displayDataBuffer, &displayBuffer[2], DISPLAY_STRING_SIZE);
		}
	}
	
	
	switch (display_state)
	{
	case begin:
		if (delay < DISPLAY_STRING_DELAY)
			delay++;
		else
			display_state = scroll;
		break;
	case scroll:
		if (displayDataBuffer[offset + DISPLAY_STRING_SIZE] != 0)
		{
			if (offset_step++ == OFFSET_STEP_DELAY)
			{
				offset++;
				offset_step = 0;
			}
		}
		else
		{
			display_state = end;
		}
		break;
	case end:
		if (delay > 0)
			delay--;
		else
		{			
			display_state = reverse_scroll;
		}
		break;
	case reverse_scroll:
		if (offset != 0)
			offset--;
		else
		{
			if (force_show)
			{
				force_show = 0;
			}
			display_state = begin;			
		}			
		break;
	}
	
	for (uint8_t i = 0; i < DISPLAY_STRING_SIZE; i++)
	{
		if (displayDataBuffer[offset + i] == 0)
			displayBuffer[i + 2] = ' ';
		else
			displayBuffer[i + 2] = displayDataBuffer[offset + i];
	}	

	if (greets_counter < 30)
	{
		greets_counter++;
		memcpy(displayBuffer, defaultScreen, DISPLAY_BUFFER_SIZE);
	}

	if (displayBuffer[0] == ACC_OFF)
	{

		mode_interrupt = MODE_INTERRUPT_CYCLES * 2;
		greets_counter = 0;
		if (main_fsm == BT_ACTIVE)
		{
			btLastState = 1;
			Bluetooth_off();
		}
		main_fsm = NORMAL_STATE;
	}

	/*
	if (displayBuffer[20] & 0x80)
	{
		char str[DISPLAY_STRING_SIZE];
		sprintf(str, "ADC: %*d", 4, GetRemoteAdcData());
		memcpy(&displayBuffer[2], str, DISPLAY_STRING_SIZE);
		//displayBuffer[2] = 'A';
		//displayBuffer[3] = 'D';
		//displayBuffer[4] = 'C';
		//displayBuffer[5] = ':';
		//displayBuffer[6] = ' ';
		//displayBuffer[7] = (ADC_GetConversionValue(ADC1) / 1000) % 10 + 48;
		//displayBuffer[8] = (ADC_GetConversionValue(ADC1) / 100) % 10 + 48;
		//displayBuffer[9] = (ADC_GetConversionValue(ADC1) / 10) % 10 + 48;
		//displayBuffer[10] = ADC_GetConversionValue(ADC1) % 10 + 48;
		//displayBuffer[11] = ' ';
	}
	// cnt++;
	// cnt2 = cnt / 5;
	// displayBuffer[2] = mode + 48;
	   //displayBuffer[3] = btLastState + 48;
	 //   displayBuffer[4] = (ADC_GetConversionValue(ADC1) / 1000) % 10 + 48;
	   // displayBuffer[5] = (ADC_GetConversionValue(ADC1) / 100) % 10 + 48;
	    //displayBuffer[6] = (ADC_GetConversionValue(ADC1) / 10) % 10 + 48;
	    //displayBuffer[7] = ADC_GetConversionValue(ADC1) % 10 + 48;
	      //displayBuffer[14] = 0x00;
	/*
	for (uint8_t i = 0, j = 2; i < COMMAND_BUFFER_SIZE; i++)
	{
		uint8_t tmp;
		if (j > 12) continue;
		tmp = (commandBuffer[i] >> 4);
		tmp = tmp > 9 ? tmp - 10 + 'A' : tmp + '0';
		displayBuffer[j++] = tmp;
		tmp = commandBuffer[i] & 0x0F;
		tmp = tmp > 9 ? tmp - 10 + 'A' : tmp + '0';
		displayBuffer[j++] = tmp;
		
	}*/
	if (memcmp(&displayBuffer[2], "            ", 12) == 0)
	{
		uint16_t res = GetLitersPerHour();
		char tmp[12] = { 0 };
		sprintf(tmp, "%*d.%d l/h", 2, res / 10, res % 10);
		strcpy(&displayBuffer[2], tmp);
	}	
	SendDisplayData();
}

